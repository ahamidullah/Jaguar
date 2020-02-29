#pragma once

struct Transform
{
	V3 position;
	V3 rotation;
};

void SetTransformRotation(Transform *transform, f32 pitch, f32 roll, f32 yaw);
void SetTransformPosition(Transform *transform, f32 x, f32 y, f32 z);
V3 CalculateTransformForward(Transform *transform);
V3 CalculateTransformUp(Transform *transform);
V3 CalculateTransformRight(Transform *transform);
