#include "jaguar.h"

u32 window_width, window_height;

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#include "memory.c"
#include "jobs.c"
#include "math.c"
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

void Update(Game_State *game_state) {
	update_input(&game_state->input, &game_state->execution_status);
	update_camera(&game_state->camera, &game_state->input);
}

void Run_Game(void *parameter) {
	Game_State *game_state = (Game_State *)parameter;
	// Initialization.
	{
		Job_Declaration jobs[] = {
			Create_Job(Initialize_Assets, game_state),
			//Create_Job(Initialize_Input, game_state),
			Create_Job(Initialize_Camera, game_state),//&game_state->camera, (V3){2, 2, 2}, (V3){1, 1, 1}, 0.4f, DEGREES_TO_RADIANS(90.0f)),
			Create_Job(Initialize_Renderer, game_state),//&game_state->camera, &game_state->permanent_arena, &game_state->frame_arena),
		};
		Job_Counter counter;
		Run_Jobs(ARRAY_COUNT(jobs), jobs, NORMAL_JOB_PRIORITY, &counter);
		Wait_For_Job_Counter(&counter);
	}
	Initialize_Input(game_state); // @TODO
	Initialize_Entities(game_state); // @TODO
	Clear_Memory_Arena(&game_state->frame_arena);
	while (game_state->execution_status != GAME_EXITING) {
		Update(game_state);
		Render(game_state);
		Clear_Memory_Arena(&game_state->frame_arena);
	}
	//Cleanup_Renderer();
	Platform_Exit_Process(PROCESS_EXIT_SUCCESS);
}

void application_entry() {
	Game_State game_state = {};
	Initialize_Memory(&game_state);
	Initialize_Jobs(&game_state, Run_Game, &game_state);
}
