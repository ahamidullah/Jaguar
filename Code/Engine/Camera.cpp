#include "Camera.h"
#include "Math.h"
#include "Basic/Array.h"

auto cameras = Array<Camera>{};

Camera *NewCamera(V3 pos, V3 lookAt, f32 speed, f32 fov)
{
	auto c = Camera
	{
		.pitch = 0.0f,
		.yaw = 0.0f,
		.roll = 0.0f,
		.transform =
		{
			.position = pos,
			.rotation = NewQuaternion(lookAt - pos),
		},
		.fov = fov,
		.focalLength = 0.01f,
		.speed = speed,
	};
	cameras.Append(c);
	return &cameras[cameras.count - 1];
}
