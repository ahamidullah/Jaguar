#include "Camera.h"
#include "Math.h"

Array<Camera> cameras;

void NewCamera(String name, V3 pos, V3 lookAt, f32 speed, f32 fov)
{
	Camera c =
	{
		.name = name,
		.pitch = 0.0f,
		.yaw = 0.0f,
		.roll = 0.0f,
		.transform =
		{
			.position = pos,
			.rotation = CreateQuaternion(lookAt - pos),
		},
		.fov = fov,
		.focalLength = 0.01f,
		.speed = speed,
	};
	AppendToArray(&cameras, c);
}

Camera *Camera(String name)
{
	Camera *c = NULL;
	for (auto i = 0; i < cameras.count; i++)
	{
		if (cameras[i].name == name)
		{
			c = &cameras[i];
			break;
		}
	}
	return c;
}
