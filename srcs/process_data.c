#include "dcrypt.h"

/*
 * Encrypt/decrypt the mapped data
 */

int process_mapped_data(t_env *env)
{
	const unsigned char *key = get_encryption_key(env);
	if (!key) return DCERROR;

	bool reverse = env->modes & DCREVERSE;
	// Only skip encrypting the custom header in reverse mode
	unsigned char *data = reverse ? env->mapped_data + DCDCRYPT_HEADER_SIZE : env->mapped_data;

	// xor_with_additive_cipher(
	// 	key,								 // The randomly generated encryption key
	// 	DCENCRYPT_KEY_SIZE,				 // The key width
	// 	(void *)data,						 // The file data (starting after the header)
	// 	env->dcrypt_header.original_filesize, // The original file size
	// 	reverse								 // Encryption/decryption mode
	// );

	if (env->modes & DCREVERSE) { // Decryption mode
		if (env->modes & DCVERBOSE)
			printf(FMT_INFO " Starting decryption...\n");

		if (aes_decrypt_data(
			data,									// The file data (starting after the header)
			env->encrypted_filesize,				// The original file size
			env->decryption_key,					// The randomly generated encryption key
			NULL									// Initialization vector
		) == -1) return DCERROR;

		if (env->modes & DCVERBOSE) {
			printf(FMT_INFO " Decrypted %zu bytes.\n", env->encrypted_filesize);
			printf(FMT_DONE "Decryption complete.\n");
		}	
	}
	else // Encryption mode
	{
		if (env->modes & DCVERBOSE)
			printf(FMT_INFO " Starting encryption...\n");

		if ((env->encrypted_filesize = aes_encrypt_data(
			data,										// The file data (starting after the header)
			env->dcrypt_header.original_filesize -1,	// The original file size
			key,										// The randomly generated encryption key
			NULL										// Initialization vector
		)) == -1) return DCERROR;

		if (env->modes & DCVERBOSE) {
			printf(
				FMT_INFO
				" Data size after encryption (custom header excluded): %zu bytes.\n",
				env->dcrypt_header.original_filesize
			);
			printf(FMT_DONE "Encryption complete.\n");
		}	
	}

	if (!reverse && !env->encryption_key)
	{
		free((void *)key);
		key = NULL;
	}

	return DCSUCCESS;
}
