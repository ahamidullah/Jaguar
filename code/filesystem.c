String_Result Read_Entire_File(const char *path, Memory_Arena *arena) {
	Platform_File_Handle file = Platform_Open_File(path, OPEN_FILE_READ_ONLY);
	if (file == PLATFORM_FILE_HANDLE_ERROR) {
		return (String_Result){NULL, 0};
	}
	Platform_File_Offset file_length = Platform_Get_File_Length(file);
	char *string_buffer = allocate_array(arena, char, (file_length+1));
	u8 read_result = Platform_Read_From_File(file, file_length, string_buffer);
	if (!read_result) {
		return (String_Result){NULL, 0};
	}
	return (String_Result){string_buffer, file_length};
}

// Returns all but the last component of the path.
String Get_Directory_From_Path(String path, Memory_Arena *arena) {
	u32 slash_index = find_last_occurrence_of_character(path, '/');
	if (slash_index == U32_MAX) {
		return S("");
	}
	u32 directory_length = slash_index;
	String directory = create_string(directory_length, arena);
	append_string_range(&directory, path, 0, directory_length);
	directory.data[directory_length] = '\0';
	return directory;
}

// Returns the last component of the path.
String Get_Filename_From_Path(String path, Memory_Arena *arena) {
	u32 slash_index = find_last_occurrence_of_character(path, '/');
	if (slash_index == U32_MAX) {
		return S("");
	}
	u32 filename_length = path.length - (slash_index + 1);
	String filename = create_string(filename_length, arena);
	append_string_range(&filename, path, slash_index + 1, filename_length);
	filename.data[filename_length] = '\0';
	return filename;
}

// Concatenates two strings and inserts a '/' between them.
String Join_Filepaths(String a, String b, Memory_Arena *arena) {
	debug_print("%s %s %u %u\n", a.data, b.data, a.length, b.length);
	u32 result_length = a.length + b.length + 1;
	String result = create_string(result_length, arena);
	append_string(&result, a);
	append_string(&result, S("/"));
	append_string(&result, b);
	result.data[result_length - 1];
	return result;
}
