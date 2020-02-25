s32 KeySymbolToScancode(KeySymbol keySymbol)
{
	s32 scancode = XKeysymToKeycode(x11Display, keySymbol);
	Assert(scancode > 0);
	return scancode;
}

void GetMousePosition(WindowContext *window, s32 *x, s32 *y)
{
	s32 screenX, screenY;
	Window root, child;
	u32 mouseButtons;
	XQueryPointer(x11Display, window->x11, &root, &child, &screenX, &screenY, x, y, &mouseButtons);
	*y = (window->height - *y); // Bottom left is zero for us, top left is zero for x11.
}

#if 0
void PlatformHandleWindowEvents(PlatformWindow window, PlatformWindowEvents *windowEvents) {
	XEvent event;
	XGenericEventCookie *cookie = &event.xcookie;
	XIRawEvent *rawEvent;
	XFlush(linuxContext.display);
	while (XPending(linuxWindowContext.display)) {
		XNextEvent(linuxWindowContext.display, &event);
		if ((event.type == ClientMessage) && ((Atom)event.xclient.data.l[0] == window.deleteWindowAtom)) {
			input->gotQuitEvent = true;
			break;
		}
		if (!input) {
			return;
		}
		if (event.type == ConfigureNotify) {
			XConfigureEvent configureEvent = event.xconfigure;
			// @TODO Window resize.
			continue;
		}
		if (!XGetEventData(window.display, cookie)
		 || cookie->type != GenericEvent
		 || cookie->extension != PlatformGetXInputOpcode()) {
			continue;
		}
		rawEvent = (XIRawEvent *)cookie->data;
		switch(rawEvent->evtype) {
		case XI_RawMotion: {
			// @TODO: Check XIMaskIsSet(re->valuators.mask, 0) for x and XIMaskIsSet(re->valuators.mask, 1) for y.
			input->mouse.raw_delta_x += rawEvent->raw_values[0];
			input->mouse.raw_delta_y -= rawEvent->raw_values[1];
		} break;
		case XI_RawKeyPress: {
			PlatformPressButton(rawEvent->detail, &input->keyboard);
		} break;
		case XI_RawKeyRelease: {
			PlatformReleaseButton(rawEvent->detail, &input->keyboard);
		} break;
		case XI_RawButtonPress: {
			u32 buttonIndex = (event.xbutton.button - 1);
			if (buttonIndex > MOUSE_BUTTON_COUNT) {
				break;
			}
			PlatformPressButton(buttonIndex, &input->mouse.buttons);
		} break;
		case XI_RawButtonRelease: {
			u32 buttonIndex = (event.xbutton.button - 1);
			if (buttonIndex > MOUSE_BUTTON_COUNT) {
				break;
			}
			PlatformReleaseButton(buttonIndex, &input->mouse.buttons);
		} break;
		case XI_FocusIn: {
		} break;
		case XI_FocusOut: {
		} break;
		}
	}
}
#endif
