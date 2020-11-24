#include "Log.h"
#include "Basic/File.h"
#include "Basic/Process.h"

namespace log
{

void ConsoleWrite(str::String s)
{
	File{1}.WriteString(s);
}

str::String OSError()
{
	return strerror(errno);
}

}
