#include "Platform/Start.cpp"
#include "Platform/Window.cpp"
#include "Platform/Threads.cpp"
#include "Platform/Files.cpp"
#include "Platform/Time.cpp"
#include "Platform/Vulkan.cpp"
#include "Platform/Memory.cpp"

#include "Common/Common.cpp"
#include "Common/Array.h"
#include "Common/Strings.cpp"
#include "Common/Memory.cpp"
#include "Common/Filesystem.cpp"
#include "Common/Log.cpp"

#include "Game/Jaguar.h" // @TODO

u32 window_width, window_height;

#include "Game/Jobs.cpp"
#include "Game/Math.cpp"
#include "Game/Timer.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "Game/stb_image.h"
#include "Game/Vulkan.cpp"
#include "Game/Gpu.cpp"
#include "Game/Render.cpp"
#include "Game/Assets.cpp"
#include "Game/Input.cpp"
#include "Game/Camera.cpp"
#include "Game/Entities.cpp"

void Update(GameState *State) {
	UpdateInput(&State->input, &State->ExecutionStatus);
	UpdateCamera(&State->camera, &State->input);
}

void RunGame(void *Parameter) {
	GameState *State = (GameState *)Parameter;
	InitializeRenderer(State);
	{
		JobDeclaration jobs[] = {
			CreateJob(InitializeAssets, State),
			//CreateJob(InitializeInput, State),
			CreateJob(InitializeCamera, State),
			//CreateJob(InitializeRenderer, State),
		};
		JobCounter counter;
		RunJobs(ArrayCount(jobs), jobs, NORMAL_PRIORITY_JOB, &counter);
		WaitForJobCounter(&counter);
	}
	InitializeInput(State); // @TODO
	InitializeEntities(State); // @TODO
	ClearMemoryArena(&State->frameArena);

	while (State->ExecutionStatus != GAME_EXITING) {
		Update(State);
		Render(&State->render_context, &State->camera, &gameEntities.meshes.instances);
		ClearMemoryArena(&State->frameArena);
	}

	//Cleanup_Renderer();
	PlatformExitProcess(PLATFORM_SUCCESS_EXIT_CODE);
}

void ApplicationEntry() {
	GameState State = {};
	InitializeMemory(&State);
	InitializeJobs(&State, RunGame, &State);
}
