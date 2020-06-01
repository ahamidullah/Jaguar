#include "Camera.h"
#include "Math.h"

struct
{
	Array<Camera> cameras;
} camerasContext;

#if 0
void InitializeCameras(void *parameter) {//Camera *camera, V3 position, V3 forward, f32 speed, f32 field_of_view)
{
	Camera *camera = (Camera *)parameter;
	create_camera_basis(camera, (V3){1, 1, 1});
	camera->speed = 0.4f;
	camera->position = (V3){2, 2, 2};
	camera->yaw = 0.0f;
	camera->pitch = 0.0f;
	camera->view_matrix = ViewMatrix(camera->position, camera->forward, camera->side, camera->up);
	camera->field_of_view = DegreesToRadians(90.0f);
	camera->focal_length = 0.01f;
	camera->projection_matrix = InfinitePerspectiveProjection(camera->field_of_view, windowWidth / (f32)windowHeight); // @TODO
}
#endif

void CreateCamera(const String &name, V3 position, V3 lookAt, f32 speed, f32 fov)
{
	Camera camera =
	{
		.name = name,
		.pitch = 0.0f,
		.yaw = 0.0f,
		.roll = 0.0f,
		.transform =
		{
			.position = position,
			.rotation = CreateQuaternion(lookAt - position),
		},
		.fov = fov,
		.focalLength = 0.01f,
		.speed = speed,
	};
	ArrayAppend(&camerasContext.cameras, camera);
}

Camera *GetCamera(const String &name)
{
	Camera *result = NULL;
	for (auto &camera : camerasContext.cameras)
	{
		if (camera.name == name)
		{
			result = &camera;
			break;
		}
	}
	return result;
}
