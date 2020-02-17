#include <errno.h>
#include <string.h>

const char *GetPlatformError() {
	return strerror(errno);
}
