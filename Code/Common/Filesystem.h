#pragma once

PlatformReadFileResult ReadEntireFile(const String &path);
String GetDirectoryFromPath(String path);
String GetFilenameFromPath(const String &path);
String JoinFilepaths(const String &a, const String &b);
