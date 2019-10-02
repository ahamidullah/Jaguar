#define PROCESS_EXIT_FAILURE 1
#define PROCESS_EXIT_SUCCESS 0

#define STDOUT 1
#define STDIN 0
#define STDERR 2

#define THREAD_LOCAL __thread

typedef struct {
	ucontext_t context;
	jmp_buf jump_buffer;
} Platform_Fiber;

typedef s32 Platform_File_Handle;
typedef u64 Platform_File_Offset;
typedef struct timespec Platform_Time;
typedef void *Platform_Dynamic_Library_Handle;
typedef void *Platform_Dynamic_Library_Function;
typedef pthread_t Platform_Thread_Handle;
typedef void *(*Platform_Thread_Procedure)(void *);
typedef void (*Platform_Fiber_Procedure)(void *);
typedef sem_t Platform_Semaphore;
typedef pthread_mutex_t Platform_Mutex;

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
} Platform_Key_Symbol;

typedef enum {
	MOUSE_BUTTON_LEFT = 0,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
} Platform_Mouse_Button;

typedef enum {
	FILE_SEEK_START = SEEK_SET,
	FILE_SEEK_CURRENT = SEEK_CUR,
	FILE_SEEK_END = SEEK_END
} Platform_File_Seek_Relative;

typedef enum Platform_Open_File_Flags {
	OPEN_FILE_READ_ONLY = O_RDONLY,
} Platform_Open_File_Flags;

Platform_File_Handle PLATFORM_FILE_HANDLE_ERROR = -1;
Platform_File_Offset PLATFORM_FILE_OFFSET_ERROR = (Platform_File_Offset)-1;

typedef struct {
	DIR *dir;
	struct dirent *dirent;
	char *filename;
	u8 is_directory;
} Platform_Directory_Iteration;

// Memory.
void *Platform_Allocate_Memory(size_t size);
void Platform_Free_Memory(void *memory, size_t size);
size_t Platform_Get_Page_Size();
void Platform_Print_Stacktrace();

// Process.
void Platform_Exit_Process(s32 return_code);
void Platform_Signal_Debug_Breakpoint();

// Input.
void Platform_Get_Mouse_Position(s32 *x, s32 *y);
u32 Platform_Key_Symbol_To_Scancode(Platform_Key_Symbol key_symbol);

// Filesystem.
u8 Platform_Iterate_Through_All_Files_In_Directory(const char *path, Platform_Directory_Iteration *context);
Platform_File_Handle Platform_Open_File(const char *path, Platform_Open_File_Flags flags);
u8 Platform_Close_File(Platform_File_Handle file);
u8 Platform_Read_From_File(Platform_File_Handle file, size_t num_bytes_to_read, void *buffer);
Platform_File_Offset Platform_Get_File_Length(Platform_File_Handle file);
Platform_File_Offset Platform_Seek_In_File(Platform_File_Handle file, Platform_File_Offset offset, Platform_File_Seek_Relative relative);
u8 Platform_Write_To_File(Platform_File_Handle file, size_t count, const void *buffer);

// Window.
void Platform_Toggle_Fullscreen();
void Platform_Capture_Cursor();
void Platform_Uncapture_Cursor();
void Platform_Cleanup_Display();
void Platform_Handle_Window_Events(Game_Input *input, Game_Execution_Status *execution_status);

// Dynamic libraries.
Platform_Dynamic_Library_Handle Platform_Open_Dynamic_Library(const char *filename);
void Platform_Close_Dynamic_Library(Platform_Dynamic_Library_Handle library);
Platform_Dynamic_Library_Function Platform_Get_Dynamic_Library_Function(Platform_Dynamic_Library_Handle library, const char *function_name);

// Time.
Platform_Time Platform_Get_Current_Time();
f64 Platform_Time_Difference(Platform_Time start, Platform_Time end);
void Platform_Sleep(u32 milliseconds);

// Vulkan.
const char *Platform_Get_Required_Vulkan_Surface_Instance_Extension();
void Platform_Create_Vulkan_Surface(VkInstance instance, VkSurfaceKHR *surface);

// Errors.
const char *Platform_Get_Error();

// Threads.
s32 Platform_Get_Processor_Count();
Platform_Thread_Handle Platform_Create_Thread(Platform_Thread_Procedure procedure, void *parameter);
void Platform_Set_Thread_Processor_Affinity(Platform_Thread_Handle thread, u32 cpu_index);
Platform_Thread_Handle Platform_Get_Current_Thread();
u32 Platform_Get_Current_Thread_ID();
void Platform_Create_Mutex(Platform_Mutex *mutex);
void Platform_Lock_Mutex(Platform_Mutex *mutex);
void Platform_Unlock_Mutex(Platform_Mutex *mutex);

// Fibers.
void Platform_Create_Fiber(Platform_Fiber *fiber, Platform_Fiber_Procedure procedure, void *parameter);
void Platform_Switch_To_Fiber(Platform_Fiber *fiber);
void Platform_Convert_Thread_To_Fiber(Platform_Fiber *fiber);
Platform_Fiber *Platform_Get_Current_Fiber();

// Semaphores.
Platform_Semaphore Platform_Create_Semaphore(u32 initial_value);
void Platform_Post_Semaphore(Platform_Semaphore *semaphore);
void Platform_Wait_Semaphore(Platform_Semaphore *semaphore);
s32 Platform_Get_Semaphore_Value(Platform_Semaphore *semaphore);

// Atomics.
s32 Platform_Atomic_Add_S32(volatile s32 *operand, s32 addend);
s64 platform_atomic_add_s64(volatile s64 *operand, s64 addend);
s32 Platform_Atomic_Fetch_And_Add_S32(volatile s32 *operand, s32 addend);
s32 Platform_Atomic_Fetch_And_Add_S64(volatile s64 *operand, s64 addend);
s32 Platform_Compare_And_Swap_S32(volatile s32 *destination, s32 old_value, s32 new_value);
s64 Platform_Compare_And_Swap_S64(volatile s64 *destination, s64 old_value, s64 new_value);
void *Platform_Compare_And_Swap_Pointers(void *volatile *target, void *old_value, void *new_value);
void *Platform_Fetch_And_Set_Pointer(void *volatile *target, void *value);
