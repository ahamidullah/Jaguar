#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "jaguar.h"

// TEMPORARY, GET RID OF ME. /////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
//////////////////////////////////////////////////////////////

#ifdef DEBUG
	const u8 debug = 1;
#else
	const u8 debug = 0;
#endif

#include "linux.c"
#include "memory.c"
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
	Game_State game_state = {
		.execution_status = GAME_RUNNING,
	};

	initialize_memory(&game_state);
	initialize_assets(&game_state);
	initialize_input(&game_state);
	initialize_camera(&game_state.camera, (V3){2, 2, 2}, (V3){1, 1, 1}, 0.4f, DEGREES_TO_RADIANS(90.0f));
	initialize_renderer(&game_state.camera, &game_state.permanent_arena, &game_state.frame_arena);
	initialize_entities(&game_state.entities, &game_state.assets, &game_state.frame_arena);
	clear_memory_arena(&game_state.frame_arena);

	//draw_sphere(game_state.entities.mesh_bounding_spheres[0].center, game_state.entities.mesh_bounding_spheres[0].radius, (V4){1.0f, 0.0f, 0.0f, 0.7f});

	while (game_state.execution_status != GAME_EXITING) {
		update(&game_state);

		render(&game_state);

		clear_memory_arena(&game_state.frame_arena);
	}

	cleanup_renderer();
}
