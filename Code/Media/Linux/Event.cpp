#include "Event.h"
#include "XCBConnection.h"
#include "Basic/Log.h"

PlatformEvents ProcessPlatformEvents(Window *w, InputButtons *kb, Mouse *m)
{
	auto userEvents = PlatformEvents{};
	auto ev = (xcb_generic_event_t *){};
	while ((ev = xcb_poll_for_event(XCBConnection())))
	{
		if (ev->response_type == 0)
		{
			auto err = (xcb_generic_error_t *)ev;
			LogError("Window", "XCB error event: error_code: %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
			free(ev);
			continue;
		}
		if ((ev->response_type & ~80) == XCB_GE_GENERIC) // XInput event.
		{
		    switch (((xcb_ge_event_t *)ev)->event_type)
		    {
         	case XCB_INPUT_RAW_MOTION:
      		{
      		} break;
         	case XCB_INPUT_RAW_KEY_PRESS:
      		{
      			auto k = (xcb_input_raw_key_press_event_t *)ev;
				kb->Press(k->detail);
      		} break;
         	case XCB_INPUT_RAW_KEY_RELEASE:
      		{
      			auto k = (xcb_input_raw_key_release_event_t *)ev;
				kb->Release(k->detail);
      		} break;
     		}
		}
		else
		{
			auto evCode = ev->response_type & 0x7f;
			switch (evCode)
			{
			case XCB_CONFIGURE_NOTIFY:
			{
				// @TODO
			} break;
			case XCB_MAPPING_NOTIFY:
			{
				RefreshKeyboardMapping((xcb_mapping_notify_event_t *)ev);
			} break;
			case XCB_CLIENT_MESSAGE:
			{
				if (((xcb_client_message_event_t *)ev)->data.data32[0] == w->xcbDeleteWindowAtom->atom)
				{
					userEvents.quit = true;
					break;
				}
			} break;
			}
		}
		free(ev);
	}
	return userEvents;
}
