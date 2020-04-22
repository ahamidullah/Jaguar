#pragma once

String ReadEntireFile(const String &path, bool *error);
PlatformTime GetFilepathLastModifiedTime(const String &filepath, bool *error);
