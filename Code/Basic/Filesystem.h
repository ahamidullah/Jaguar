#pragma once

String GetFilepathDirectory(const String &path);
String GetFilepathFilename(const String &path);
String GetFilepathExtension(const String &path);
void SetFilepathExtension(String *path, const String &newExtension);
String JoinFilepaths(const String &a, const String &b);
String CleanFilepath(const String &filepath);