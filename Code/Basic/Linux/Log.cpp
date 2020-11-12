#include "../Log.h"
#include "../File.h"
#include "../Process.h"

void ConsoleWrite(string::String s)
{
	File{1}.WriteString(s);
}

string::String PlatformError()
{
	return strerror(errno);
}
