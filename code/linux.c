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
#include <execinfo.h>
#include <dirent.h>

#define STDOUT 1
#define STDIN 0
#define STDERR 2

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

void debug_print(const char *fmt, ...);
void print_stacktrace();

#define ASSERT(x)\
	do {\
		if (!(x)) {\
			debug_print("%s: %s: line %d: assertion failed '%s'\n", __FILE__, __func__, __LINE__, #x);\
			print_stacktrace();\
			raise(SIGILL);\
		}\
	} while(0)

void signal_debug_breakpoint() {
	raise(SIGTRAP);
}

typedef enum Key_Symbol {
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
} Key_Symbol;

typedef enum Mouse_Button {
	MOUSE_BUTTON_LEFT = 0,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
} Mouse_Button;

typedef enum File_Seek_Relative {
	FILE_SEEK_START = SEEK_SET,
	FILE_SEEK_CURRENT = SEEK_CUR,
	FILE_SEEK_END = SEEK_END
} File_Seek_Relative;

typedef s32       File_Handle;
typedef pthread_t Thread_Handle;
typedef struct timespec  Platform_Time;
typedef sem_t     Semaphore_Handle;
typedef void *    Library_Handle;
typedef u64       File_Offset;

File_Handle FILE_HANDLE_ERROR = -1;
File_Offset FILE_OFFSET_ERROR = (File_Offset)-1;

u32 window_width, window_height;
//f32 aspect_ratio;

// @TODO: Store colormap and free it on exit.
// @TODO: Free blank_cursor.
struct Linux_Context {
	Display   *display;
	Window     window;
	Atom       wm_delete_window;
	s32        xinput_opcode;
	Cursor     blank_cursor;
	//snd_pcm_t *pcm_handle;
} linux_context;

void platform_exit(int exit_code);
u8 platform_write_file(File_Handle fh, size_t n, const void *buf);

const char *get_platform_error() {
	return strerror(errno);
}

s32 x11_error_handler(Display *display, XErrorEvent *event) {
	char buffer[256];
	XGetErrorText(linux_context.display, event->error_code, buffer, sizeof(buffer));
	_abort("X11 error: %s.", buffer);
	return 0;
}

#if 0
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

s32 main(s32 argc, char **argv) {
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

	s32 screen = XDefaultScreen(linux_context.display);
	Window root_window = XRootWindow(linux_context.display, screen);

	// Initialize XInput2, which we require for raw input.
	{
		if (!XQueryExtension(linux_context.display, "XInputExtension", &linux_context.xinput_opcode, &(s32){0}, &(s32){0})) {
			_abort("The X server does not support the XInput extension");
		}
		// We are supposed to pass in the minimum version we require to XIQueryVersion, and it passes back what version is available.
		s32 major_version = 2, minor_version = 0;
		XIQueryVersion(linux_context.display, &major_version, &minor_version);
		if (major_version < 2) {
			_abort("XInput version 2.0 or greater is required: version %d.%d is available", major_version, minor_version);
		}
		u8 mask[] = {0, 0, 0};
		XIEventMask event_mask = {
			.deviceid = XIAllMasterDevices,
			.mask_len = sizeof(mask),
			.mask = mask,
		};
		XISetMask(mask, XI_RawMotion);
		XISetMask(mask, XI_RawButtonPress);
		XISetMask(mask, XI_RawButtonRelease);
		XISetMask(mask, XI_RawKeyPress);
		XISetMask(mask, XI_RawKeyRelease);
		XISetMask(mask, XI_FocusOut);
		XISetMask(mask, XI_FocusIn);
		if (XISelectEvents(linux_context.display, root_window, &event_mask, 1) != Success) {
			_abort("Failed to select XInput events");
		}
	}

	// Create window.
	{
		XVisualInfo visual_info_template = {};
		visual_info_template.screen = screen;

		s32 number_of_visuals;
		XVisualInfo *visual_info = XGetVisualInfo(linux_context.display, VisualScreenMask, &visual_info_template, &number_of_visuals);
		ASSERT(visual_info->class == TrueColor);

		XSetWindowAttributes window_attributes = {};
		window_attributes.colormap = XCreateColormap(linux_context.display, root_window, visual_info->visual, AllocNone);
		window_attributes.background_pixel = 0xFFFFFFFF;
		window_attributes.border_pixmap = None;
		window_attributes.border_pixel = 0;
		window_attributes.event_mask = StructureNotifyMask;

		s32 window_attributes_mask = CWBackPixel
		                           | CWColormap
		                           | CWBorderPixel
		                           | CWEventMask;

		s32 requested_window_width = 1200;
		s32 requested_window_height = 1000;

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
		if (XGetGeometry(linux_context.display, linux_context.window, &root_window, &win_x, &win_y, &window_width, &window_height, &border_width, &depth) == 0) {
			_abort("Failed to get the screen's geometry.");
		}
		//aspect_ratio = ((f32)window_width / (f32)window_height);
	}

	// Create a blank cursor for when we want to hide the cursor.
	{
		XColor xcolor;
		static char cursor_pixels[] = {0x00};
		Pixmap pixmap = XCreateBitmapFromData(linux_context.display, linux_context.window, cursor_pixels, 1, 1);
		linux_context.blank_cursor = XCreatePixmapCursor(linux_context.display, pixmap, pixmap, &xcolor, &xcolor, 1, 1); 
		XFreePixmap(linux_context.display, pixmap);
	}

	application_entry();

	//platform_exit(EXIT_SUCCESS);

	return 0;
}

void cleanup_platform_display() {
	XDestroyWindow(linux_context.display, linux_context.window);
	XCloseDisplay(linux_context.display);
}

u32 key_symbol_to_scancode(Key_Symbol key_symbol) {
	u32 scancode = XKeysymToKeycode(linux_context.display, key_symbol);
	ASSERT(scancode > 0);
	return scancode;
}

void get_mouse_position(s32 *x, s32 *y) {
	s32 screen_x, screen_y;
	Window root, child;
	u32 mouse_buttons;
	XQueryPointer(linux_context.display, linux_context.window, &root, &child, &screen_x, &screen_y, x, y, &mouse_buttons);
	*y = (window_height - *y); // Bottom left is zero for us, top left is zero for x11.
}

void render_resize_window();
void press_button(u32 index, IO_Buttons *buttons);
void release_button(u32 index, IO_Buttons *buttons);

void handle_platform_events(Game_Input *input, Game_Execution_Status *execution_status) {
	XEvent event;
	XGenericEventCookie *cookie = &event.xcookie;
	XIRawEvent *raw_event;

	XFlush(linux_context.display);
	while (XPending(linux_context.display)) {
		XNextEvent(linux_context.display, &event);

		if (event.type == ClientMessage && (Atom)event.xclient.data.l[0] == linux_context.wm_delete_window) {
			*execution_status = GAME_EXITING;
			break;
		}
		if (event.type == ConfigureNotify) {
			XConfigureEvent configure_event = event.xconfigure;
			// @TODO Window resize.
			break;
		}

		if (!XGetEventData(linux_context.display, cookie)
		 || cookie->type != GenericEvent
		 || cookie->extension != linux_context.xinput_opcode) {
			continue;
		}

		raw_event = (XIRawEvent *)cookie->data;

		switch(raw_event->evtype) {
		case XI_RawMotion: {
			// @TODO: Check XIMaskIsSet(re->valuators.mask, 0) for x and XIMaskIsSet(re->valuators.mask, 1) for y.
			input->mouse.raw_delta_x += raw_event->raw_values[0];
			input->mouse.raw_delta_y -= raw_event->raw_values[1];
		} break;
		case XI_RawKeyPress: {
			press_button(raw_event->detail, &input->keyboard);
		} break;
		case XI_RawKeyRelease: {
			release_button(raw_event->detail, &input->keyboard);
		} break;
		case XI_RawButtonPress: {
			u32 button_index = (event.xbutton.button - 1);
			if (button_index > MOUSE_BUTTON_COUNT) {
				break;
			}
			press_button(button_index, &input->mouse.buttons);
		} break;
		case XI_RawButtonRelease: {
			u32 button_index = (event.xbutton.button - 1);
			if (button_index > MOUSE_BUTTON_COUNT) {
				break;
			}
			release_button(button_index, &input->mouse.buttons);
		} break;
		case XI_FocusIn: {
		} break;
		case XI_FocusOut: {
		} break;
		}
	}
}

u8 write_file(File_Handle file_handle, size_t count, const void *buffer) {
	size_t total_bytes_written = 0;
	ssize_t current_bytes_written = 0; // Maximum number of bytes that can be returned by a write. (Like size_t, but signed.)
	const char *position = (char *)buffer;

	do {
		current_bytes_written = write(file_handle, position, (count - total_bytes_written));
		total_bytes_written += current_bytes_written;
		position += current_bytes_written;
	} while (total_bytes_written < count && current_bytes_written != 0);

	if (total_bytes_written != count) {
		// @TODO: Add file name to file handle.
		log_print(MAJOR_ERROR_LOG, "Could not write to file: %s", get_platform_error());
		return 0;
	}
	return 1;
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
	return function;
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
		return 0;
	}
	return 1;
}

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
		return 0;
	} else if (total_bytes_read != num_bytes_to_read) {
		// @TODO: Add file name to file handle.
		log_print(MAJOR_ERROR_LOG, "Could only read %lu bytes, but %lu bytes were requested", total_bytes_read, num_bytes_to_read);
		return 0;
	}

	return 1;
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

