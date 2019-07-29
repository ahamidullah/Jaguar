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
#include "assets.cpp"
#include "input.cpp"
#include "camera.cpp"

void update(Game_State *game_state) {
	update_input(&game_state->input, &game_state->execution_status);
	update_camera(&game_state->camera, &game_state->input);
}

void application_entry() {
	Game_State game_state = {};
	game_state.execution_status = GAME_RUNNING;

	initialize_memory(&game_state);
	initialize_renderer(&game_state);
	initialize_assets(&game_state);
	initialize_input(&game_state);
	initialize_camera(&game_state.camera, {2, 2, 2}, {1, 1, 1}, 1);

	//build_vulkan_command_buffers(&game_state.assets);

	while (game_state.execution_status != GAME_EXITING) {
		update(&game_state);

		render(&game_state);

		clear_memory_arena(&game_state.frame_arena);
	}

	cleanup_renderer();
}
