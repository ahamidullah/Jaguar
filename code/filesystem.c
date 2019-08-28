String_Result read_entire_file(const char *path, Memory_Arena *arena) {
	File_Handle file_handle = open_file(path, O_RDONLY); // @TODO: Platform generic flags.
	if (file_handle == FILE_HANDLE_ERROR) {
		return (String_Result){NULL, 0};
	}
	File_Offset file_length = get_file_length(file_handle);
	char *string_buffer = allocate_array(arena, char, (file_length+1));
	u8 read_result = read_file(file_handle, file_length, string_buffer);
	if (!read_result) {
		return (String_Result){NULL, 0};
	}
	return (String_Result){string_buffer, file_length};
}

char *get_directory(const char *path, Memory_Arena *arena) {
	const char *slash = find_last_occurrence_of_character(path, '/');
	if (slash == NULL) {
		return "";
	}
	size_t directory_length = slash - path;
	char *directory = allocate_array(arena, char, directory_length + 1);
	copy_string_range(directory, path, directory_length);
	directory[directory_length] = '\0';
	return directory;
}

// get_filename_from_path returns the last component of the path.
char *get_filename_from_path(const char *path, Memory_Arena *arena) {
	size_t path_length = string_length(path);
	const char *slash = find_last_occurrence_of_character(path, '/');
	if (slash == NULL) {
		return "";
	}
	size_t result_length = (path + path_length) - (slash + 1) + 1;
	char *result = allocate_array(arena, char, result_length);
	copy_string(result, slash + 1);
	return result;
}

char *join_paths(const char *a, const char *b, Memory_Arena *arena) {
	size_t a_length = string_length(a);
	size_t b_length = string_length(b);
	char *result = allocate_array(arena, char, a_length + b_length + 2);
	copy_string(result, a);
	result[a_length] = '/';
	copy_string(result + a_length + 1, b);
	result[a_length + b_length + 1] = '\0';
	return result;
}