typedef struct {
	DIR *dir;
	struct dirent *dirent;
	char *filename;
	u8 is_directory;
} Directory_Iteration;

u8 iterate_through_all_files_in_directory(const char *path, Directory_Iteration *context) {
	if (!context->dir) { // First read.
		context->dir = opendir(path);
		if (!context->dir) {
			log_print(MAJOR_ERROR_LOG, "Failed to open animation directory %s: %s\n", path, strerror(errno));
			return 0;
		}
	}
	while ((context->dirent = readdir(context->dir))) {
		if (!strcmp(context->dirent->d_name, ".") || !strcmp(context->dirent->d_name, "..")) {
			continue;
		}
		context->filename = context->dirent->d_name;
		context->is_directory = (context->dirent->d_type == DT_DIR);
		return 1;
	}
	return 0;
}

void capture_cursor() {
	XDefineCursor(linux_context.display, linux_context.window, linux_context.blank_cursor);
	XGrabPointer(linux_context.display, linux_context.window, True, 0, GrabModeAsync, GrabModeAsync, None, linux_context.blank_cursor, CurrentTime);
}

void uncapture_cursor() {
	XUndefineCursor(linux_context.display, linux_context.window);
	XUngrabPointer(linux_context.display, CurrentTime);
}

Platform_Time get_current_platform_time() {
	Platform_Time time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return time;
}

