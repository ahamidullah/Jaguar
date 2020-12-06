#include "File.h"
#include "../Log.h"

namespace fs
{

File Open(str::String path, s64 flags, bool *err)
{
	auto f = File
	{
		.handle = open(path.CString(), flags, 0666),
		.path = path.Copy(),
	};
	if (f.handle < 0)
	{
		log::Error("File", "Failed to open file %k: %k.", path, PlatformError());
		*err = true;
		return {};
	}
	return f;
}

bool File::IsOpen()
{
	return this->path != "";
}

bool File::Close()
{
	if (close(this->handle) == -1)
	{
		log::Error("File", "Failed to close file %k: %k.", this->path, PlatformError());
		return false;
	}
	this->handle = -1;
	this->path = "";
	return true;
}

bool File::Read(arr::View<u8> out)
{
	auto totRead = 0;
	auto curRead = 0; // Maximum number of bytes that can be returned by a read. (Like size_t, but signed.)
	auto cursor = &out[0];
	do
	{
		curRead = read(this->handle, cursor, out.count - totRead);
		totRead += curRead;
		cursor += curRead;
	} while (totRead < out.count && curRead != 0 && curRead != -1);
	if (curRead == -1)
	{
		log::Error("File", "Failed to read file %k: %k.", this->path, PlatformError());
		return false;
	}
	else if (totRead != out.count)
	{
		log::Error("File", "Failed to read file %k: could only read %lu bytes, but %lu bytes were requested.", this->path, totRead, out.count);
		return false;
	}
	return true;
}

bool File::Write(arr::View<u8> a)
{
	auto totWrit = size_t{0};
	auto curWrit = ssize_t{0}; // Maximum number of bytes that can be returned by a write. (Like size_t, but signed.)
	auto cursor = &a[0];
	do
	{
		curWrit = write(this->handle, cursor, (a.count - totWrit));
		totWrit += curWrit;
		cursor += curWrit;
	} while (totWrit < a.count && curWrit != 0);
	if (totWrit != a.count)
	{
		log::Error("File", "Failed to write file %k: %k.", this->path, PlatformError());
		return false;
	}
	return true;
}

bool File::WriteString(str::String s)
{
	return this->Write(s.buffer);
}

// @TODO: Handle files larger than 2GB?
s64 File::Length(bool *err)
{
	auto stat = (struct stat){};
	if (fstat(this->handle, &stat) == -1)
	{
		log::Error("File", "Failed get length of file %k: %k.", this->path, PlatformError());
		*err = true;
		return 0;
	}
	return stat.st_size;
}

s64 File::Seek(s64 seek, FileSeekRelative rel, bool *err)
{
	auto off = lseek(this->handle, seek, (s32)rel);
	if (off == (off_t)-1)
	{
		log::Error("File", "Failed to seek file %k: %k.", this->path, PlatformError());
		*err = true;
		return 0;
	}
	return off;
}

time::Time File::LastModifiedTime(bool *err)
{
	auto stat = (struct stat){};
	if (fstat(this->handle, &stat) == -1)
	{
		log::Error("File", "Failed to get last modified time of file %k: %k.", this->path, PlatformError());
		*err = true;
		return {};
	}
	return
	{
		.ts = stat.st_mtim,
	};
}

bool Exists(str::String path)
{
	if (access(path.CString(), F_OK) != -1)
	{
		return true;
	}
	return false;
}

bool Delete(str::String path)
{
	if (unlink(path.CString()) != 0)
	{
		log::Error("File", "Failed to delete file %k: %k.", path, PlatformError());
		return false;
	}
	return true;
}

}
