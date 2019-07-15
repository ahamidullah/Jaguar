void initialize_input(Game_State *game_state) {
	game_state->input.mouse.sensitivity = 1.0f;
	get_mouse_xy(&game_state->input.mouse.x, &game_state->input.mouse.y);
}

template <u32 T>
void press_button(u32 index, IO_Buttons<T> *buttons) {
	buttons->pressed[index] = !buttons->down[index];
	buttons->down[index] = true;
}

template <u32 T>
void release_button(u32 index, IO_Buttons<T> *buttons) {
	buttons->released[index] = buttons->down[index];
	buttons->down[index] = false;
	//db.released = db.down;
	//db.down = false;
	//return db;
}

template <u32 T>
u8 button_down(u32 index, IO_Buttons<T> *buttons) {
	return buttons->down[index];
}

template <u32 T>
u8 button_up(u32 index, IO_Buttons<T> *buttons) {
	return !buttons->down[index];
}

template <u32 T>
u8 button_pressed(u32 index, IO_Buttons<T> *buttons) {
	return buttons->pressed[index];
}

template <u32 T>
u8 button_released(u32 index, IO_Buttons<T> *buttons) {
	return buttons->released[index];
}

u8 key_down(Key_Symbol key_symbol, Game_Input *input) {
	return button_down(key_symbol_to_scancode(key_symbol), &input->keyboard);
}

u8 key_up(Key_Symbol key_symbol, Game_Input *input) {
	return button_up(key_symbol_to_scancode(key_symbol), &input->keyboard);
}

u8 key_pressed(Key_Symbol key_symbol, Game_Input *input) {
	return button_pressed(key_symbol_to_scancode(key_symbol), &input->keyboard);
}

u8 key_released(Key_Symbol key_symbol, Game_Input *input) {
	return button_released(key_symbol_to_scancode(key_symbol), &input->keyboard);
}

void update_input(Game_Input *input, Game_Execution_Status *execution_status) {
	// Clear per-frame input.
	{
		memset(&input->mouse.buttons.pressed, 0, sizeof(u8) * MOUSE_BUTTON_COUNT);
		memset(&input->mouse.buttons.released, 0, sizeof(u8) * MOUSE_BUTTON_COUNT);

		memset(&input->keyboard.pressed, 0, sizeof(u8) * MAX_SCANCODES);
		memset(&input->keyboard.released, 0, sizeof(u8) * MAX_SCANCODES);
	}

	handle_platform_events(input, execution_status);

	// Update mouse position.
	{
		s32 old_x = input->mouse.x;
		s32 old_y = input->mouse.y;

		get_mouse_xy(&input->mouse.x, &input->mouse.y);

		input->mouse.delta_x = input->mouse.x - old_x;
		input->mouse.delta_y = input->mouse.y - old_y;
	}
}
