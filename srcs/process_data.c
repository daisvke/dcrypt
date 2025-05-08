#include "dcrypt.h"

/*
 * Encrypt/decrypt the mapped data
 */

int process_mapped_data(t_env *env)
{
	bool reverse = env->modes & DC_REVERSE;
	// Only skip encrypting the custom header in reverse mode
	unsigned char *data = reverse ?
		env->mapped_data + DC_DCRYPT_HEADER_SIZE : env->mapped_data;

	if (env->modes & DC_REVERSE) { // Decryption mode
		if (env->modes & DC_VERBOSE)
			printf(FMT_INFO " Starting decryption...\n");

		// We need to convert the hex string key given from the command line to its byte representation
		unsigned char	key[DC_AES_KEY_SIZE + 1];
		hexstr_to_bytes(env->decryption_key, key, DC_AES_KEY_SIZE);
		key[DC_AES_KEY_SIZE] = '\0';

		int	decrypted_size;
		if ((decrypted_size = aes_decrypt_data(
			env,
			data,									// The file data (starting after the header)
			env->encrypted_filesize,				// The encrypted file size (without the custom header size)
			#ifdef _WIN32
			// Import raw AES key to to get a key handle linked to the key
			import_raw_aes_key(env, key, DC_AES_KEY_SIZE),
			# else
			key,									// The randomly generated encryption key
			#endif
			env->dcrypt_header.iv_key				// Initialization vector
		)) == DC_CRYPT_ERROR) return DC_ERROR;

		if (decrypted_size != (int)env->dcrypt_header.original_filesize &&
			(env->modes & DC_VERBOSE)
		) {
			fprintf(
				stderr,
				FMT_ERROR
				"Decrypted data length (%d) doesn't match original filesize (%ld).\n",
				decrypted_size, env->dcrypt_header.original_filesize
			);
			return DC_ERROR;
		}

		if (env->modes & DC_VERBOSE) {
			printf(FMT_INFO " Decrypted %u bytes.\n", env->encrypted_filesize);
			printf(FMT_DONE "Decryption complete.\n");
		}
	}
	else // Encryption mode
	{
		if (env->modes & DC_VERBOSE)
			printf(FMT_INFO "Starting encryption...\n");

		if ((env->encrypted_filesize = aes_encrypt_data(
			env,
			data,										// The file data (starting after the header)
			&env->encrypted_data,
			env->dcrypt_header.original_filesize,		// The original file size
			#ifdef _WIN32
			win_env.hKey,
			# else
			env->encryption_key,						// The randomly generated encryption key
			#endif
			env->dcrypt_header.iv_key					// Initialization vector
		)) == DC_CRYPT_ERROR) return DC_ERROR;

		if (env->modes & DC_VERBOSE) {
			printf(
				FMT_INFO
				" Data size after encryption (custom header excluded): %zu bytes.\n",
				env->dcrypt_header.original_filesize
			);
			printf(FMT_DONE "Encryption complete.\n");
		}	
	}

	return DC_SUCCESS;
}
