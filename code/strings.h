#if !defined(STRINGS_H)
#define STRINGS_H

typedef struct String {
	char *data;
	u32 length;
	u32 capacity;
	u8 is_constant;
} String;

#endif
