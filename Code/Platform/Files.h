#pragma once

#if defined(__linux__)
	#include "Platform/Linux/Files.h"
#else
	#error unsupported platform
#endif

#include "Common/Strings.h"

struct PlatformOpenFileResult
{
	PlatformFileHandle file;
	bool error;
};

struct PlatformReadFileResult
{
	String string;
	bool error;
};

bool PlatformIterateThroughDirectory(const char *path, PlatformDirectoryIteration *context);
PlatformOpenFileResult PlatformOpenFile(const String &path, PlatformOpenFileFlags flags);
bool PlatformCloseFile(PlatformFileHandle file);
PlatformReadFileResult PlatformReadFromFile(PlatformFileHandle file, size_t numberOfBytesToRead);
PlatformFileOffset PlatformGetFileLength(PlatformFileHandle file);
PlatformFileOffset PlatformSeekInFile(PlatformFileHandle file, PlatformFileOffset offset, PlatformFileSeekRelative relative);
bool PlatformWriteToFile(PlatformFileHandle file, size_t count, const void *buffer);
