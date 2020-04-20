#pragma once

String ReadEntireFile(const String &path, bool *error);
bool WriteStringToFile(FileHandle file, const String &string);
PlatformTime GetFilepathLastModifiedTime(const String &filepath, bool *error);
