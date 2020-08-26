#include "../Log.h"
#include "../File.h"
#include "../Process.h"

void ConsoleWrite(String s)
{
	File{1}.WriteString(s);
}

String PlatformError()
{
	return strerror(errno);
}
