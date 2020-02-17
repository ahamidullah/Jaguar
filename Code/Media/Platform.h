#pragma once

#include "Common/Common.h"

#if defined(__linux__)
	#include "Platform/Linux/Platform.h"
#else
	#error unsupported platform
#endif

void PlatformExitProcess(s32 returnCode);
void PlatformSignalDebugBreakpoint();
const char *PlatformGetError();

#if 0
#include "Common.h"
#include "Strings.h"

#if defined(__linux__)
	#include "Linux.h"
#else
	#error unsupported platform
#endif

struct PlatformOpenFileResult {
	PlatformFileHandle file;
	bool error;
};

struct PlatformReadFileResult {
	String string;
	bool error;
};

// Memory.
void *PlatformAllocateMemory(size_t size);
void PlatformFreeMemory(void *memory, size_t size);
size_t PlatformGetPageSize();
void PlatformPrintStacktrace();

// Process.
void PlatformExitProcess(s32 returnCode);
void PlatformSignalDebugBreakpoint();

// Input.
void PlatformGetMousePosition(s32 *x, s32 *y);
u32 PlatformKeySymbolToScancode(PlatformKeySymbol keySymbol);

// Filesystem.
bool PlatformIterateThroughDirectory(const char *path, PlatformDirectoryIteration *context);
PlatformOpenFileResult PlatformOpenFile(const String &Path, PlatformOpenFileFlags Flags);
bool PlatformCloseFile(PlatformFileHandle file);
PlatformReadFileResult PlatformReadFromFile(PlatformFileHandle File, size_t NumberOfBytesToRead);
PlatformFileOffset PlatformGetFileLength(PlatformFileHandle File);
PlatformFileOffset PlatformSeekInFile(PlatformFileHandle file, PlatformFileOffset offset, PlatformFileSeekRelative relative);
bool PlatformWriteToFile(PlatformFileHandle file, size_t count, const void *buffer);

// Window.
void PlatformToggleFullscreen();
void PlatformCaptureCursor();
void PlatformUncaptureCursor();
void PlatformCleanupDisplay();
void PlatformHandleWindowEvents(Game_Input *input, GameExecutionStatus *executionStatus);

// Dynamic libraries.
PlatformDynamicLibraryHandle PlatformOpenDynamicLibrary(const char *filename);
void PlatformCloseDynamicLibrary(PlatformDynamicLibraryHandle library);
PlatformDynamicLibraryFunction PlatformGetDynamicLibraryFunction(PlatformDynamicLibraryHandle library, const char *functionName);

// Time.
PlatformTime PlatformGetCurrentTime();
f64 PlatformTimeDifference(PlatformTime start, PlatformTime end);
void PlatformSleep(u32 milliseconds);

// Vulkan.
#if defined(USE_VULKAN_RENDER_API)
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h> 
const char *PlatformGetRequiredVulkanSurfaceInstanceExtension();
void PlatformCreateVulkanSurface(VkInstance instance, VkSurfaceKHR *surface);
#endif

// Errors.
const char *PlatformGetError();

// Threads.
s32 PlatformGetProcessorCount();
PlatformThreadHandle PlatformCreateThread(PlatformThreadProcedure procedure, void *parameter);
void PlatformSetThreadProcessorAffinity(PlatformThreadHandle thread, u32 cpuIndex);
PlatformThreadHandle PlatformGetCurrentThread();
u32 PlatformGetCurrentThreadID();
void PlatformCreateMutex(PlatformMutex *mutex);
void PlatformLockMutex(PlatformMutex *mutex);
void PlatformUnlockMutex(PlatformMutex *mutex);

// Fibers.
void PlatformCreateFiber(PlatformFiber *fiber, PlatformFiberProcedure procedure, void *parameter);
void PlatformSwitchToFiber(PlatformFiber *fiber);
void PlatformConvertThreadToFiber(PlatformFiber *fiber);
PlatformFiber *PlatformGetCurrentFiber();

// Semaphores.
PlatformSemaphore PlatformCreateSemaphore(u32 initialValue);
void PlatformSignalSemaphore(PlatformSemaphore *semaphore);
void PlatformWaitOnSemaphore(PlatformSemaphore *semaphore);
s32 PlatformGetSemaphoreValue(PlatformSemaphore *semaphore);

// Atomics.
s32 PlatformAtomicAddS32(volatile s32 *operand, s32 addend);
s64 PlatformAtomicAddS64(volatile s64 *operand, s64 addend);
s32 PlatformAtomicFetchAndAddS32(volatile s32 *operand, s32 addend);
s32 PlatformAtomicFetchAndAddS64(volatile s64 *operand, s64 addend);
s32 PlatformCompareAndSwapS32(volatile s32 *destination, s32 oldValue, s32 newValue);
s64 PlatformCompareAndSwapS64(volatile s64 *destination, s64 oldValue, s64 newValue);
void *PlatformCompareAndSwapPointers(void *volatile *target, void *oldValue, void *newValue);
void *PlatformFetchAndSetPointer(void *volatile *target, void *value);
#endif
