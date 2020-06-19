#pragma once

struct String;

void PrintStacktrace();
String GetPlatformError();
void WriteToConsole(const String &writeString);
