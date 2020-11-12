#pragma once

#include "Transform.h"
#include "Common.h"

struct Camera
{
	string::String name;
	f32 pitch, yaw, roll;
	Transform transform;
	f32 fov;
	f32 focalLength;
	f32 speed;
};

Camera *NewCamera(string::String name, V3 pos, V3 lookAt, f32 speed, f32 fov);
array::View<Camera> Cameras();
Camera *LookupCamera(string::String name);