// Time in milliseconds.
f64 platform_time_difference(Platform_Time start, Platform_Time end) {
	return ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_nsec - start.tv_nsec) / 1000000.0);
}

void print_stacktrace() {
	debug_print("Stack trace:\n");
	const u32 address_buffer_size = 100;
	void *addresses[address_buffer_size];
	s32 address_count = backtrace(addresses, address_buffer_size);
	if (address_count == address_buffer_size) {
		debug_print("Stack trace is probably truncated.\n");
	}
	char **function_names = backtrace_symbols(addresses, address_count);
	if (!function_names) {
		debug_print("Failed to get function names");
		return;
	}
	for (s32 i = 0; i < address_count; i++) {
		debug_print("\t%s\n", function_names[i]);
	}
}

#define MAP_ANONYMOUS 0x20

void *acquire_platform_memory(size_t size) {
	void *result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (result == (void *)-1) {
		_abort("Failed to get memory from platform: %s", get_platform_error());
	}

	return result;
}

void release_platform_memory(void *memory, size_t size) {
	s32 return_code = munmap(memory, size);
	if (return_code == -1) {
		_abort("Failed to free memory: %s", get_platform_error());
	}
}

void platform_toggle_fullscreen() {
	XEvent event;
	memset(&event, 0, sizeof(event));
	event.xclient.type         = ClientMessage;
	event.xclient.window       = linux_context.window;
	event.xclient.message_type = XInternAtom(linux_context.display, "_NET_WM_STATE", True);
	event.xclient.format       = 32;
	event.xclient.data.l[0]    = 2;
	event.xclient.data.l[1]    = XInternAtom(linux_context.display, "_NET_WM_STATE_FULLSCREEN", True);
	event.xclient.data.l[2]    = 0;
	event.xclient.data.l[3]    = 1;
	event.xclient.data.l[4]    = 0;
	XSendEvent(linux_context.display, DefaultRootWindow(linux_context.display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
}

#if 0
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
	ASSERT(0);
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

#endif
