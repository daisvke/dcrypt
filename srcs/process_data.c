#include "stockholm.h"

/*
 * Encrypt/decrypt the mapped data
 */

int fa_process_mapped_data(void)
{
	const unsigned char *key = get_encryption_key();
	if (!key) return FA_ERROR;

	bool reverse = g_modes & FA_REVERSE;
	// Only skip encrypting the custom header in reverse mode
	unsigned char *data = reverse ? g_mapped_data + FA_STOCKHLM_HEADER_SIZE : g_mapped_data;

	// xor_with_additive_cipher(
	// 	key,								 // The randomly generated encryption key
	// 	FA_ENCRYPT_KEY_SIZE,				 // The key width
	// 	(void *)data,						 // The file data (starting after the header)
	// 	g_stockhlm_header.original_filesize, // The original file size
	// 	reverse								 // Encryption/decryption mode
	// );

	if (g_modes & FA_REVERSE) { // Decryption mode
		if (g_modes & FA_VERBOSE)
			printf("\n > STARTING DECRYPTION...\n\n");

		if (aes_decrypt_data(
			data,									// The file data (starting after the header)
			g_encrypted_filesize,					// The original file size
			key,									// The randomly generated encryption key
			NULL									// Initialization vector
		) == -1) return FA_ERROR;
	}
	else // Encryption mode
	{
		if (g_modes & FA_VERBOSE)
			printf("\n > STARTING ENCRYPTION...\n\n");

		if ((g_encrypted_filesize = aes_encrypt_data(
			data,									// The file data (starting after the header)
			g_stockhlm_header.original_filesize,	// The original file size
			key,									// The randomly generated encryption key
			NULL									// Initialization vector
		)) == -1) return FA_ERROR;
	}
	printf("encrypted filesize: %d\n", g_encrypted_filesize);

	if (g_modes & FA_VERBOSE)
		printf(FA_GREEN_COLOR "Done!\n\n" FA_RESET_COLOR);

	if (!reverse)
	{
		free((void *)key);
		key = NULL;
	}

	return FA_SUCCESS;
}
