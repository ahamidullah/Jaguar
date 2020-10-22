#include "Engine.h"
#include "Render.h"
#include "Entity.h"
#include "Job.h"
#include "Camera.h"
#include "Media/Input.h"
#include "Basic/Process.h"
#include "Basic/Log.h"
#include "Basic/Time.h"

s64 windowWidth, windowHeight;

s64 WindowWidth()
{
	return windowWidth;
}

s64 WindowHeight()
{
	return windowHeight;
}

auto cam = (Camera *){};

void InitializeGameLoop()
{
	cam = NewCamera("Main", {2, 2, 2}, {0, 0, 0}, 0.2f, DegreesToRadians(90.0f));
	LoadAsset("Box");
	//auto e = NewEntity();
	//auto t = Transform{};
	//SetEntityTransform(e, t);
	//SetEntityModel(e, SponzaAssetID);
}

void GameLoop(f32 deltaTime)
{
	if (MouseDeltaX() != 0 || MouseDeltaY() != 0)
	{
		auto dPitch = -MouseDeltaY() * MouseSensitivity();
		auto dYaw = -MouseDeltaX() * MouseSensitivity();
		cam->transform.RotateEulerLocal({dPitch, dYaw, 0.0f});
	}
	if (KeyDown(AKey))
	{
		cam->transform.position -= cam->speed * cam->transform.Right();
	}
	else if (KeyDown(DKey))
	{
		cam->transform.position += cam->speed * cam->transform.Right();
	}
	if (KeyDown(QKey))
	{
		cam->transform.position.z -= cam->speed;
	}
	else if (KeyDown(EKey))
	{
		cam->transform.position.z += cam->speed;
	}
	if (KeyDown(WKey))
	{
		cam->transform.position += cam->speed * cam->transform.Forward();
	}
	else if (KeyDown(SKey))
	{
		cam->transform.position -= cam->speed * cam->transform.Forward();
	}
}

void Update()
{
	GameLoop(0.0f);
}

void Test2(void *x)
{
	//LogInfo("Engine", "NUMBER2");
}

void Test(void *x)
{
	LogInfo("Engine", "HELLLLOOOOOO");
	{
		auto j = Array<JobDeclaration>{};
		for (auto i = 0; i < 1000; i += 1)
		{
			j.Append(NewJobDeclaration(Test2, NULL));
		}
		auto c = (JobCounter *){};
		RunJobs(j, NormalJobPriority, &c);
		c->Wait();
	}
	{
		auto j = Array<JobDeclaration>{};
		for (auto i = 0; i < 1000; i += 1)
		{
			j.Append(NewJobDeclaration(Test2, NULL));
		}
		auto c = (JobCounter *){};
		RunJobs(j, NormalJobPriority, &c);
		c->Wait();
	}
}

#include <stdio.h>

void RunGame(void *)
{
	windowWidth = RenderWidth();
	windowHeight = RenderHeight();
	auto win = NewWindow(windowWidth, windowHeight);
	LogInfo("Engine", "Window dimensions: %dx%d", windowWidth, windowHeight);
	InitializeRenderer(&win);
	InitializeAssets(NULL);
	InitializeEntities(); // @TODO
	InitializeGameLoop();
	#if 0
	auto j = Array<JobDeclaration>{};
	for (auto i = 0; i < 150; i += 1)
	{
		j.Append(NewJobDeclaration(Test, NULL));
	}
	auto c = (JobCounter *){};
	RunJobs(j, NormalJobPriority, &c);
	c->Wait();
	ExitProcess(ProcessSuccess);
	#endif
	auto t = NewTimer("Frame");
	#if 0
	for (auto i = 0; i < 1000; i += 1)
	{
		auto j = Array<JobDeclaration>{};
		j.Append(NewJobDeclaration(Test2, NULL));
		auto c = (JobCounter *){};
		RunJobs(j, NormalJobPriority, NULL);
		//c->Wait();
	}
		for (auto i = 0; i < 100000; i += 1)
		{
			for (auto k = 0; k < 1; k += 1)
			{
				//printf("doing %d\n", i);
				auto c = (JobCounter *){};
				auto j = Array<JobDeclaration>{};
				j.Append(NewJobDeclaration(Test2, NULL));
				RunJobs(j, NormalJobPriority, &c);
				c->Wait();
				//Assert(c->waitingFibers.count == 1);
				if (c->waitingFibers.count != 1)
				{
					//printf("missing %ld\n", c->waitingFibers.count);
				}
			}
			/*
			for (auto k = 0; k < 1; k += 1)
			{
				auto c = (JobCounter *){};
				auto j = Array<JobDeclaration>{};
				j.Append(NewJobDeclaration(Test2, NULL));
				RunJobs(j, NormalJobPriority, &c);
				c->Wait();
			}
			*/
		}
	#endif
	while (true)
	{
		t.Print(TimeMillisecond);
		t.Reset();
		auto winEvents = ProcessInput(&win);
		if (winEvents.quit)
		{
			break;
		}
		Update();
		Render();
	}
	ExitProcess(ProcessSuccess);
}

void LogBuildOptions()
{
	#ifdef DebugBuild
		LogInfo("Engine", "Speed: Debug");
	#else
		LogInfo("Engine", "Speed: Optimized");
	#endif
	#ifdef DevelopmentBuild
		LogInfo("Engine", "Runtime: Development");
	#else
		LogInfo("Engine", "Runtime: Release");
	#endif
	#ifdef VulkanBuild
		LogInfo("Engine", "Rendering API: Vulkan");
	#endif
	#ifdef AddressSanitizerBuild
		LogInfo("Engine", "Static Analysis: AddressSanitizer");
	#endif
	#ifdef ThreadSanitizerBuild
		LogInfo("Engine", "Static Analysis: ThreadSanitizer");
	#endif
}

//s32 ApplicationEntry(s32 argc, char *argv[])
s32 main(s32 argc, char *argv[])
{
	LogBuildOptions();
	InitializeInput();
	InitializeJobs(RunGame, NULL);
	Abort("Engine", "Invalid exit from ApplicationEntry.");
	return ProcessFail;
}
