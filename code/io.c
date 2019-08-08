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

const char *get_directory(const char *path, Memory_Arena *arena) {
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

char *join_paths(const char *path_1, const char *path_2, Memory_Arena *arena) {
	size_t path_1_length = string_length(path_1);
	size_t path_2_length = string_length(path_2);
	char *result = allocate_array(arena, char, path_1_length + path_2_length + 2);
	copy_string(result, path_1);
	result[path_1_length] = '/';
	copy_string(result + path_1_length + 1, path_2);
	result[path_1_length + path_2_length + 1] = '\0';
	return result;
}
