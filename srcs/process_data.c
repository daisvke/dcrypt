#include "stockholm.h"

/*
 * Encrypt/decrypt the mapped data
 */

int process_mapped_data(t_env *env)
{
	const unsigned char *key = get_encryption_key(env);
	if (!key) return SH_ERROR;

	bool reverse = env->g_modes & SH_REVERSE;
	// Only skip encrypting the custom header in reverse mode
	unsigned char *data = reverse ? env->g_mapped_data + SH_STOCKHLM_HEADER_SIZE : env->g_mapped_data;

	// xor_with_additive_cipher(
	// 	key,								 // The randomly generated encryption key
	// 	SH_ENCRYPT_KEY_SIZE,				 // The key width
	// 	(void *)data,						 // The file data (starting after the header)
	// 	env->g_stockhlm_header.original_filesize, // The original file size
	// 	reverse								 // Encryption/decryption mode
	// );

	if (env->g_modes & SH_REVERSE) { // Decryption mode
		if (env->g_modes & SH_VERBOSE)
			printf(FMT_INFO " Starting decryption...\n");

		if (aes_decrypt_data(
			data,									// The file data (starting after the header)
			env->g_encrypted_filesize,				// The original file size
			env->g_decryption_key,					// The randomly generated encryption key
			NULL									// Initialization vector
		) == -1) return SH_ERROR;

		if (env->g_modes & SH_VERBOSE) {
			printf(FMT_INFO " Decrypted %zu bytes.\n", env->g_encrypted_filesize);
			printf(FMT_DONE "Decryption complete.");
		}	
	}
	else // Encryption mode
	{
		if (env->g_modes & SH_VERBOSE)
			printf(FMT_INFO " Starting encryption...\n");

		if ((env->g_encrypted_filesize = aes_encrypt_data(
			data,									// The file data (starting after the header)
			env->g_stockhlm_header.original_filesize,	// The original file size
			key,									// The randomly generated encryption key
			NULL									// Initialization vector
		)) == -1) return SH_ERROR;

		if (env->g_modes & SH_VERBOSE) {
			printf(
				FMT_INFO
				" Data size after encryption (custom header excluded): %zu bytes.\n",
				env->g_stockhlm_header.original_filesize
			);
			printf(FMT_DONE "Encryption complete.\n");
		}	
	}

	if (!reverse && !env->g_encryption_key)
	{
		free((void *)key);
		key = NULL;
	}

	return SH_SUCCESS;
}
