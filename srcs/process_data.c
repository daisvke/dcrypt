#include "stockholm.h"

char *get_encryption_key(void)
{
	char *key;

	// In decryption mode, we use the key saved in the file header
	if (g_modes & FA_REVERSE)
	{
		key = (char *)g_stockhlm_header.encryption_key;

		if (g_modes & FA_VERBOSE)
			printf("Using encryption key => " FA_YELLOW_COLOR "%s\n", key);
	}
	else // In encryption mode, we generate a new encryption key
	{
		// Generate the key that will be used for the encryption
		key = fa_keygen(FA_KEYCHARSET, FA_AES_ENCRYPT_KEY_LEN);
		if (!key) return NULL;

		memcpy(g_stockhlm_header.encryption_key, key, FA_AES_ENCRYPT_KEY_LEN);

		if (g_modes & FA_VERBOSE)
			printf("Generated random key => " FA_YELLOW_COLOR "%s\n", key);
	}

	return key;
}

/*
 * Encrypt .text section before injection as we want the final output file
 *  to have its main code obfuscated in case we are packing a virus
 */

int fa_process_mapped_data(void)
{
	char *key = get_encryption_key();
	if (!key) return 1;

	if (g_modes & FA_VERBOSE)
		printf("\n > STARTING ENCRYPTION...\n\n");

	bool		reverse = g_modes & FA_REVERSE;
	// Only skip encrypting the custom header in reverse mode
	const char	*data = reverse ?
		g_mapped_data + FA_NEW_HEADER_SIZE : g_mapped_data;

	/*
	 * Encrypt the .text section before inserting the parasite code.
	 * The section will be decrypted by the latter during execution.
	 */

	xor_with_additive_cipher(
		key,									// The randomly generated encryption key
		FA_AES_ENCRYPT_KEY_LEN,					// The key width
		data,		// The file data (starting after the header)
		g_stockhlm_header.original_filesize,	// The original file size
		reverse								 	// Encryption/decryption mode
	);
	printf("loaded mapped: %s\n",g_mapped_data+FA_NEW_HEADER_SIZE);

	printf("encrypted size: %ld\n", g_stockhlm_header.original_filesize);

	if (g_modes & FA_VERBOSE)
		printf(FA_GREEN_COLOR "\nDone!\n\n" FA_RESET_COLOR);

	if (!reverse) {
		free(key);
		key = NULL;
	}

	return 0;
}
