#include "Engine.h"
#include "Render.h"
#include "Entity.h"
#include "Job.h"
#include "Camera.h"

#include "Code/Media/Media.h"
#include "Code/Media/Input.h"

#include "Code/Basic/Process.h"
#include "Code/Basic/Log.h"

u32 windowWidth, windowHeight;

u32 GetWindowWidth()
{
	return windowWidth;
}

u32 GetWindowHeight()
{
	return windowHeight;
}

void InitializeGameLoop()
{
	CreateCamera("main", {10000, -10000, 10000}, {0, 0, 0}, 100.4f, DegreesToRadians(90.0f));

	auto e = CreateEntity();
	auto t = Transform{};
	SetEntityTransform(e, t);
	SetEntityModel(e, SPONZA_ASSET, t);
}

void GameLoop(f32 deltaTime)
{
	auto camera = GetCamera("main");
	if (!camera)
	{
		return;
	}

	if (GetMouseDeltaX() != 0 || GetMouseDeltaY() != 0)
	{
		auto deltaPitch = -GetMouseDeltaY() * GetMouseSensitivity();
		auto deltaYaw = -GetMouseDeltaX() * GetMouseSensitivity();
		auto angles = ToAngles(camera->transform.rotation);
		auto pitchRotation = ToQuaternion(EulerAngles{.pitch = angles.pitch + deltaPitch});
		auto yawRotation = ToQuaternion(EulerAngles{.yaw = angles.yaw + deltaYaw});
		camera->transform.rotation = Normalize(yawRotation * pitchRotation);
	}

	if (IsKeyDown(A_KEY))
	{
		camera->transform.position -= camera->speed * CalculateRightVector(camera->transform.rotation);
	}
	else if (IsKeyDown(D_KEY))
	{
		camera->transform.position += camera->speed * CalculateRightVector(camera->transform.rotation);
	}
	if (IsKeyDown(Q_KEY))
	{
		camera->transform.position.z -= camera->speed;
	}
	else if (IsKeyDown(E_KEY))
	{
		camera->transform.position.z += camera->speed;
	}
	if (IsKeyDown(W_KEY))
	{
		camera->transform.position += camera->speed * CalculateForwardVector(camera->transform.rotation);
	}
	else if (IsKeyDown(S_KEY))
	{
		camera->transform.position -= camera->speed * CalculateForwardVector(camera->transform.rotation);
	}
}

void Update()
{
	GameLoop(0.0f);
}

void RunGame(void *)
{
	windowWidth = GetRenderWidth();
	windowHeight = GetRenderHeight();
	auto window = CreateWindow(windowWidth, windowHeight, false);

	InitializeRenderer(&window);
	{
		JobDeclaration jobs[] = {
			CreateJob(InitializeAssets, NULL),
			//CreateJob(InitializeInput, NULL),
			//CreateJob(InitializeRenderer, NULL),
		};
		JobCounter counter;
		RunJobs(CArrayCount(jobs), jobs, NORMAL_PRIORITY_JOB, &counter);
		WaitForJobCounter(&counter);
	}
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

	ExitProcess(PROCESS_EXIT_SUCCESS);
}

s32 ApplicationEntry(s32 argc, char *argv[])
{
	InitializeMedia(true);
	InitializeJobs(RunGame, NULL);

	Abort("Invalid exit from ApplicationEntry.\n");
	return PROCESS_EXIT_FAILURE;
}
