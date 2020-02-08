#include "Filesystem.h"

PlatformReadFileResult ReadEntireFile(const String &path) {
	auto [file, error] = PlatformOpenFile(path, OPEN_FILE_READ_ONLY);
	if (error) {
		PlatformReadFileResult{};
	}
	Defer(PlatformCloseFile(file));
	PlatformFileOffset fileLength = PlatformGetFileLength(file);
	return PlatformReadFromFile(file, fileLength);
}

// Returns all but the last component of the path.
String GetDirectoryFromPath(const String &path) {
	s64 slashIndex = FindLastOccurrenceOfCharacter(path, '/');
	if (slashIndex < 0) {
		return String{};
	}
	u32 directoryLength = slashIndex;
	String directory = CreateString(directoryLength);
	Copy_Memory(&path[0], &directory[0], directoryLength);
	return directory;
}

// Returns the last component of the path.
String GetFilenameFromPath(const String &path) {
	s64 slashIndex = FindLastOccurrenceOfCharacter(path, '/');
	if (slashIndex < 0) {
		return String{};
	}
	u32 filenameLength = Length(path) - (slashIndex + 1);
	String filename = CreateString(filenameLength);
	Copy_Memory(&path[0] + slashIndex + 1, &filename[0], filenameLength);
	return filename;
}

// Concatenates two strings and inserts a '/' between them.
String JoinFilepaths(const String &a, const String &b) {
	String result = CreateString(Length(a) + Length(b) + 1);
	Copy_Memory(&a[0], &result[0], Length(a));
	result[Length(a)] = '/';
	Copy_Memory(&b[0], &result[0] + Length(a) + 1, Length(b));
	return result;
}
