#pragma once

struct Camera
{
	String name;
	//M4 viewMatrix;
	//M4 projectionMatrix;
	Transform transform;
	f32 fov;
	f32 focalLength;
	f32 speed;
};

void CreateCamera(const String &name, V3 position, V3 lookAt, f32 speed, f32 fov);
Camera *GetCamera(const String &name);
