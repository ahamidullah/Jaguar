#define MAX_SENSITIVITY 1.0f

IO_Buttons initialize_buttons(Memory_Arena *arena, u32 count) {
	IO_Buttons result;
	result.down = allocate_array(arena, u8, count);
	result.pressed = allocate_array(arena, u8, count);
	result.released = allocate_array(arena, u8, count);
	return result;
}

void InitializeInput(void *job_parameter) {
	GameState *game_state = (GameState *)job_parameter;
	game_state->input.keyboard = initialize_buttons(&game_state->permanent_arena, SCANCODE_COUNT);
	game_state->input.mouse.buttons = initialize_buttons(&game_state->permanent_arena, MOUSE_BUTTON_COUNT);
	game_state->input.mouse.sensitivity = 0.2 * MAX_SENSITIVITY;
	PlatformGetMousePosition(&game_state->input.mouse.x, &game_state->input.mouse.y);
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

u8 key_down(PlatformKeySymbol key_symbol, Game_Input *input)
{
	return button_down(PlatformKeySymbolToScancode(key_symbol), &input->keyboard);
}

u8 key_up(PlatformKeySymbol key_symbol, Game_Input *input)
{
	return button_up(PlatformKeySymbolToScancode(key_symbol), &input->keyboard);
}

u8 key_pressed(PlatformKeySymbol key_symbol, Game_Input *input)
{
	return button_pressed(PlatformKeySymbolToScancode(key_symbol), &input->keyboard);
}

u8 key_released(PlatformKeySymbol key_symbol, Game_Input *input)
{
	return button_released(PlatformKeySymbolToScancode(key_symbol), &input->keyboard);
}

void UpdateInput(Game_Input *input, GameExecutionStatus *execution_status) {
	{ // Clear per-frame input.
		memset(input->mouse.buttons.pressed, 0, sizeof(u8) * MOUSE_BUTTON_COUNT);
		memset(input->mouse.buttons.released, 0, sizeof(u8) * MOUSE_BUTTON_COUNT);
		memset(input->keyboard.pressed, 0, sizeof(u8) * SCANCODE_COUNT);
		memset(input->keyboard.released, 0, sizeof(u8) * SCANCODE_COUNT);
		input->mouse.raw_delta_x = 0;
		input->mouse.raw_delta_y = 0;
	}
	PlatformHandleWindowEvents(input, execution_status);
	{ // Update mouse position.
		s32 old_x = input->mouse.x;
		s32 old_y = input->mouse.y;
		PlatformGetMousePosition(&input->mouse.x, &input->mouse.y);
		input->mouse.delta_x = input->mouse.x - old_x;
		input->mouse.delta_y = input->mouse.y - old_y;
	}
	if (key_pressed(ESCAPE_KEY, input)) {
		static u8 b = 1;
		if (b) {
			PlatformCaptureCursor();
			b = 0;
		} else {
			PlatformUncaptureCursor();
			b = 1;
		}
	}
}
