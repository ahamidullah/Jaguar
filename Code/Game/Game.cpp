#include "Engine/Engine.h"

void InitializeGame()
{
	CreateCamera("main", {50000, 50000, 50000}, {0, 0, 0}, 1000.4f, DegreesToRadians(90.0f));

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
