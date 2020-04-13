#include "Engine.h"

u32 windowWidth, windowHeight;

#include "Job.cpp"
#include "Math.cpp"
#include "Timer.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // @TODO: Move this out of here.
#include "Vulkan.cpp"
#include "GPU.cpp"
#include "Render.cpp"
#include "Asset.cpp"
#include "Camera.cpp"
#include "Entity.cpp"
#include "Transform.cpp"

void InitializeGame();
void GameLoop(f32 deltaTime);

bool Update()
{
	if (QuitWindowEvent())
	{
		return false;
	}
	GameLoop(0.0f);
	return true;
}

void RunGame(void *)
{
	windowWidth = 1200;
	windowHeight = 1000;
	auto window = CreateWindow(windowWidth, windowHeight);

	InitializeRenderer(&window);
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

	InitializeGame();

	bool running = true;
	while (running)
	{
		GetPlatformInput(&window);
		running = Update();
		Render();
	}

	ExitProcess(ProcessExitCode::SUCCESS);
}

s32 ApplicationEntry(s32 argc, char *argv[])
{
	InitializeMedia(true, JOB_FIBER_COUNT);
	InitializeJobs(RunGame, NULL);

	InvalidCodePath();
	return 0;
}
