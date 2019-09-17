String_Result read_entire_file(const char *path, Memory_Arena *arena) {
	File file = platform_open_file(path, O_RDONLY); // @TODO: Platform generic flags.
	if (file == FILE_HANDLE_ERROR) {
		return (String_Result){NULL, 0};
	}
	File_Offset file_length = platform_get_file_length(file);
	char *string_buffer = allocate_array(arena, char, (file_length+1));
	u8 read_result = platform_read_file(file, file_length, string_buffer);
	if (!read_result) {
		return (String_Result){NULL, 0};
	}
	return (String_Result){string_buffer, file_length};
}

// Returns all but the last component of the path.
String get_directory_from_path(String path, Memory_Arena *arena) {
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
String get_filename_from_path(String path, Memory_Arena *arena) {
	u32 slash_index = find_last_occurrence_of_character(path, '/');
	if (slash_index == U32_MAX) {
		return S("");
	}
	u32 filename_length = path.length - (slash_index + 1);
	String filename = create_string(filename_length, arena);
	append_string_range(&filename, path, slash_index + 1, filename_length);
	filename.data[filename_length] = '\0';
	debug_print("GET %s %s %u %u %u\n", path.data, filename.data, path.length, filename.length, slash_index);
	return filename;
}

// Concatenates two strings and inserts a '/' between them.
String join_filepaths(String a, String b, Memory_Arena *arena) {
	debug_print("%s %s %u %u\n", a.data, b.data, a.length, b.length);
	u32 result_length = a.length + b.length + 1;
	String result = create_string(result_length, arena);
	append_string(&result, a);
	append_string(&result, S("/"));
	append_string(&result, b);
	result.data[result_length - 1];
	return result;
}
