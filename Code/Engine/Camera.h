#pragma once

#include "Transform.h"
#include "Common.h"

struct Camera
{
	f32 pitch, yaw, roll;
	Transform transform;
	f32 fov;
	f32 focalLength;
	f32 speed;
};

Camera *NewCamera(V3 pos, V3 lookAt, f32 speed, f32 fov);
