#include <X11/X.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

//#include <alsa/asoundlib.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <dlfcn.h>
#include <errno.h>

#include <string.h>

#define STDOUT 1
#define STDIN 0
#define STDERR 2

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

void debug_print(const char *fmt, ...);

#define assert(x)\
	do {\
		if (!(x)) {\
			debug_print("%s: %s: line %d: assertion failed '%s'\n", __FILE__, __func__, __LINE__, #x);\
			raise(SIGILL);\
		}\
	} while(0)

enum Key_Symbol {
	W_KEY = XK_w,
	A_KEY = XK_a,
	S_KEY = XK_s,
	D_KEY = XK_d,
	E_KEY = XK_e,
	G_KEY = XK_g,
	Q_KEY = XK_q,
	R_KEY = XK_r,
	F_KEY = XK_f,
	P_KEY = XK_p,
	L_KEY = XK_l,
	C_KEY = XK_c,
	J_KEY = XK_j,
	K_KEY = XK_k,
	I_KEY = XK_i,
	M_KEY = XK_m,
	BACKSPACE_KEY = XK_BackSpace,
	LCTRL_KEY = XK_Control_L,
	RCTRL_KEY = XK_Control_R,
	LALT_KEY = XK_Alt_L,
	RALT_KEY = XK_Alt_R,
	ESCAPE_KEY = XK_Escape,
};

enum Mouse_Button {
	MOUSE_BUTTON_LEFT = 0,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
};

typedef s32       File_Handle;
typedef pthread_t Thread_Handle;
typedef timespec  Platform_Time;
typedef sem_t     Semaphore_Handle;
typedef void *    Library_Handle;
typedef u64       File_Offset;

File_Handle FILE_HANDLE_ERROR = -1;
File_Offset FILE_OFFSET_ERROR = (File_Offset)-1;

enum File_Seek_Relative {
	FILE_SEEK_START = SEEK_SET,
	FILE_SEEK_CURRENT = SEEK_CUR,
	FILE_SEEK_END = SEEK_END
};

u32  window_pixel_width          =  0;
u32  window_pixel_height         =  0;
f32  window_scaled_meter_width   =  0;
f32  window_scaled_meter_height  =  0;
u32  pixels_per_meter            =  0;
f32  meters_per_pixel            =  0;
f32  scaled_meters_per_pixel     =  0;

// @TODO: Store colormap and free it on exit.
struct Linux_Context {
	Display   *display;
	Window     window;
	Atom       wm_delete_window;
	s32        xinput_opcode;
	//snd_pcm_t *pcm_handle;
} linux_context;

void platform_exit(int exit_code);
bool platform_write_file(File_Handle fh, size_t n, const void *buf);

const char *get_platform_error() {
	return strerror(errno);
}

static s8 x11_error_occured = false;

s32 x11_error_handler(Display *, XErrorEvent *event) {
	char buffer[256];
	XGetErrorText(linux_context.display, event->error_code, buffer, sizeof(buffer));
	log_print(MAJOR_ERROR_LOG, "X11 error: %s.", buffer);
	x11_error_occured = true;
	return 0;
}

#if 0
const char *first_occurrence_of(const char *s, char c);
const char *first_occurrence_of(const char *s, const char *substring);
size_t string_length(const char *s);

s8 is_extension_supported(const char *extension_list, const char *extension) {
	const char *start;
	const char *where, *terminator;

	// Extension names should not have spaces.
	where = first_occurrence_of(extension, ' ');
	if (where || *extension == '\0') {
		return false;
	}

	for (start = extension_list;;) {
		where = first_occurrence_of(start, extension);
		if (!where) {
			break;
		}

		terminator = where + string_length(extension);
		if (where == start || *(where - 1) == ' ') {
			if (*terminator == ' ' || *terminator == '\0') {
				return true;
			}
		}
		start = terminator;
	}

	return false;
}

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;

GLXContext create_opengl_context(Display *display, GLXDrawable drawable, s32 screen) {
	s32 context_attributes[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 1,
		GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};

	s32 framebuffer_attributes[] = {
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		GLX_DOUBLEBUFFER, True,
		None
	};

	s32 framebuffer_config_count = 0;
	GLXFBConfig *framebuffer_configs = glXChooseFBConfig(display, screen, framebuffer_attributes, &framebuffer_config_count);
	if (framebuffer_configs == NULL || framebuffer_config_count == 0) {
		_abort("Failed to retrieve frame buffer configurations");
	}

	s32 best_fbc = -1, best_num_samp = -1;
	for (s32 i = 0; i < framebuffer_config_count; ++i) {
		XVisualInfo *visual = glXGetVisualFromFBConfig(display, framebuffer_configs[i]);
		if (visual) {
			s32 sample_buffer, samples;
			glXGetFBConfigAttrib(display, framebuffer_configs[i], GLX_SAMPLE_BUFFERS, &sample_buffer);
			glXGetFBConfigAttrib(display, framebuffer_configs[i], GLX_SAMPLES, &samples);

			if (best_fbc < 0 || (sample_buffer && samples > best_num_samp)) {
				best_fbc = i, best_num_samp = samples;
			}
		}
		XFree(visual);
	}

	if (best_fbc < 0) {
		_abort("Failed to get a frame buffer");
	}

	auto selected_framebuffer_config = framebuffer_configs[0]; // @TEMP
	XFree(framebuffer_configs);

	int major_version, minor_version;
	if (!glXQueryVersion(display, &major_version, &minor_version)) {
		_abort("Unable to query GLX version");
	}
	if ((major_version == 1 && minor_version < 3) || major_version < 1) {
		_abort("GLX version is too old");
	}

	const char *extensions = glXQueryExtensionsString(display, screen);
	if (!is_extension_supported(extensions, "GLX_ARB_create_context")) {
		_abort("OpenGL does not support glXCreateContextAttribsARB extension");
	}

	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
	if (!glXCreateContextAttribsARB) {
		_abort("Could not load glXCreateContextAttribsARB()");
	}

	auto opengl_context = glXCreateContextAttribsARB(display, selected_framebuffer_config, 0, True, context_attributes);

	XSync(display, False);
	if (x11_error_occured || opengl_context == NULL) {
		_abort("Failed to create OpenGL context");
	}
	if (glXMakeCurrent(display, drawable, opengl_context) == False) {
		_abort("Could not call glXMakeCurrent on the main thread OpenGL context");
	}

	// Try to enable vsync.
	if (is_extension_supported(extensions, "GLX_EXT_swap_control")) {
		typedef int (*glXSwapIntervalEXTProc)(Display *, GLXDrawable, int);
		glXSwapIntervalEXTProc glXSwapIntervalEXT = NULL;
		glXSwapIntervalEXT = (glXSwapIntervalEXTProc)glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalEXT");
		if (glXSwapIntervalEXT) {
			glXSwapIntervalEXT(glXGetCurrentDisplay(), glXGetCurrentDrawable(), 1);
		} else {
			log_print(CRITICAL_ERROR_LOG, "Could not load glXSwapIntervalEXT()");
		}
	} else if (is_extension_supported(extensions, "GLX_MESA_swap_control")) {
		typedef int (*glXSwapIntervalMESAProc)(int);
		glXSwapIntervalMESAProc glXSwapIntervalMESA = NULL;
		glXSwapIntervalMESA = (glXSwapIntervalMESAProc)glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalMESA");
		if (glXSwapIntervalMESA) {
			glXSwapIntervalMESA(1);
		} else {
			log_print(CRITICAL_ERROR_LOG, "Could not load glXSwapIntervalMESA()");
		}
	} else if (is_extension_supported(extensions, "GLX_SGI_swap_control")) {
		typedef int (*glXSwapIntervalSGIProc)(int);
		glXSwapIntervalSGIProc glXSwapIntervalSGI = NULL;
		glXSwapIntervalSGI = (glXSwapIntervalSGIProc)glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalSGI");
		if (glXSwapIntervalSGI) {
			glXSwapIntervalSGI(1);
		} else {
			log_print(CRITICAL_ERROR_LOG, "Could not load glXSwapIntervalSGI()");
		}
	}

	return opengl_context;
}
#endif

#if 0
void render_init();
void render_cleanup();
void init_assets();
void assets_load_all();
void debug_init();

// @TODO: Move job stuff to another file!
#define MAX_JOBS 256
#define NUM_JOB_THREADS 4

typedef void (*Do_Job_Callback)(void *, File_Handle);

struct Thread_Job {
	void *job_data;
	Do_Job_Callback do_job_callback;
};

Semaphore_Handle platform_make_semaphore(u32 initial_value);

struct Job_Queue {
	Thread_Job jobs[MAX_JOBS];
	Semaphore_Handle semaphore;
	volatile u32 read_head;
	volatile u32 write_head;
};

Job_Queue job_queue;

void *job_thread_start(void *job_thread_data);

Thread_Handle platform_create_thread(Thread_Procedure tp, void *thread_argument);

void platform_toggle_fullscreen();
#endif

void application_entry();

s32 main(s32, char **) {
	srand(time(0));

	// Enable X11 multithreading.
	//XInitThreads();

	// Install a new error handler.
	// Note this error handler is global.  All display connections in all threads of a process use the same error handler.
	XSetErrorHandler(&x11_error_handler);

	linux_context.display = XOpenDisplay(NULL);
	if (!linux_context.display) {
		_abort("Failed to create display");
	}

	auto screen = XDefaultScreen(linux_context.display);
	auto root_window = XRootWindow(linux_context.display, screen);

	// Initialize XInput2.
	{
		s32 event, error, major_version, minor_version;
		if (!XQueryExtension(linux_context.display, "XInputExtension", &linux_context.xinput_opcode, &event, &error)) {
			_abort("The X server does not support the XInput extension");
		}

		XIQueryVersion(linux_context.display, &major_version, &minor_version);
		if (major_version < 2) {
			_abort("XInput version 2.0 or greater is required: version %d.%d is available", major_version, minor_version);
		}

		XIEventMask event_mask;
		u8 mask[3] = { 0,0,0 };
		event_mask.deviceid = XIAllMasterDevices;
		event_mask.mask_len = sizeof(mask);
		event_mask.mask = mask;

		XISetMask(mask, XI_RawMotion);
		XISetMask(mask, XI_RawButtonPress);
		XISetMask(mask, XI_RawButtonRelease);
		XISetMask(mask, XI_RawKeyPress);
		XISetMask(mask, XI_RawKeyRelease);

		if (XISelectEvents(linux_context.display, root_window, &event_mask, 1) != Success) {
			_abort("Failed to select XInput events");
		}
	}

	// Create window.
	{
		XVisualInfo visual_info_template = {};
		visual_info_template.screen = screen;

		s32 number_of_visuals;
		auto visual_info = XGetVisualInfo(linux_context.display, VisualScreenMask, &visual_info_template, &number_of_visuals);
		assert(visual_info->c_class == TrueColor);

		XSetWindowAttributes window_attributes = {};
		window_attributes.colormap = XCreateColormap(linux_context.display, root_window, visual_info->visual, AllocNone);
		window_attributes.background_pixel = 0xFFFFFFFF;
		window_attributes.border_pixmap = None;
		window_attributes.border_pixel = 0;
		window_attributes.event_mask = StructureNotifyMask
		                             | FocusChangeMask
		                             | EnterWindowMask
		                             | LeaveWindowMask
		                             | ExposureMask
		                             | ButtonPressMask
		                             | ButtonReleaseMask
		                             | KeyPressMask
		                             | KeyReleaseMask;

		s32 window_attributes_mask = CWBackPixel
		                           | CWColormap
		                           | CWBorderPixel
		                           | CWEventMask;

		s32 requested_window_width = 800;
		s32 requested_window_height = 600;

		linux_context.window = XCreateWindow(linux_context.display,
		                                     root_window,
		                                     0,
		                                     0,
		                                     requested_window_width,
		                                     requested_window_height,
		                                     0,
		                                     visual_info->depth,
		                                     InputOutput,
		                                     visual_info->visual,
		                                     window_attributes_mask,
		                                     &window_attributes);

		if (!linux_context.window) {
			_abort("Failed to create a window");
		}

		XFree(visual_info);

		XStoreName(linux_context.display, linux_context.window, "cge");
		XMapWindow(linux_context.display, linux_context.window);
	}

	XFlush(linux_context.display);
	if ((linux_context.wm_delete_window = XInternAtom(linux_context.display, "WM_DELETE_WINDOW", 1))) {
		XSetWMProtocols(linux_context.display, linux_context.window, &linux_context.wm_delete_window, 1);
	} else {
		log_print(MINOR_ERROR_LOG, "Unable to register WM_DELETE_WINDOW atom.");
	}

	// Get actual window dimensions without window borders.
	{
		s32 win_x, win_y;
		u32 border_width, depth;
		if (XGetGeometry(linux_context.display, linux_context.window, &root_window, &win_x, &win_y, &window_pixel_width, &window_pixel_height, &border_width, &depth) == 0) {
			_abort("Failed to get the screen's geometry.");
		}
	}
#if 0
	// @TODO: Handle this stuff on resize too.
	u32 reference_window_width  = 640;
	u32 reference_window_height = 360;

	u32 scale = u32_min(window_pixel_width / reference_window_width, window_pixel_height / reference_window_height);

	pixels_per_meter        = 32;
	meters_per_pixel        = 1.0f / pixels_per_meter;
	scaled_meters_per_pixel = meters_per_pixel / scale;

	window_scaled_meter_width  = window_pixel_width  * scaled_meters_per_pixel;
	window_scaled_meter_height = window_pixel_height * scaled_meters_per_pixel;

	printf("Pixels per meter: %u\n"
	       "Meters per pixel: %.9g\n"
	       "Scale: %u\n"
	       "Scaled meters per pixel: %.9g\n"
	       "Reference window: %u %u\n"
	       "Window pixel: %u %u\n"
	       "Window meter: %.9g %.9g\n\n",
	       pixels_per_meter, meters_per_pixel, scale, scaled_meters_per_pixel, reference_window_width, reference_window_height, window_pixel_width, window_pixel_height, window_scaled_meter_width, window_scaled_meter_height);

	linux_context.gl_context = platform_init_opengl_context();

	init_assets();

	for (u32 i = 0; i < NUM_JOB_THREADS; ++i) {
		platform_create_thread(job_thread_start, &job_queue);
	}

	render_init();

	debug_init();
#endif

	//XSync(linux_context.display, False);

	application_entry();

	//render_cleanup();

	//platform_exit(EXIT_SUCCESS);

	return 0;
}

void cleanup_platform_display() {
	XDestroyWindow(linux_context.display, linux_context.window);
	XCloseDisplay(linux_context.display);
}

u32 key_symbol_to_scancode(Key_Symbol key_symbol) {
	u32 scancode = XKeysymToKeycode(linux_context.display, key_symbol);
	assert(scancode > 0);
	return scancode;
}

#if 0
void input_key_down(Keyboard *, u32, Key_Symbol);

u32 platform_keysym_to_scancode(Key_Symbol ks) {
	u32 sc = XKeysymToKeycode(linux_context.display, ks);
	assert(sc > 0);
	return sc;
}

Key_Symbol platform_keycode_to_keysym(u32 keycode, s8 level) {
	// @TODO: Use this?
	/*
	int keysyms_per_keycode_return;
	KeySym *keysym = XGetKeyboardMapping(dpy,
			xe->xkey.keycode,
			1,
			&keysyms_per_keycode_return);
	XFree(keysym);
	*/
	KeySym keysym = NoSymbol;
	#if 0
	//unsigned int event_mask = ShiftMask | LockMask;
	XkbDescPtr keyboard_map = XkbGetMap(linux_context.display, XkbAllClientInfoMask, XkbUseCoreKbd);
	if (keyboard_map) {
		//What is diff between XkbKeyGroupInfo and XkbKeyNumGroups?
		unsigned char info = XkbKeyGroupInfo(keyboard_map, keycode);
		unsigned int num_groups = XkbKeyNumGroups(keyboard_map, keycode);

		//Get the group
		unsigned int group = 0x00;
		switch (XkbOutOfRangeGroupAction(info)) {
			case XkbRedirectIntoRange: {
							   /* If the RedirectIntoRange flag is set, the four least significant
							    * bits of the groups wrap control specify the index of a group to
							    * which all illegal groups correspond. If the specified group is
							    * also out of range, all illegal groups map to Group1.
							    */
							   group = XkbOutOfRangeGroupInfo(info);
							   if (group >= num_groups) group = 0;
						   } break;
			case XkbClampIntoRange: {
							/* If the ClampIntoRange flag is set, out-of-range groups correspond
							 * to the nearest legal group. Effective groups larger than the
							 * highest supported group are mapped to the highest supported group;
							 * effective groups less than Group1 are mapped to Group1 . For
							 * example, a key with two groups of symbols uses Group2 type and
							 * symbols if the global effective group is either Group3 or Group4.
							 */
							group = num_groups - 1;
						} break;
			case XkbWrapIntoRange: {
						       /* If neither flag is set, group is wrapped into range using integer
							* modulus. For example, a key with two groups of symbols for which
							* groups wrap uses Group1 symbols if the global effective group is
							* Group3 or Group2 symbols if the global effective group is Group4.
							*/
					       } // Fall-through.
			default: {
					 if (num_groups != 0) {
						 group %= num_groups;
					 }
				 } break;
		}

		//int level = 

		/*
		   XkbKeyTypePtr key_type = XkbKeyKeyType(keyboard_map, keycode, group);
		   unsigned int active_mods = event_mask & key_type->mods.mask;

		   int level2 = 0;
		   for (int i = 0; i < key_type->map_count; ++i) {
		   if (key_type->map[i].active && key_type->map[i].mods.mask == active_mods) {
		   level2 = key_type->map[i].level;
		   }
		   }

		 */
		//keysym = XkbKeySymEntry(keyboard_map, keycode, level, group);
		//XkbFreeClientMap(keyboard_map, XkbAllClientInfoMask, true);
		keysym = XkbKeycodeToKeysym(linux_context.display, keycode, group, level);
	}
	#endif

	return (Key_Symbol)keysym;
}

#endif

/*
void
platform_reset_mouse(Mouse *m)
{
	//m->left.num_presses = m->left.num_releases = 0;
	//m->middle.num_presses = m->middle.num_releases = 0;
	//m->right.num_presses = m->right.num_releases = 0;
}

Mouse
platform_clear_mouse(Mouse m)
{
	_memset(&m.spatial, 0, sizeof(m.spatial));
	_memset(&m.buttons, 0, sizeof(m.buttons));
	return m;
}

Keyboard
platform_reset_keyboard(Keyboard old_kb)
{
	for (int i = 0; i < MAX_SCANCODES; ++i) {
		old_kb.keys[i].pressed = 0;
		old_kb.keys[i].released = 0;
	}
	old_kb.num_keys_pressed_since_last_pull = 0;
	return old_kb;
}

*/
/*
bool
platform_was_key_pressed_at_all(const Digital_Button *keys, Key_Symbol ks)
{
	unsigned scancode = platform_keysym_to_scancode(ks);
	assert(scancode < MAX_SCANCODES);
	return keys[scancode].num_presses;
}

bool
platform_was_key_pressed_toggle(const Digital_Button *keys, Key_Symbol ks)
{
	unsigned scancode = platform_keysym_to_scancode(ks);
	assert(scancode < MAX_SCANCODES);
	return (keys[scancode].num_presses % 2) == 1;
}

bool
platform_is_key_down(const Digital_Button *keys, Key_Symbol ks)
{
	unsigned scancode = platform_keysym_to_scancode(ks);
	assert(scancode < MAX_SCANCODES);
	return keys[scancode].down;
}

*/

void get_mouse_xy(s32 *x, s32 *y) {
	s32 screen_x, screen_y;
	Window root, child;
	u32 mouse_buttons;
	//XIButtonState button_state;
	//XIModifierState mods;
	//XIGroupState group;

	//V2 old_position = mouse->position;

	XQueryPointer(linux_context.display, linux_context.window, &root, &child, &screen_x, &screen_y, x, y, &mouse_buttons);
	*y = (window_pixel_height - *y); // Bottom left is zero for us, top left is zero for x11.

	//XIQueryPointer(linux_context.display, 2, linux_context.window, &root, &child, &root_x, &root_y, &win_x, &win_y, &button_state, &mods, &group);
	//mouse->position.x = win_x;// * scaled_meters_per_pixel;
	//mouse->position.y = (window_pixel_height - win_y);// * scaled_meters_per_pixel; // Bottom left is zero for us, top left is zero for x11.

	//mouse->delta_position.x = old_position.x - mouse->position.x;
	//mouse->delta_position.y = old_position.y - mouse->position.y;

	//mouse->delta_position.x = mouse->position.x - old_window_x;
	//mouse->delta_position.y = mouse->position.x - old_window_y;
	// @NOTE: We can't set mouse buttons here because we would miss multiple button presses that happen in the span of a single update.
}


#include <stdio.h>

void render_resize_window();
template <u32 T> void press_button(u32 index, IO_Buttons<T> *buttons);
template <u32 T> void release_button(u32 index, IO_Buttons<T> *buttons);

static void parse_valuators(const double *input_values,unsigned char *mask,int mask_len,
                            double *output_values,int output_values_len) {
    int i = 0,z = 0;
    int top = mask_len * 8;
    //if (top > MAX_AXIS)
        //top = MAX_AXIS;

    memset(output_values,0,output_values_len * sizeof(double));
    for (; i < top && z < output_values_len; i++) {
        if (XIMaskIsSet(mask, i)) {
            const int value = (int) *input_values;
            output_values[z] = value;
            input_values++;
        }
        z++;
    }
}
void handle_platform_events(Game_Input *input, Game_Execution_Status *execution_status) {
#if 0
#if 0
	// Reset per-frame mouse state.
	for (s32 i = 0; i < NUM_MOUSE_BUTTONS; ++i) {
		input->mouse.buttons[i].pressed = input->mouse.buttons[i].released = 0;
	}

	// Reset per-frame keyboard state.
	for (s32 i = 1; i < MAX_SCANCODES; ++i) {
		input->keyboard[i].pressed  = 0;
		input->keyboard[i].released = 0;
	}
#endif
	//Window active;
	//int revert_to;
	//XGetInputFocus(linux_context.display, &active, &revert_to);
	//if (active == linux_context.window)
		//platform_get_mouse_position(&in->mouse.screen_x, &in->mouse.screen_y, &in->mouse.window_x, &in->mouse.window_y, &in->mouse.delta_screen_x, &in->mouse.delta_screen_y);

	//f32 delta_x = 0.0f, delta_y = 0.0f;
	//f64 delta_delta;
	//u8 mouse_moved = false;

	// @TODO: Set up an auto-pause when we lose focus?
	XFlush(linux_context.display);
	XEvent event;
	while (XPending(linux_context.display)) {
		XNextEvent(linux_context.display, &event);
		if (event.type == ClientMessage && (Atom)event.xclient.data.l[0] == linux_context.wm_delete_window) {
			*execution_status = GAME_EXITING;
		}

		switch(event.type) {
		case KeyPress: {
			press_button(event.xkey.keycode, &input->keyboard);
		} break;
		case KeyRelease: {
			if (XEventsQueued(linux_context.display, QueuedAfterReading)) {
				XEvent next_event;
				XPeekEvent(linux_context.display, &next_event);
				// X11 generates keyrelease and a keypress events continuously while the key is held down.
				// Detect this case and skip those events so that we only get a keypress and keyrelease when a key is
				// actually pressed and released.
				if (next_event.type == KeyPress && next_event.xkey.time == event.xkey.time && next_event.xkey.keycode == event.xkey.keycode) {
					XNextEvent(linux_context.display, &event);
					break;
				}
			}
			release_button(event.xkey.keycode, &input->keyboard);
		} break;
		case ButtonPress: {
				if ((event.xbutton.button - 1) < 0 || (event.xbutton.button - 1) > MOUSE_BUTTON_COUNT) {
					break;
				}
				press_button(event.xbutton.button - 1, &input->mouse.buttons);
		} break;
		case ButtonRelease: {
				if ((event.xbutton.button - 1) < 0 || (event.xbutton.button - 1) > MOUSE_BUTTON_COUNT) {
					break;
				}
				release_button(event.xbutton.button - 1, &input->mouse.buttons);
		} break;
		case ConfigureNotify: {
			XConfigureEvent xce = event.xconfigure;
			// This event type is generated for a variety of happenings, so check whether the window has been resized.
			if ((u32)xce.width != window_pixel_width || (u32)xce.height != window_pixel_height) {
			/*
				window_pixel_width         = (u32)xce.width;
				window_pixel_height        = (u32)xce.height;
				window_scaled_meter_width  = window_pixel_width * scaled_meters_per_pixel;
				window_scaled_meter_height = window_pixel_height * scaled_meters_per_pixel;

				render_resize_window();
			*/
			}
		} break;
		case FocusOut: {
			// @TODO: Clear all keyboard state on focus out. Our 'down' state isn't valid anymore because we won't get
			// the releases.
			//in->mouse = platform_clear_mouse(in->mouse); // Clear residual mouse motion so we don't keep using it in calculations.
		} break;
		case FocusIn: {
			//platform_get_mouse_position(&in->mouse.screen_x, &in->mouse.screen_y, &in->mouse.window_x, &in->mouse.window_y, &in->mouse.delta_screen_x, &in->mouse.delta_screen_y);
			//in->mouse.spatial = platform_pull_mouse_spatial(in->mouse.spatial); // Reset mouse position so the view doesn't "jump" when we regain focus.
		} break;
		}
	}

	//update_mouse_position(&input->mouse);
#endif
	XEvent ev;
	XGenericEventCookie *cookie = &ev.xcookie; // hacks!
	XIDeviceEvent *devev;

	XFlush(linux_context.display);
	while (XPending(linux_context.display)) {
		XNextEvent(linux_context.display, &ev);
		if (!XGetEventData(linux_context.display, cookie)) { // extended event
			continue;
		}
		// check if this belongs to XInput
		if(cookie->type == GenericEvent && cookie->extension == linux_context.xinput_opcode)
		{
			static int last = -1;

			devev = (XIDeviceEvent *)cookie->data;
			switch(devev->evtype) {
			case XI_RawMotion: {
			   static Time prev_time = 0;
			   static double prev_rel_coords[2];
				const XIRawEvent *rawev = (const XIRawEvent*)cookie->data;
				/*
				const double *dd = rawev->raw_values;
				int v1 = (int) *dd;
				dd++;
				int v2 = (int) *dd;
				double relative_coords[2] = {v1, v2};
				*/


				double relative_coords[2];
            parse_valuators(rawev->raw_values,rawev->valuators.mask,
                            rawev->valuators.mask_len,relative_coords,2);

				if ((rawev->time == prev_time) && (relative_coords[0] == prev_rel_coords[0]) && (relative_coords[1] == prev_rel_coords[1])) {
					//break;
				}
				prev_rel_coords[0] = relative_coords[0];
				prev_rel_coords[1] = relative_coords[1];
				prev_time = rawev->time;
				printf("%g %g\n", rawev->raw_values[0], rawev->raw_values[1]);
			} break;
			}
		}
	}
}

u8 write_file(File_Handle fh, size_t n, const void *buf) {
	size_t tot_writ = 0;
	ssize_t cur_writ = 0; // Maximum number of bytes that can be returned by a write. (Like size_t, but signed.)
	const char *pos = (char *)buf;

	do {
		cur_writ = write(fh, pos, (n - tot_writ));
		tot_writ += cur_writ;
		pos += cur_writ;
	} while (tot_writ < n && cur_writ != 0);

	if (tot_writ != n) {
		// @TODO: Add file name to file handle.
		log_print(MAJOR_ERROR_LOG, "Could not write to file: %s", get_platform_error());
		return false;
	}

	return true;
}

size_t format_string(const char *fmt, va_list arg_list, char *buf);

void debug_print(const char *fmt, va_list args) {
	char buf[4096];
	size_t nbytes_writ = format_string(fmt, args, buf);
	write_file(STDOUT, nbytes_writ, buf);
}

void debug_print(const char *fmt, ...) {
	char buf[4096];
	va_list arg_list;
	va_start(arg_list, fmt);
	size_t nbytes_writ = format_string(fmt, arg_list, buf);
	write_file(STDOUT, nbytes_writ, buf);
	va_end(arg_list);
}

void *open_shared_library(const char *filename) {
	void* library = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
	if (!library) {
		_abort("Failed to load shared library: %s", dlerror());
	}

	return library;
}

void close_shared_library(Library_Handle library) {
	s32 error_code = dlclose(library);
	if (error_code < 0) {
		log_print(MINOR_ERROR_LOG, "Failed to close shared library: %s\n", dlerror());
	}
}

void *load_shared_library_function(Library_Handle library, const char *function_name) {
	void *function = dlsym(library, function_name);
	if (!function) {
		_abort("Failed to load shared library function %s", function_name);
	}
}

// @TODO: Handle modes.
File_Handle open_file(const char *path, s32 flags) {
	File_Handle file_handle = open(path, flags, 0666);
	if (file_handle < 0) {
		log_print(MAJOR_ERROR_LOG, "Could not open file: %s", path);
		return FILE_HANDLE_ERROR;
	}
	return file_handle;
}

u8 close_file(File_Handle file_handle) {
	s32 result = close(file_handle);
	if (result == -1) {
		log_print(MINOR_ERROR_LOG, "Could not close file: %s", get_platform_error());
		return false;
	}
	return true;
}

//#include <errno.h>
//#include <string.h>
u8 read_file(File_Handle file_handle, size_t num_bytes_to_read, void *buffer) {
	size_t total_bytes_read = 0;
	ssize_t current_bytes_read = 0; // Maximum number of bytes that can be returned by a read. (Like size_t, but signed.)
	char *position = (char *)buffer;

	do {
		current_bytes_read = read(file_handle, position, num_bytes_to_read - total_bytes_read);
		total_bytes_read += current_bytes_read;
		position += current_bytes_read;
	} while (total_bytes_read < num_bytes_to_read && current_bytes_read != 0 && current_bytes_read != -1);

	if (current_bytes_read == -1) {
		log_print(MAJOR_ERROR_LOG, "Could not read from file: %s", get_platform_error());
		return false;
	} else if (total_bytes_read != num_bytes_to_read) {
		// @TODO: Add file name to file handle.
		log_print(MAJOR_ERROR_LOG, "Could only read %lu bytes, but %lu bytes were requested", total_bytes_read, num_bytes_to_read);
		return false;
	}

	return true;
}

File_Offset get_file_length(File_Handle file_handle) {
	struct stat stat; 
	if (fstat(file_handle, &stat) == 0) {
		return (File_Offset)stat.st_size;
	}

	return FILE_OFFSET_ERROR; 
}

File_Offset seek_file(File_Handle file_handle, File_Offset offset, File_Seek_Relative relative) {
	off_t result = lseek(file_handle, offset, relative);
	if (result == (off_t)-1) {
		log_print(MAJOR_ERROR_LOG, "File seek failed: %s", get_platform_error());
	}

	return result;
}

#if 0
void
platform_swap_buffers()
{
	glXSwapBuffers(linux_context.display, linux_context.window);
}

// 
// @TODO: Signal IO errors.
//


char *
platform_get_memory(size_t len)
{
	void *m = mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (m == (void *)-1)
		_abort("Failed to get memory from platform - %s.", perrno());
	return (char *)m;
}

void
platform_free_memory(void *m, size_t len)
{
	int ret = munmap(m, len);
	if (ret == -1)
		_abort("Failed to free memory from platform - %s.", perrno());
}

size_t
platform_get_page_size()
{
	return sysconf(_SC_PAGESIZE);
}

inline Time_Spec
platform_get_time()
{
	Time_Spec t;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	return t;
}

#include <math.h>

inline u32
platform_get_time_ms()
{
	Time_Spec t;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	return (t.tv_sec * 1000) + round(t.tv_nsec / 1.0e6);
}

inline u64
platform_get_time_us()
{
	Time_Spec t;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	return (t.tv_sec * 1000000) + round(t.tv_nsec / 1.0e3);
}

// Time in milliseconds.
inline long
platform_time_diff(Time_Spec start, Time_Spec end, unsigned resolution)
{
	assert(0);
	return (end.tv_nsec - start.tv_nsec) / resolution;
}

float
platform_get_seconds_elapsed(Time_Spec start, Time_Spec end)
{
	return (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1.0e9);
}

long
platform_keysym_to_codepoint(Key_Symbol keysym)
{
	// First check for Latin-1 characters (1:1 mapping).
	if ((keysym >= 0x0020 && keysym <= 0x007e) || (keysym >= 0x00a0 && keysym <= 0x00ff))
		return keysym;
	// Also check for directly encoded 24-bit unicode characters.
	if ((keysym & 0xff000000) == 0x01000000)
		return keysym & 0x00ffffff;
	// Do a search throught the keysym to unicode mapping table for our keysym.
	// @SPEED: Could make this a binary search or a hash table to improve latency.
	for (size_t i = 0; i < (sizeof(key_symbol_to_unicode) / sizeof(key_symbol_to_unicode[0])); ++i) {
		if (key_symbol_to_unicode[i].key_symbol == keysym)
			return key_symbol_to_unicode[i].unicode;
	}
	// No match.
	return -1;
}

void
platform_sleep(u32 milliseconds)
{
	struct timespec ts;

	ts.tv_sec  = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;

	int result = nanosleep(&ts, NULL);
	if (result) {
		log_print(MINOR_ERROR_LOG, "nanosleep() ended early -- %s.", perrno());
	}
}

#define READ_AND_ADVANCE_STREAM(type, stream) *((type *)stream); stream += sizeof(type);

struct Pcm_Playback_Info {
	u16 num_channels;
	u32 sample_rate;
	u32 bits_per_sample;
	u32 bytes_per_frame;
	u32 bytes_per_period;
	snd_pcm_uframes_t frames_per_period;
	struct pollfd fd;
} pcm_playback_info;

void
platform_pcm_open_device()
{
	snd_pcm_hw_params_t *pcm_parameters;
	s32                  dir;
	s32                  return_code;

	return_code = snd_pcm_open(&linux_context.pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (return_code < 0) {
		_abort("Unable to open pcm device: %s.", snd_strerror(return_code));
	}

	pcm_playback_info.num_channels = 2;
	pcm_playback_info.sample_rate = 44100;
	pcm_playback_info.bits_per_sample = 16;
	pcm_playback_info.bytes_per_frame = pcm_playback_info.num_channels * (pcm_playback_info.bits_per_sample / 8);

	snd_pcm_hw_params_alloca(&pcm_parameters);
	snd_pcm_hw_params_any(linux_context.pcm_handle, pcm_parameters);
	snd_pcm_hw_params_set_access(linux_context.pcm_handle, pcm_parameters, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(linux_context.pcm_handle, pcm_parameters, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(linux_context.pcm_handle, pcm_parameters, pcm_playback_info.num_channels);
	snd_pcm_hw_params_set_rate_near(linux_context.pcm_handle, pcm_parameters, &pcm_playback_info.sample_rate, &dir);

	pcm_playback_info.frames_per_period = 8192;
	snd_pcm_hw_params_set_period_size_near(linux_context.pcm_handle, pcm_parameters, &pcm_playback_info.frames_per_period, &dir);

	snd_pcm_uframes_t frames_per_buffer = pcm_playback_info.frames_per_period * 2;
	snd_pcm_hw_params_set_buffer_size(linux_context.pcm_handle, pcm_parameters, frames_per_buffer);

	return_code = snd_pcm_hw_params(linux_context.pcm_handle, pcm_parameters);
	if (return_code < 0) {
		_abort("Unable to set sound hardware parameters: %s\n", snd_strerror(return_code));
	}

	snd_pcm_hw_params_get_period_size(pcm_parameters, &pcm_playback_info.frames_per_period, &dir);
	snd_pcm_hw_params_get_period_time(pcm_parameters, &pcm_playback_info.sample_rate, &dir);

	pcm_playback_info.bytes_per_period = pcm_playback_info.frames_per_period * pcm_playback_info.bytes_per_frame;

	if(snd_pcm_poll_descriptors(linux_context.pcm_handle, &pcm_playback_info.fd, 1) < 0) {
		_abort("Error getting file descriptor for PCM.");
	}
}

u8
platform_pcm_less_than_one_period_left_in_buffer()
{
	s32 return_code = poll(&pcm_playback_info.fd, 1, 0);
	if (return_code == -1) {
		log_print(MINOR_ERROR_LOG, "Error polling PCM file descriptor -- %s.", perrno());
	}

	return return_code ? true : false;
}

s32
platform_pcm_write_period(s16 *period_buffer)
{
	s32 frames_written_this_call = snd_pcm_writei(linux_context.pcm_handle, period_buffer, pcm_playback_info.frames_per_period);
	if (frames_written_this_call == -EPIPE) {
		log_print(MINOR_ERROR_LOG, "PCM underrun occurred.\n");
		snd_pcm_prepare(linux_context.pcm_handle);
		return -1;
	} else if (frames_written_this_call < 0) {
		log_print(MINOR_ERROR_LOG, "Error from snd_pcm_writei: %s.\n", snd_strerror(frames_written_this_call));
		return -1;
	}

	if ((u32)frames_written_this_call != pcm_playback_info.frames_per_period) {
		log_print(MINOR_ERROR_LOG, "PCM short write, wrote %d frames and expected %d.\n", frames_written_this_call, (s32)pcm_playback_info.frames_per_period);
	}

	return frames_written_this_call;

		//wav_file += frames_written_this_call * bytes_per_frame;
		//frames_written += frames_written_this_call;
#if 0
	auto ma = mem_make_arena();
	String wav_file_string = read_entire_file("../data/sounds/speech.wav", &ma);
	const char *wav_file = wav_file_string.data;

	wav_file += 4;

	u32 wav_file_length = READ_AND_ADVANCE_STREAM(u32, wav_file);

	wav_file += 8;

	u32 format_data_length = READ_AND_ADVANCE_STREAM(u32, wav_file);
	u16 format_type        = READ_AND_ADVANCE_STREAM(u16, wav_file);
	u16 num_channels       = READ_AND_ADVANCE_STREAM(u16, wav_file);
	u32 sample_rate        = READ_AND_ADVANCE_STREAM(u32, wav_file);
	u32 bytes_per_second   = READ_AND_ADVANCE_STREAM(u32, wav_file);
	u16 bytes_per_frame    = READ_AND_ADVANCE_STREAM(u16, wav_file);
	u16 bits_per_sample    = READ_AND_ADVANCE_STREAM(u16, wav_file);

	char chunk_name[5] = {};
	strncpy(chunk_name, wav_file, 4);

	while (strcmp(chunk_name, "data") != 0) {
		wav_file += 4;
		u32 chunk_data_length = READ_AND_ADVANCE_STREAM(u32, wav_file);
		wav_file += chunk_data_length;

		strncpy(chunk_name, wav_file, 4);
	}

	wav_file += 4;

	u32 sample_data_length = READ_AND_ADVANCE_STREAM(u32, wav_file);

	printf("%u\n%u\n%u\n%u\n%u\n%u\n%u\n%u\n", format_data_length, format_type, num_channels, sample_rate, bytes_per_second, bytes_per_frame, bits_per_sample, sample_data_length); 

	s32 total_frames   = sample_data_length / bytes_per_frame;
	s32 frames_written = 0;

	while (frames_written < total_frames) {
		s32 frames_written_this_call = snd_pcm_writei(linux_context.pcm_handle, wav_file, pcm_playback_info.frames_per_period);
		if (frames_written_this_call == -EPIPE) {
			log_print(MINOR_ERROR_LOG, "PCM underrun occurred.\n");
			snd_pcm_prepare(linux_context.pcm_handle);
			// @TODO: Exit early?
		} else if (frames_written_this_call < 0) {
			log_print(MINOR_ERROR_LOG, "Error from snd_pcm_writei: %s.\n", snd_strerror(frames_written_this_call));
			// @TODO: Exit early?
		} else {
			if (frames_written_this_call != (s32)pcm_playback_info.frames_per_period) {
				log_print(MINOR_ERROR_LOG, "PCM short write, wrote %d frames and expected %d.\n", frames_written_this_call, (s32)pcm_playback_info.frames_per_period);
			}

			wav_file += frames_written_this_call * bytes_per_frame;
			frames_written += frames_written_this_call;
		}
	}

	snd_pcm_drain(linux_context.pcm_handle);
	snd_pcm_close(linux_context.pcm_handle);
#endif
}

void
platform_pcm_close_device()
{
	snd_pcm_drain(linux_context.pcm_handle);
	snd_pcm_close(linux_context.pcm_handle);
}

Thread_Handle
platform_create_thread(Thread_Procedure tp, void *thread_argument)
{
	pthread_attr_t attrs;
	if (pthread_attr_init(&attrs)) {
		_abort("Failed on pthread_attr_init(): %s", perrno());
	}

	pthread_t handle;
	if (pthread_create(&handle, &attrs, tp, thread_argument)) {
		_abort("Failed on pthread_create(): %s", perrno());
	}

	return handle;
}

Semaphore_Handle
platform_make_semaphore(u32 initial_value)
{
	sem_t s;
	sem_init(&s, 0, initial_value);
	return s;
}

void
platform_post_semaphore(sem_t *s)
{
	sem_post(s);
}

void
platform_wait_semaphore(sem_t *s)
{
	sem_wait(s);
}

s32
platform_get_semaphore_value(sem_t *s)
{
	s32 v;
	sem_getvalue(s, &v);
	return v;
}

void
platform_toggle_fullscreen()
{
	XEvent e;
	memset(&e, 0, sizeof(e));

	e.xclient.type         = ClientMessage;
	e.xclient.window       = linux_context.window;
	e.xclient.message_type = XInternAtom(linux_context.display, "_NET_WM_STATE", True);
	e.xclient.format = 32;
	e.xclient.data.l[0] = 2;
	e.xclient.data.l[1] = XInternAtom(linux_context.display, "_NET_WM_STATE_FULLSCREEN", True);
	e.xclient.data.l[2] = 0;
	e.xclient.data.l[3] = 1;
	e.xclient.data.l[4] = 0;

	XSendEvent(linux_context.display, DefaultRootWindow(linux_context.display), False, SubstructureRedirectMask | SubstructureNotifyMask, &e);
}
#endif
