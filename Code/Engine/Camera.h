#pragma once

struct Camera
{
	String name;

	M4 viewMatrix;
	M4 projectionMatrix;

	V3 position;
	V3 forward;
	V3 side;
	V3 up;

	f32 fov;
	f32 focalLength;
	f32 yaw;
	f32 pitch;
	f32 speed;
};

void CreateCamera(const String &name, V3 position, V3 lookAt, f32 speed, f32 fov);
void UpdatePlayerControlledCamera(const String &name, Input *input);
Camera *GetCamera(const String &name);
