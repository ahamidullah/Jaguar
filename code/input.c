#define MAX_SENSITIVITY 1.0f

IO_Buttons initialize_buttons(Memory_Arena *arena, u32 count) {
	IO_Buttons result;
	result.down = allocate_array(arena, u8, count);
	result.pressed = allocate_array(arena, u8, count);
	result.released = allocate_array(arena, u8, count);
	return result;
}

void Initialize_Input(void *job_parameter) {
	Game_State *game_state = (Game_State *)job_parameter;
	game_state->input.keyboard = initialize_buttons(&game_state->permanent_arena, SCANCODE_COUNT);
	game_state->input.mouse.buttons = initialize_buttons(&game_state->permanent_arena, MOUSE_BUTTON_COUNT);
	game_state->input.mouse.sensitivity = 0.2 * MAX_SENSITIVITY;
	Platform_Get_Mouse_Position(&game_state->input.mouse.x, &game_state->input.mouse.y);
}

void press_button(u32 index, IO_Buttons *buttons) {
	buttons->pressed[index] = !buttons->down[index];
	buttons->down[index] = 1;
}

void release_button(u32 index, IO_Buttons *buttons) {
	buttons->released[index] = buttons->down[index];
	buttons->down[index] = 0;
}

u8 button_down(u32 index, IO_Buttons *buttons) {
	return buttons->down[index];
}

u8 button_up(u32 index, IO_Buttons *buttons) {
	return !buttons->down[index];
}

u8 button_pressed(u32 index, IO_Buttons *buttons) {
	return buttons->pressed[index];
}

u8 button_released(u32 index, IO_Buttons *buttons) {
	return buttons->released[index];
}

u8 key_down(Platform_Key_Symbol key_symbol, Game_Input *input) {
	return button_down(Platform_Key_Symbol_To_Scancode(key_symbol), &input->keyboard);
}

u8 key_up(Platform_Key_Symbol key_symbol, Game_Input *input) {
	return button_up(Platform_Key_Symbol_To_Scancode(key_symbol), &input->keyboard);
}

u8 key_pressed(Platform_Key_Symbol key_symbol, Game_Input *input) {
	return button_pressed(Platform_Key_Symbol_To_Scancode(key_symbol), &input->keyboard);
}

u8 key_released(Platform_Key_Symbol key_symbol, Game_Input *input) {
	return button_released(Platform_Key_Symbol_To_Scancode(key_symbol), &input->keyboard);
}

void update_input(Game_Input *input, Game_Execution_Status *execution_status) {
	{ // Clear per-frame input.
		memset(input->mouse.buttons.pressed, 0, sizeof(u8) * MOUSE_BUTTON_COUNT);
		memset(input->mouse.buttons.released, 0, sizeof(u8) * MOUSE_BUTTON_COUNT);
		memset(input->keyboard.pressed, 0, sizeof(u8) * SCANCODE_COUNT);
		memset(input->keyboard.released, 0, sizeof(u8) * SCANCODE_COUNT);
		input->mouse.raw_delta_x = 0;
		input->mouse.raw_delta_y = 0;
	}
	Platform_Handle_Window_Events(input, execution_status);
	{ // Update mouse position.
		s32 old_x = input->mouse.x;
		s32 old_y = input->mouse.y;
		Platform_Get_Mouse_Position(&input->mouse.x, &input->mouse.y);
		input->mouse.delta_x = input->mouse.x - old_x;
		input->mouse.delta_y = input->mouse.y - old_y;
	}
	if (key_pressed(ESCAPE_KEY, input)) {
		static u8 b = 1;
		if (b) {
			Platform_Capture_Cursor();
			b = 0;
		} else {
			Platform_Uncapture_Cursor();
			b = 1;
		}
	}
}
