#pragma once

#include "../Window.h"
#include "../Input.h"

struct PlatformEvents
{
	bool quit;
};

struct InputButtons;
struct Mouse;
PlatformEvents ProcessPlatformEvents(Window *w, InputButtons *kb, Mouse *m);
