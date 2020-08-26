#include "Engine.h"
#include "Render.h"
#include "Entity.h"
#include "Job.h"
#include "Camera.h"

#include "Code/Media/Media.h"
#include "Code/Media/Input.h"

#include "Code/Basic/Process.h"
#include "Code/Basic/Log.h"

s64 windowWidth, windowHeight;

s64 WindowWidth()
{
	return windowWidth;
}

s64 WindowHeight()
{
	return windowHeight;
}

void InitializeGameLoop()
{
	LoadAsset(AssetIDSponza);

	NewCamera("main", {10000, -10000, 10000}, {0, 0, 0}, 100.4f, DegreesToRadians(90.0f));

	auto e = NewEntity();
	auto t = Transform{};
	SetEntityTransform(e, t);
	SetEntityModel(e, AssetIDSponza);
}

void GameLoop(f32 deltaTime)
{
	auto cam = Camera("main");
	if (!cam)
	{
		LogPrint(LogLevelError, "Engine", "Couldn't get main camera.\n");
		return;
	}
	if (MouseDeltaX() != 0 || MouseDeltaY() != 0)
	{
		auto dPitch = -MouseDeltaY() * MouseSensitivity();
		auto dYaw = -MouseDeltaX() * MouseSensitivity();
		TransformRotateEuler(&cam->transform.rotation dPitch, dYaw, 0.0f);
	}
	if (IsKeyDown(AKey))
	{
		cam->transform.position -= cam->speed * QuaternionRightVector(cam->transform.rotation);
	}
	else if (IsKeyDown(DKey))
	{
		cam->transform.position += cam->speed * QuaternionRightVector(cam->transform.rotation);
	}
	if (IsKeyDown(QKey))
	{
		cam->transform.position.z -= cam->speed;
	}
	else if (IsKeyDown(EKey))
	{
		cam->transform.position.z += cam->speed;
	}
	if (IsKeyDown(WKey))
	{
		cam->transform.position += cam->speed * QuaternionForwardVector(cam->transform.rotation);
	}
	else if (IsKeyDown(SKey))
	{
		cam->transform.position -= cam->speed * QuaternionForwardVector(cam->transform.rotation);
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
	auto window = NewWindow(windowWidth, windowHeight, false);
	LogPrint(LogLevelInfo, "Engine", "Window dimensions: %dx%d\n", windowWidth, windowHeight);

	InitializeRenderer(&window);
	InitializeAssets(NULL);
	InitializeEntities(); // @TODO
	InitializeGameLoop();

	while (true)
	{
		auto windowEvents = GetInput(&window);
		if (windowEvents.quit)
		{
			break;
		}

		Update();

		Render();
	}

	ExitProcess(ProcessExitSuccess);
}

void LogBuildOptions()
{
	LogPrint(
		LogLevelInfo,
		"Engine",
		"Build options:\n"
		"	Speed: "
#if defined(DEBUG_BUILD)
		"Debug\n"
#elif defined(OPTIMIZED_BUILD)
		"Optimized\n"
#endif
		"	Runtime: "
#if defined(DEVELOPMENT_BUILD)
		"Development\n"
#elif defined(RELEASE_BUILD)
		"Release\n"
#endif
		"	Rendering API: "
#if defined(USING_VULKAN_API)
		"Vulkan\n"
#endif
#if defined(ADDRESS_SANITIZER_BUILD)
		"Extra: AddressSanitizer\n"
#elif defined(THREAD_SANITIZER_BUILD)
		"Extra: ThreadSanitizer\n"
#endif
	);
}

s32 ApplicationEntry(s32 argc, char *argv[])
{
	LogBuildOptions();

	InitializeMedia(true);
	InitializeJobs(RunGame, NULL);

	Abort("Engine", "Invalid exit from ApplicationEntry.\n");
	return ProcessExitFailure;
}
