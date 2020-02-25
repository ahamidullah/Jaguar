#include "Engine.h"

THREAD_LOCAL u32 threadIndex; // @TODO
u32 windowWidth, windowHeight;

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
#include "Entity.cpp"
#include "Renderer/Shader.cpp"

void GameLoop(f32 deltaTime);

bool Loop(Input *input)
{
	if (input->windowEvents.quit)
	{
		return false;
	}
	GameLoop(0.0f);
	return true;
}

bool Update(Input *input)
{
	bool running = Loop(input);
	return running;
}

void RunGame(void *Parameter)
{
	windowWidth = 800;
	windowHeight = 600;
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

s32 ApplicationEntry(s32 argc, char *argv[])
{
	InitializeMedia(true, JOB_FIBER_COUNT);
	InitializeJobs(RunGame, NULL);

	InvalidCodePath();
	return 0;
}
