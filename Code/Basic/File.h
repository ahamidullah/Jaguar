#pragma once

struct OpenFileResult {
	FileHandle file;
	bool error;
};

struct ReadFileResult {
	String string;
	bool error;
};

ReadFileResult ReadEntireFile(const String &path);
