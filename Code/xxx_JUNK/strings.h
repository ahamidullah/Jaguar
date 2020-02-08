#pragma once

struct String {
	char *data;
	u32 length;
	u32 capacity;
	u8 is_constant;
};

