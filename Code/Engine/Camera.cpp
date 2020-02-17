// @TODO: Rename to cameras?
#include "Camera.h"

struct {
	Array<Camera> cameras;
} camerasContext;

void create_camera_basis(Camera *camera, V3 forward) {
	// @TODO: Move this out to a create basis function.
	const V3 world_up = {0.0f, 0.0f, 1.0f};
	camera->forward = Normalize(forward);
	camera->side = Normalize(CrossProduct(camera->forward, world_up));
	camera->up = Normalize(CrossProduct(camera->side, camera->forward));
}

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
	camera->projection_matrix = InfinitePerspectiveProjection(camera->field_of_view, window_width / (f32)window_height); // @TODO
}
#endif

void CreateCamera(const String &name, V3 position, V3 lookAt, f32 speed, f32 fov) {
	Camera camera;
	camera.name = name;
	create_camera_basis(&camera, lookAt - position);
	camera.speed = speed;
	camera.position = position;
	camera.yaw = 0.0f;
	camera.pitch = 0.0f;
	camera.viewMatrix = ViewMatrix(camera.position, camera.forward, camera.side, camera.up);
	camera.fov = DegreesToRadians(fov);
	camera.focalLength = 0.01f;
	camera.projectionMatrix = InfinitePerspectiveProjection(camera.fov, window_width / (f32)window_height); // @TODO
	Append(&camerasContext.cameras, camera);
}

V3 calculate_camera_forward(f32 pitch, f32 yaw) {
	f32 pitch_radians = DegreesToRadians(pitch);
	f32 yaw_radians = DegreesToRadians(yaw);
	return {
		Cos(pitch_radians) * Cos(yaw_radians),
		Cos(pitch_radians) * Sin(yaw_radians),
		Sin(pitch_radians),
	};
}

Camera *GetCamera(const String &name) {
	Camera *result = NULL;
	for (auto &camera : camerasContext.cameras) {
		if (camera.name == name) {
			result = &camera;
			break;
		}
	}
	return result;
}

void UpdatePlayerControlledCamera(const String &name, Input *input) {
	auto camera = GetCamera(name);
	if (!camera) {
		LogPrint(LogType::ERROR, "got invalid camera name: %s\n", &name[0]);
		return;
	}

	if (input->mouse.rawDeltaX != 0 || input->mouse.rawDeltaY != 0) {
		camera->yaw += -input->mouse.rawDeltaX * input->mouse.sensitivity;
		camera->pitch += input->mouse.rawDeltaY * input->mouse.sensitivity;
		if (camera->pitch > 89.0f) {
			camera->pitch = 89.0f;
		}
		if (camera->pitch < -89.0f) {
			camera->pitch = -89.0f;
		}
		create_camera_basis(camera, calculate_camera_forward(camera->pitch, camera->yaw));
	}

	if (IsKeyDown(A_KEY, input)) {
		camera->position = camera->position - (camera->speed * camera->side);
	} else if (IsKeyDown(D_KEY, input)) {
		camera->position = camera->position + (camera->speed * camera->side);
	}

	if (IsKeyDown(Q_KEY, input)) {
		camera->position.Z -= camera->speed;
	} else if (IsKeyDown(E_KEY, input)) {
		camera->position.Z += camera->speed;
	}

	if (IsKeyDown(W_KEY, input)) {
		camera->position = camera->position + (camera->speed * camera->forward);
	} else if (IsKeyDown(S_KEY, input)) {
		camera->position = camera->position - (camera->speed * camera->forward);
	}

	camera->viewMatrix = ViewMatrix(camera->position, camera->forward, camera->side, camera->up);
}
