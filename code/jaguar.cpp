#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "jaguar.h"

// TEMPORARY. GET RID OF ME.
#include <stdlib.h>
#include <math.h>
////////////////////////////

#ifdef DEBUG
	const u8 debug = true;
#else
	const u8 debug = false;
#endif

#include "linux.cpp"
#include "memory.cpp"
#include "math.cpp"
#include "library.cpp"
#include "vulkan.cpp"

void application_entry() {
	initialize_memory();
	initialize_vulkan();

	Input input = {};

	Execution_State execution_state = RUNNING_STATE;
	while (execution_state != EXITING_STATE) {
		execution_state = handle_platform_events(&input, execution_state);

		render();

		clear_memory_arena(&temporary_memory_arena);

		vkDeviceWaitIdle(vulkan_context.device);
	}

	vulkan_cleanup();
}
