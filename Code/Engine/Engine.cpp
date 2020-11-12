#include "Engine.h"
#include "Render.h"
#include "Entity.h"
#include "Job.h"
#include "Camera.h"
#include "Media/Input.h"
#include "Basic/Process.h"
#include "Basic/Log.h"
#include "Basic/Time/Timer.h"

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
	auto t = Time::NewTimer("Frame");
	while (true)
	{
		t.Print(Time::Millisecond);
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
	if (argc > 1)
	{
		if (string::Equal(argv[1], "-lv"))
		{
			SetLogLevel(VerboseLog);
		}
	}
	LogBuildOptions();
	InitializeInput();
	InitializeJobs(RunGame, NULL);
	Abort("Engine", "Invalid exit from ApplicationEntry.");
	return ProcessFail;
}
