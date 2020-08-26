#pragma once

#include "Transform.h"

#include "Basic/String.h"

#include "Common.h"

struct Camera
{
	String name;
	f32 pitch, yaw, roll;
	Transform transform;
	f32 fov;
	f32 focalLength;
	f32 speed;
};

void NewCamera(String name, V3 pos, V3 lookAt, f32 speed, f32 fov);
Camera *Camera(String name);
