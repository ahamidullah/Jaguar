#pragma once

struct PlatformDirectoryIteration
{
	DIR *dir;
	struct dirent *dirent;
	char *filename;
	u8 isDirectory;
};

typedef s32 PlatformFileHandle;
typedef u64 PlatformFileOffset;

enum PlatformFileSeekRelative
{
	FILE_SEEK_START = SEEK_SET,
	FILE_SEEK_CURRENT = SEEK_CUR,
	FILE_SEEK_END = SEEK_END
};

enum PlatformOpenFileFlags
{
	OPEN_FILE_READ_ONLY = O_RDONLY,
};

constexpr PlatformFileHandle FILE_HANDLE_ERROR = -1;
constexpr PlatformFileOffset FILE_OFFSET_ERROR = (PlatformFileOffset)-1;
