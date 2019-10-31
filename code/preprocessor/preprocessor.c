#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <execinfo.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

void Abort(const char *format, ...) {
	va_list arguments;
	va_start(arguments, format);
	printf("###########################################################################\n");
	printf("[PROGRAM ABORT]\n");
	vprintf(format, arguments);
	printf("###########################################################################\n");
	va_end(arguments);
	assert(0);
	exit(1);
}

void Copy_Memory(const void *source, void *destination, size_t size) {
	memcpy(destination, source, size);
}

#include "../platform.h"
#include "../strings.h"
#include "../strings.c"
#include "parse.c"
#include "filesystem.c"
#include "materials.c"

s32 main(s32 argc, char **argv) {
	Compile_Materials();
}
