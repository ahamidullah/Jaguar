#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define STDOUT 1
#define STDIN 0
#define STDERR 2

#define THREAD_LOCAL __thread

typedef struct {
	ucontext_t context;
	jmp_buf jump_buffer;
} Fiber;

typedef s32 File;
typedef u64 File_Offset;
typedef struct timespec Platform_Time;
typedef void * Shared_Library;
typedef pthread_t Thread;
typedef void *(*Thread_Procedure)(void *);
typedef void (*Fiber_Procedure)(void *);
typedef sem_t Semaphore;

typedef enum {
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

typedef enum {
	MOUSE_BUTTON_LEFT = 0,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
} Mouse_Button;

typedef enum {
	FILE_SEEK_START = SEEK_SET,
	FILE_SEEK_CURRENT = SEEK_CUR,
	FILE_SEEK_END = SEEK_END
} File_Seek_Relative;

File FILE_HANDLE_ERROR = -1;
File_Offset FILE_OFFSET_ERROR = (File_Offset)-1;

#ifdef DEBUG
#define ASSERT(x)\
	do {\
		if (!(x)) {\
			log_print(CRITICAL_ERROR_LOG, "%s: %s: line %d: assertion failed '%s'\n", __FILE__, __func__, __LINE__, #x);\
			platform_print_stacktrace();\
			raise(SIGILL);\
		}\
	} while(0)
#else
#define ASSERT(x)
#endif

typedef struct {
	DIR *dir;
	struct dirent *dirent;
	char *filename;
	u8 is_directory;
} Directory_Iteration;

// Memory.
void *platform_allocate_memory(size_t size);
void platform_free_memory(void *memory, size_t size);
size_t platform_get_page_size();
void platform_print_stacktrace();

// Input.
void platform_get_mouse_position(s32 *x, s32 *y);
u32 platform_key_symbol_to_scancode(Key_Symbol key_symbol);

// Filesystem.
u8 platform_iterate_through_all_files_in_directory(const char *path, Directory_Iteration *context);
File platform_open_file(const char *path, s32 flags);
u8 platform_close_file(File file);
u8 platform_read_file(File file, size_t num_bytes_to_read, void *buffer);
File_Offset platform_get_file_length(File file);
File_Offset platform_seek_file(File file, File_Offset offset, File_Seek_Relative relative);
u8 platform_write_file(File file, size_t count, const void *buffer);

// Window.
void platform_toggle_fullscreen();
void platform_capture_cursor();
void platform_uncapture_cursor();
void platform_cleanup_display();

// Shared libraries.
Shared_Library platform_open_shared_library(const char *filename);
void platform_close_shared_library(Shared_Library library);
void *platform_load_shared_library_function(Shared_Library library, const char *function_name);

// Time.
Platform_Time platform_get_current_time();
f64 platform_time_difference(Platform_Time start, Platform_Time end);
void Platform_Sleep(u32 milliseconds);

// Events.
void platform_handle_events(Game_Input *input, Game_Execution_Status *execution_status);
void platform_signal_debug_breakpoint();

// Vulkan.
const char *platform_get_required_vulkan_surface_instance_extension();
void platform_create_vulkan_surface(VkInstance instance, VkSurfaceKHR *surface);

// Errors.
const char *platform_get_error();

// Threads.
s32 Platform_Get_Processor_Count();
Thread Platform_Create_Thread(Thread_Procedure procedure, void *parameter);
void Platform_Set_Thread_Processor_Affinity(Thread thread, u32 cpu_index);
Thread Platform_Get_Current_Thread();

// Fibers.
void Platform_Create_Fiber(Fiber *fiber, Fiber_Procedure procedure, void *parameter);
void Platform_Switch_To_Fiber(Fiber *fiber);
void Platform_Convert_Thread_To_Fiber(Fiber *fiber);
Fiber *Platform_Get_Current_Fiber();

// Semaphores.
Semaphore Platform_Create_Semaphore(u32 initial_value);
void Platform_Post_Semaphore(sem_t *semaphore);
void Platform_Wait_Semaphore(sem_t *semaphore);
s32 Platform_Get_Semaphore_Value(sem_t *semaphore);

// Atomics.
s32 Platform_Atomic_Add_S32(volatile s32 *operand, s32 addend);
s64 platform_atomic_add_s64(volatile s64 *operand, s64 addend);
s32 Platform_Compare_And_Swap_S32(volatile s32 *destination, s32 old_value, s32 new_value);
s64 platform_compare_and_swap_s64(volatile s64 *destination, s64 old_value, s64 new_value);
void *Platform_Compare_And_Swap_Pointers(void *volatile *target, void *old_value, void *new_value);
void *Platform_Fetch_And_Set_Pointer(void *volatile *target, void *value);
