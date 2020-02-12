#include "Platform/Platform.cpp"
#include "Platform/Windows.cpp"
#include "Platform/Threads.cpp"
#include "Platform/Fibers.cpp"
#include "Platform/Files.cpp"
#include "Platform/Time.cpp"
#include "Platform/Memory.cpp"
#include "Platform/DynamicLibraries.cpp"

#include "Common/Common.cpp"
#include "Common/Array.h"
#include "Common/Strings.cpp"
#include "Common/Memory.cpp"
#include "Common/Filesystem.cpp"
#include "Common/Log.cpp"

//#include "Game/Jaguar.h" // @TODO

u32 window_width, window_height;

#include "Jobs.cpp"
#include "Math.cpp"
#include "Timer.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Vulkan.cpp"
#include "GPU.cpp"
#include "Render.cpp"
#include "Assets.cpp"
#include "Camera.cpp"
#include "Entities.cpp"

bool Loop(PlatformInput *input, PlatformWindowEvents *windowEvents) {
	if (windowEvents->quit) {
		return false;
	}
	return true;
}

bool Update(PlatformInput *input, PlatformWindowEvents *windowEvents) {
	bool running = Loop(input, windowEvents);
	return running;
}

void RunGame(void *Parameter) {
	InitializeRenderer(NULL);
	{
		JobDeclaration jobs[] = {
			CreateJob(InitializeAssets, NULL),
			//CreateJob(InitializeInput, NULL),
			//CreateJob(InitializeRenderer, NULL),
		};
		JobCounter counter;
		RunJobs(ArrayCount(jobs), jobs, NORMAL_PRIORITY_JOB, &counter);
		WaitForJobCounter(&counter);
	}
	InitializeEntities(); // @TODO

	PlatformWindow window;
	PlatformInput input;
	PlatformWindowEvents windowEvents;
	bool running = true;
	while (running) {
		PlatformGetInput(window, &input, &windowEvents);
		running = Update(&input, &windowEvents);
		Render();
	}

	//Cleanup_Renderer();
	PlatformExitProcess(PLATFORM_SUCCESS_EXIT_CODE);
}

void ApplicationEntry() {
	InitializeJobs(RunGame, NULL);
}
