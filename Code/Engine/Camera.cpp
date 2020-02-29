// @TODO: Rename to cameras?
#include "Camera.h"

struct
{
	Array<Camera> cameras;
} cameraGlobals;

#if 0
void create_camera_basis(Camera *camera, V3 forward) {
	// @TODO: Move this out to a create basis function.
	const V3 world_up = {0.0f, 0.0f, 1.0f};
	camera->forward = Normalize(forward);
	camera->side = Normalize(CrossProduct(camera->forward, world_up));
	camera->up = Normalize(CrossProduct(camera->side, camera->forward));
}
#endif

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
		.transform =
		{
			.position = position,
			//.rotation = ,
		},
		.fov = fov,
		.focalLength = 0.01f,
		.speed = speed,
	};
	//create_camera_basis(&camera, lookAt - position);
	Append(&cameraGlobals.cameras, camera);
}

V3 CalculateCameraForward(f32 pitch, f32 yaw)
{
	f32 pitch_radians = DegreesToRadians(pitch);
	f32 yaw_radians = DegreesToRadians(yaw);
	return
	{
		Cos(pitch_radians) * Cos(yaw_radians),
		Cos(pitch_radians) * Sin(yaw_radians),
		Sin(pitch_radians),
	};
}

Camera *GetCamera(const String &name)
{
	Camera *result = NULL;
	for (auto &camera : cameraGlobals.cameras)
	{
		if (camera.name == name)
		{
			result = &camera;
			break;
		}
	}
	return result;
}

#if 0
void UpdatePlayerControlledCamera(const String &name)
{
	auto camera = GetCamera(name);
	if (!camera)
	{
		LogPrint(LogType::ERROR, "got invalid camera name: %s\n", &name[0]);
		return;
	}

	if (GetMouseDeltaX() != 0 || GetMouseDeltaY() != 0)
	{
		camera->yaw += -GetMouseDeltaX() * input->mouse.sensitivity;
		camera->pitch += GetMouseDeltaY() * input->mouse.sensitivity;
		if (camera->pitch > 89.0f)
		{
			camera->pitch = 89.0f;
		}
		if (camera->pitch < -89.0f)
		{
			camera->pitch = -89.0f;
		}
		create_camera_basis(camera, CalculateCameraForward(camera->pitch, camera->yaw));
	}

	if (IsKeyDown(A_KEY))
	{
		camera->position = camera->position - (camera->speed * camera->side);
	}
	else if (IsKeyDown(D_KEY))
	{
		camera->position = camera->position + (camera->speed * camera->side);
	}

	if (IsKeyDown(Q_KEY))
	{
		camera->position.Z -= camera->speed;
	}
	else if (IsKeyDown(E_KEY))
	{
		camera->position.Z += camera->speed;
	}

	if (IsKeyDown(W_KEY))
	{
		camera->position = camera->position + (camera->speed * camera->forward);
	}
	else if (IsKeyDown(S_KEY))
	{
		camera->position = camera->position - (camera->speed * camera->forward);
	}

	camera->viewMatrix = ViewMatrix(camera->position, camera->forward, camera->side, camera->up);
}
#endif
