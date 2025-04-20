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
			// Import raw AES key to to get a key handle linked to the key
			import_raw_aes_key(env, win_env.hProv, "824238A6C65F0C698E290A41F2B67857", DC_AES_KEY_SIZE),
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
			&env->encrypted_data,
			env->dcrypt_header.original_filesize,		// The original file size
			#ifdef _WIN32
			win_env.hKey,
			# else
			env->encryption_key,						// The randomly generated encryption key
			#endif
			env->dcrypt_header.iv_key					// Initialization vector
		)) == -1) return DC_ERROR;

// 		DWORD keyLen = 0;
// DWORD size = sizeof(DWORD);
// if (!CryptGetKeyParam(win_env.hKey, KP_KEYLEN, (BYTE *)&keyLen, &size, 0)) {
//     printf("Key handle is invalid before decryption: %lu\n", GetLastError());
//     return -1;
// }

// 		if (aes_decrypt_data(
// 			env->encrypted_data,						// The file data (starting after the header)
// 			env->encrypted_filesize,					// The encrypted file size (without the custom header size)
// 			#ifdef _WIN32
// 			// Import raw AES key to to get a key handle linked to the key
// 			win_env.hKey,
// 			# else
// 			env->decryption_key,					// The randomly generated encryption key
// 			#endif
// 			env->dcrypt_header.iv_key				// Initialization vector
// 		) == -1) return DC_ERROR;

		printf("encryyyyy size: %X %X\n",env->encrypted_data[0],env->encrypted_data[1]);
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
