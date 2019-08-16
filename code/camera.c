void create_camera_basis(Camera *camera, V3 forward) {
	// @TODO: Move this out to a create basis function.
	const V3 world_up = {0.0f, 0.0f, 1.0f};
	camera->forward = normalize(forward);
	camera->side = normalize(cross_product(camera->forward, world_up));
	camera->up = normalize(cross_product(camera->side, camera->forward));
}

void initialize_camera(Camera *camera, V3 position, V3 forward, f32 speed) {
	camera->speed = speed;
	camera->position = position;
	create_camera_basis(camera, forward);
	camera->yaw = 0.0f;
	camera->pitch = 0.0f;
	camera->view_matrix = view_matrix(camera->position, camera->forward, camera->side, camera->up);
}

V3 calculate_camera_forward(f32 pitch, f32 yaw) {
	f32 pitch_radians = DEGREES_TO_RADIANS(pitch);
	f32 yaw_radians = DEGREES_TO_RADIANS(yaw);
	return (V3){
		cos(pitch_radians) * cos(yaw_radians),
		cos(pitch_radians) * sin(yaw_radians),
		sin(pitch_radians),
	};
}

void update_camera(Camera *camera, Game_Input *input) {
	if (input->mouse.raw_delta_x != 0 || input->mouse.raw_delta_y != 0) {
		camera->yaw += -input->mouse.raw_delta_x * input->mouse.sensitivity;
		camera->pitch += input->mouse.raw_delta_y * input->mouse.sensitivity;
		if (camera->pitch > 89.0f) {
			camera->pitch = 89.0f;
		}
		if (camera->pitch < -89.0f) {
			camera->pitch = -89.0f;
		}
		create_camera_basis(camera, calculate_camera_forward(camera->pitch, camera->yaw));
	}

	if (key_down(A_KEY, input)) {
		camera->position = subtract_v3(camera->position, multiply_f32_v3(camera->speed, camera->side));
	} else if (key_down(D_KEY, input)) {
		camera->position = add_v3(camera->position, multiply_f32_v3(camera->speed, camera->side));
	}

	if (key_down(Q_KEY, input)) {
		camera->position.z -= camera->speed;
	} else if (key_down(E_KEY, input)) {
		camera->position.z += camera->speed;
	}

	if (key_down(W_KEY, input)) {
		camera->position = add_v3(camera->position, multiply_f32_v3(camera->speed, camera->forward));
	} else if (key_down(S_KEY, input)) {
		camera->position = subtract_v3(camera->position, multiply_f32_v3(camera->speed, camera->forward));
	}

	camera->view_matrix = view_matrix(camera->position, camera->forward, camera->side, camera->up);
}
