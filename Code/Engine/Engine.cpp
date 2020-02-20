#include "Engine.h"

THREAD_LOCAL u32 threadIndex; // @TODO
u32 window_width, window_height;

#include "Jobs.cpp"
#include "Math.cpp"
#include "Timer.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // @TODO: Move this out of here.
#include "Vulkan.cpp"
#include "GPU.cpp"
#include "Render.cpp"
#include "Assets.cpp"
#include "Camera.cpp"
#include "Entities.cpp"

void GameLoop();

bool Loop(Input *input)
{
	if (input->windowEvents.quit)
	{
		return false;
	}
	GameLoop();
	return true;
}

bool Update(Input *input)
{
	bool running = Loop(input);
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

	WindowContext window;
	Input input;
	bool running = true;
	while (running)
	{
		GetPlatformInput(&window, &input);
		running = Update(&input);
		Render();
	}

	//Cleanup_Renderer();
	ExitProcess(ProcessExitCode::SUCCESS);
}

void ApplicationEntry()
{
	InitializeMedia(JOB_FIBER_COUNT);
	InitializeJobs(RunGame, NULL);
}
