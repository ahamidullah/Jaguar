#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "jaguar.h"
#include "memory.cpp"
#include "library.cpp"
#include "math.cpp"
#include "vulkan.cpp"
//#include "font.cpp"
#include "input.cpp"
#include "camera.cpp"
//#include "entities.cpp"
//#include "debug.cpp"

#include <unistd.h>

void update(Game_State *game_state) {
	update_input(&game_state->input, &game_state->execution_status);
	update_camera(&game_state->camera, &game_state->input);
}

s32 main(s32 argc, char *argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		_abort("SDL could not initialize: %s", SDL_GetError());
	}

	u8 fullscreen = false;

	u32 window_flags = SDL_WINDOW_VULKAN
	                 | SDL_WINDOW_SHOWN
	                 | SDL_WINDOW_RESIZABLE
	                 | SDL_WINDOW_ALLOW_HIGHDPI;

	if (fullscreen) {
		window_flags |= SDL_WINDOW_BORDERLESS;
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	s32 request_window_width = 800;
	s32 request_window_height = 600;

	SDL_Window *window = SDL_CreateWindow("cge", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, request_window_width, request_window_height, window_flags);
	if (!window) {
		_abort("SDL failed to create window: %s", SDL_GetError());
	}

	SDL_Vulkan_GetDrawableSize(window, &window_width, &window_height);

	Game_State game_state = {};
	game_state.execution_status = GAME_RUNNING;

	initialize_memory(&game_state);
	initialize_renderer(&game_state);
	initialize_assets(&game_state);
	initialize_input(&game_state);
	initialize_camera(&game_state.camera, {2, 2, 2}, {1, 1, 1}, 1);

	build_vulkan_command_buffers(&game_state.assets);

	while (game_state.execution_status != GAME_EXITING) {
		update(&game_state);

		render(&game_state);

		clear_memory_arena(&game_state.frame_arena);
	}

#if 0
	f32 delta_time = 0.0f, unscaled_delta_time = 0.0f;
	f32 previous_update_time = 0.0f;

	const f32 time_multiplier_step = 0.2f;
	s32 time_multiplier_ticks = 1.0f / time_multiplier_step;

	f32 time_multiplier_fade_time = 0.0f;
	const f32 total_time_multipler_text_fade_time = MILLISECONDS(1.5f);

	Font *ff = get_font("data/fonts/hack.ttf", 24);

	u8 running = true;
	while (running) {
		if (previous_update_time == 0.0f) {
			unscaled_delta_time = 0.0f;
			previous_update_time = SDL_GetTicks();
		} else {
			u64 time = SDL_GetTicks();
			unscaled_delta_time = time - previous_update_time;
			previous_update_time = time;
		}

		// @TODO: Move to some debug system.
		if (key_pressed(&input, SDLK_F11) && time_multiplier_ticks - 1 > 0) {
			time_multiplier_ticks -= 1;
			time_multiplier_fade_time = total_time_multipler_text_fade_time;
		}

		if (key_pressed(&input, SDLK_F12)) {
			time_multiplier_ticks += 1;
			time_multiplier_fade_time = total_time_multipler_text_fade_time;
		}

		delta_time = unscaled_delta_time * (time_multiplier_ticks * time_multiplier_step);

		u8 should_quit = handle_sdl_events(&input);
		if (should_quit) {
			break;
		}

		update_camera(&camera, &input);

		render(delta_time, camera);

		if (time_multiplier_fade_time > 0.0f) {
			char *text = format_string("Time Multiplier: %.2fx", time_multiplier_ticks * time_multiplier_step);
			V4 fg_color = { white.x, white.y, white.z, time_multiplier_fade_time / total_time_multipler_text_fade_time };
			V4 bg_color = { black.x, black.y, black.z, fg_color.w };
			draw_shadowed_text(text, ff, window_width / 2, window_height * 0.75f, CENTER_ORIGIN, fg_color, bg_color);
			time_multiplier_fade_time -= unscaled_delta_time;
		}

		SDL_GL_SwapWindow(window);
	}
#endif

	return 0;
}
