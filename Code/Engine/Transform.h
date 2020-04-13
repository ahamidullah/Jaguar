#pragma once

struct Transform
{
	V3 position;
	Quaternion rotation;
};

void SetTransformRotation(Transform *transform, f32 pitch, f32 roll, f32 yaw);
void SetTransformPosition(Transform *transform, f32 x, f32 y, f32 z);
