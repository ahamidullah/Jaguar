#pragma once

String ReadEntireFile(const String &path, u8 *error);
bool WriteStringToFile(FileHandle file, const String &string);
