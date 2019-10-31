typedef struct Directory_Iteration {
	DIR *dir;
	struct dirent *dirent;
	String filename;
	bool is_directory;
} Directory_Iteration;

u32 Get_Number_Of_Files_In_Directory(const char *path) {
	DIR *dir = opendir(path);
	assert(dir);
	struct dirent *entry;
	s32 file_count = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
			file_count++;
		}
	}
	closedir(dir);
	return file_count;
}

u8 Iterate_Through_All_Files_In_Directory(String path, Directory_Iteration *context) {
	if (!context->dir) { // First read.
		context->dir = opendir(path.data);
		if (!context->dir) {
			printf("Failed to open directory %s: %s\n", path.data, strerror(errno));
			return 0;
		}
	}
	while ((context->dirent = readdir(context->dir))) {
		if (!strcmp(context->dirent->d_name, ".") || !strcmp(context->dirent->d_name, "..")) {
			continue;
		}
		context->filename = S(context->dirent->d_name);
		context->is_directory = (context->dirent->d_type == DT_DIR);
		return 1;
	}
	return 0;
}

void Read_Entire_File(String path, char *output) {
	FILE *file_handle = fopen(path.data, "r");
	if (!file_handle) {
		Abort("Failed to open file %s: %s\n", path.data, strerror(errno));
	}
	fseek(file_handle, 0L, SEEK_END);
	s64 file_length = ftell(file_handle);
	fseek(file_handle, 0L, SEEK_SET);
	size_t read_byte_count = fread(output, 1, file_length, file_handle);
	if (read_byte_count != file_length) {
		Abort("Failed to read file %s: %s\n", path.data, strerror(errno));
	}
	output[file_length] = '\0';
	fclose(file_handle);
}

void _Join_Filepaths(const char *a, const char *b, char *output) {
	size_t a_length = strlen(a);
	size_t b_length = strlen(b);
	strcpy(output, a);
	output[a_length] = '/';
	strcpy(output + a_length + 1, b);
	output[a_length + b_length + 1] = '\0';
}

String Join_Filepaths(String a, String b) {
	u32 result_length = a.length + b.length + 1;
	String result = Create_String(result_length);
	Append_String(&result, a);
	Append_String(&result, S("/"));
	Append_String(&result, b);
	result.data[result_length - 1];
	return result;
}
