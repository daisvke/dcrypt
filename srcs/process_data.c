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

		if (aes_decrypt_data(
			data,									// The file data (starting after the header)
			env->encrypted_filesize,				// The encrypted file size (without the custom header size)
			#ifdef _WIN32
			import_raw_aes_key(win_env.hProv, env->decryption_key, strlen((const char *)env->decryption_key)),
			# else
			env->decryption_key,					// The randomly generated encryption key
			#endif
			env->dcrypt_header.iv_key				// Initialization vector
		) == -1) return DC_ERROR;

		if (env->modes & DC_VERBOSE) {
			printf(FMT_INFO " Decrypted %zu bytes.\n", env->encrypted_filesize);
			printf(FMT_DONE "Decryption complete.\n");
		}	
	}
	else // Encryption mode
	{
		if (env->modes & DC_VERBOSE)
			printf(FMT_INFO " Starting encryption...\n");

		if ((env->encrypted_filesize = aes_encrypt_data(
			data,										// The file data (starting after the header)
			env->dcrypt_header.original_filesize,		// The original file size
			#ifdef _WIN32
			win_env.hKey,
			# else
			env->encryption_key,						// The randomly generated encryption key
			#endif
			env->dcrypt_header.iv_key					// Initialization vector
		)) == -1) return DC_ERROR;

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
