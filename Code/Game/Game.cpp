#include "Engine/Engine.h"

void InitializeGame()
{
	CreateCamera("main", {2, 2, 2}, {0, 0, 0}, 0.4f, DegreesToRadians(90.0f));

	auto e = CreateEntity();
	auto t = Transform{};
	SetEntityTransform(e, t);
	SetEntityModel(e, ANVIL_ASSET, t);
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
		auto newYaw = camera->transform.rotation.z + -GetMouseDeltaX() * GetMouseSensitivity();
		auto newPitch = camera->transform.rotation.x + GetMouseDeltaY() * GetMouseSensitivity();
		if (newPitch > 89.0f)
		{
			newPitch = 89.0f;
		}
		if (newPitch < -89.0f)
		{
			newPitch = -89.0f;
		}
		SetTransformRotation(&camera->transform, newPitch, 0.0f, newYaw);
	}

	V3 newPosition = camera->transform.position;
	if (IsKeyDown(A_KEY))
	{
		//newPosition -= camera->speed * CalculateTransformRight(&camera->transform);
	}
	else if (IsKeyDown(D_KEY))
	{
		//newPosition += camera->speed * CalculateTransformRight(&camera->transform);
	}
	if (IsKeyDown(Q_KEY))
	{
		newPosition.z -= camera->speed;
	}
	else if (IsKeyDown(E_KEY))
	{
		newPosition.z += camera->speed;
	}
	if (IsKeyDown(W_KEY))
	{
		newPosition += camera->speed * CalculateTransformForward(&camera->transform);
	}
	else if (IsKeyDown(S_KEY))
	{
		newPosition -= camera->speed * CalculateTransformForward(&camera->transform);
	}
	auto f = CalculateTransformForward(&camera->transform);
	ConsolePrint("speed %f, forward %f %f %f, p %f %f %f\n", camera->speed, f[0], f[1], f[2], newPosition[0], newPosition[1], newPosition[2]);
	camera->transform.position = newPosition;
}
