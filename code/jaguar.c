#include "jaguar.h"

u32 window_width, window_height;

#include "memory.c"
#include "jobs.c"
#include "math.c"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#include "strings.c"
#include "filesystem.c"
#include "log.c"
#include "timer.c"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vulkan.c"
#include "renderer.c"
#include "assets.c"
#include "input.c"
#include "camera.c"
#include "entities.c"

void update(Game_State *game_state) {
	update_input(&game_state->input, &game_state->execution_status);
	update_camera(&game_state->camera, &game_state->input);
}

void application_entry() {
	Game_State game_state = {};
	initialize_memory(&game_state);
	Initialize_Jobs(&game_state.permanent_arena);
	initialize_assets(&game_state);
	initialize_input(&game_state);
	initialize_camera(&game_state.camera, (V3){2, 2, 2}, (V3){1, 1, 1}, 0.4f, DEGREES_TO_RADIANS(90.0f));
	initialize_renderer(&game_state.camera, &game_state.permanent_arena, &game_state.frame_arena);
	initialize_entities(&game_state.entities, &game_state.assets, &game_state.frame_arena);
	clear_memory_arena(&game_state.frame_arena);

	while (game_state.execution_status != GAME_EXITING) {
		update(&game_state);
		render(&game_state);
		clear_memory_arena(&game_state.frame_arena);
	}

	cleanup_renderer();
}
