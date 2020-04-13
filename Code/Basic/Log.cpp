void ConsolePrintVarArgs(const String &format, va_list arguments)
{
	WriteToConsole(FormatStringVarArgs(format, arguments))
}

void ConsolePrint(const String &format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	ConsolePrintVarArgs(format, arguments);
	va_end(arguments);
}

void LogPrintVarArgs(LogType logType, const char *format, va_list arguments)
{
	// @TODO: Print message to a log file as well.
	if (debug)
	{
		ConsolePrintVarArgs(format, arguments);
	}
}

// @TODO: Handle logType.
void LogPrint(LogType logType, const String &format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	LogPrintVarArgs(logType, format, arguments);
	va_end(arguments);
}
