#include "../Input.h"
#include "XCBConnection.h"
#include "Basic/HashTable.h"
#include "Basic/Assert.h"
#include "Basic/Log.h"

auto keysymsToScancodes = NewHashTableIn<u32, u32>(GlobalHeap(), 0, HashU32);

void InitializePlatformInput()
{
    auto syms = xcb_key_symbols_alloc(XCBConnection());
    if (!syms)
    {
    	Abort("Input", "Failed xcb_key_symbols_alloc().");
    }
    Defer(xcb_key_symbols_free(syms));
    auto AddKey = [syms](u32 k)
    {
		auto kc = xcb_key_symbols_get_keycode(syms, k);
		if (!kc)
		{
			Abort("Input", "Failed xcb_key_symbols_get_keycode for KeySymbol %d.", k);
		}
		keysymsToScancodes.Insert(k, *kc);
		free(kc);
    };
    AddKey(XK_BackSpace);
	AddKey(XK_Tab);
	AddKey(XK_Linefeed);
	AddKey(XK_Clear);
	AddKey(XK_Return);
	AddKey(XK_Pause);
	AddKey(XK_Scroll_Lock);
	AddKey(XK_Sys_Req);
	AddKey(XK_Escape);
	AddKey(XK_Delete);
	AddKey(XK_Home);
	AddKey(XK_Left);
	AddKey(XK_Up);
	AddKey(XK_Right);
	AddKey(XK_Down);
	AddKey(XK_Prior);
	AddKey(XK_Page_Up);
	AddKey(XK_Next);
	AddKey(XK_Page_Down);
	AddKey(XK_End);
	AddKey(XK_Begin);
	AddKey(XK_KP_Space);
	AddKey(XK_KP_Tab);
	AddKey(XK_KP_Enter);
	AddKey(XK_KP_F1);
	AddKey(XK_KP_F2);
	AddKey(XK_KP_F3);
	AddKey(XK_KP_F4);
	AddKey(XK_KP_Home);
	AddKey(XK_KP_Left);
	AddKey(XK_KP_Up);
	AddKey(XK_KP_Right);
	AddKey(XK_KP_Down);
	AddKey(XK_KP_Prior);
	AddKey(XK_KP_Page_Up);
	AddKey(XK_KP_Next);
	AddKey(XK_KP_Page_Down);
	AddKey(XK_KP_End);
	AddKey(XK_KP_Begin);
	AddKey(XK_KP_Add);
	AddKey(XK_KP_Delete);
	AddKey(XK_KP_Equal);
	AddKey(XK_KP_Multiply);
	AddKey(XK_KP_Add);
	AddKey(XK_KP_Separator);
	AddKey(XK_KP_Subtract);
	AddKey(XK_KP_Decimal);
	AddKey(XK_KP_Divide);
	AddKey(XK_KP_0);
	AddKey(XK_KP_1);
	AddKey(XK_KP_2);
	AddKey(XK_KP_3);
	AddKey(XK_KP_4);
	AddKey(XK_KP_5);
	AddKey(XK_KP_6);
	AddKey(XK_KP_7);
	AddKey(XK_KP_8);
	AddKey(XK_KP_9);
	AddKey(XK_F1);
	AddKey(XK_F2);
	AddKey(XK_F3);
	AddKey(XK_F4);
	AddKey(XK_F5);
	AddKey(XK_F6);
	AddKey(XK_F7);
	AddKey(XK_F8);
	AddKey(XK_F9);
	AddKey(XK_F10);
	AddKey(XK_F11);
	AddKey(XK_F12);
	AddKey(XK_Shift_L);
	AddKey(XK_Shift_R);
	AddKey(XK_Control_L);
	AddKey(XK_Control_R);
	AddKey(XK_Caps_Lock);
	AddKey(XK_Shift_Lock);
	AddKey(XK_Meta_L);
	AddKey(XK_Meta_R);
	AddKey(XK_Alt_L);
	AddKey(XK_Alt_R);
	AddKey(XK_Super_L);
	AddKey(XK_Super_R);
	AddKey(XK_Hyper_L);
	AddKey(XK_Hyper_R);
	AddKey(XK_space);
	AddKey(XK_exclam);
	AddKey(XK_quotedbl);
	AddKey(XK_numbersign);
	AddKey(XK_dollar);
	AddKey(XK_percent);
	AddKey(XK_ampersand);
	AddKey(XK_apostrophe);
	AddKey(XK_quoteright);
	AddKey(XK_parenleft);
	AddKey(XK_parenright);
	AddKey(XK_asterisk);
	AddKey(XK_plus);
	AddKey(XK_comma);
	AddKey(XK_minus);
	AddKey(XK_period);
	AddKey(XK_slash);
	AddKey(XK_0);
	AddKey(XK_1);
	AddKey(XK_2);
	AddKey(XK_3);
	AddKey(XK_4);
	AddKey(XK_5);
	AddKey(XK_6);
	AddKey(XK_7);
	AddKey(XK_8);
	AddKey(XK_9);
	AddKey(XK_colon);
	AddKey(XK_semicolon);
	AddKey(XK_less);
	AddKey(XK_equal);
	AddKey(XK_greater);
	AddKey(XK_question);
	AddKey(XK_at);
	AddKey(XK_A);
	AddKey(XK_B);
	AddKey(XK_C);
	AddKey(XK_D);
	AddKey(XK_E);
	AddKey(XK_F);
	AddKey(XK_G);
	AddKey(XK_H);
	AddKey(XK_I);
	AddKey(XK_J);
	AddKey(XK_K);
	AddKey(XK_L);
	AddKey(XK_M);
	AddKey(XK_N);
	AddKey(XK_O);
	AddKey(XK_P);
	AddKey(XK_Q);
	AddKey(XK_R);
	AddKey(XK_S);
	AddKey(XK_T);
	AddKey(XK_U);
	AddKey(XK_V);
	AddKey(XK_W);
	AddKey(XK_X);
	AddKey(XK_Y);
	AddKey(XK_Z);
	AddKey(XK_bracketleft);
	AddKey(XK_backslash);
	AddKey(XK_bracketright);
	AddKey(XK_asciicircum);
	AddKey(XK_underscore);
	AddKey(XK_grave);
	AddKey(XK_quoteleft);
	AddKey(XK_a);
	AddKey(XK_b);
	AddKey(XK_c);
	AddKey(XK_d);
	AddKey(XK_e);
	AddKey(XK_f);
	AddKey(XK_g);
	AddKey(XK_h);
	AddKey(XK_i);
	AddKey(XK_j);
	AddKey(XK_k);
	AddKey(XK_l);
	AddKey(XK_m);
	AddKey(XK_n);
	AddKey(XK_o);
	AddKey(XK_p);
	AddKey(XK_q);
	AddKey(XK_r);
	AddKey(XK_s);
	AddKey(XK_t);
	AddKey(XK_u);
	AddKey(XK_v);
	AddKey(XK_w);
	AddKey(XK_x);
	AddKey(XK_y);
	AddKey(XK_z);
	AddKey(XK_braceleft);
	AddKey(XK_bar);
	AddKey(XK_braceright);
	AddKey(XK_asciitilde);
}

u32 KeySymbolToScancode(KeySymbol k)
{
	auto sc = keysymsToScancodes.Lookup(k, -1);
	if (sc == -1)
	{
		Abort("Input", "Failed scancode hashtable lookup for KeySymbol %d.", k);
	}
	return sc;
}

KeySymbol ScancodeToKeySymbol(u32 scancode)
{
	// @TODO
	return KeySymbol{};
}

void QueryMousePosition(PlatformWindow *w, s32 *x, s32 *y)
{
    auto err = (xcb_generic_error_t *){};
	auto cookie = xcb_query_pointer(XCBConnection(), w->xcbHandle);
	auto reply = xcb_query_pointer_reply(XCBConnection(), cookie, &err);
	if (err)
	{
		LogError("Input", "Failed xcb_query_pointer_reply: error_code %hu, major_code: %hu, minor_code: %hu, sequence: %hu.", err->error_code, err->major_code, err->minor_code, err->sequence);
		return;
	}
	*x = reply->win_x;
	*y = reply->win_y;
	free(reply);
	//*y = (w->height - *y); // Bottom left is zero for us, top left is zero for x11.
}
