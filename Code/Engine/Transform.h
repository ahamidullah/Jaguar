#pragma once

#include "Math.h"

struct Transform
{
	V3 position;
	Quaternion rotation;
};

void TransformRotateEuler(Transform *t, f32 pitch, f32 yaw, f32 roll)
