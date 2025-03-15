#include "stockholm.h"

/* Encrypt .text section before injection as we want the final output file
 *  to have its main code obfuscated in case we are packing a virus
 */
int fa_process_mapped_data(void)
{
	if (g_modes & FA_VERBOSE)
		printf("\n > STARTING ENCRYPTION...\n\n");

	// Generate the key that will be used for the encryption
	char *key = fa_keygen(FA_KEYCHARSET, FA_AES_ENCRYPT_KEY_LEN);
	if (!key)
		return 1;

	memcpy(g_stockhlm_header.encryption_key, key, FA_AES_ENCRYPT_KEY_LEN);

	if (g_modes & FA_VERBOSE)
		printf("Generated random key => " FA_YELLOW_COLOR "%s\n", key);

	bool reverse = g_modes & FA_REVERSE;

	/* Encrypt the .text section before inserting the parasite code.
	 * The section will be decrypted by the latter during execution.
	 */
	xor_with_additive_cipher(
		key,					// The randomly generated encryption key
		FA_AES_ENCRYPT_KEY_LEN, // The key width
		g_mapped_data,
		g_stockhlm_header.original_filesize, // The .text section size
		reverse								 // Encryption/decryption mode
	);

	printf("encrypted size: %ld,orignial:l %ld\n", g_stockhlm_header.original_filesize, g_stockhlm_header.original_filesize);

	if (g_modes & FA_VERBOSE)
		printf(FA_GREEN_COLOR "Done!\n\n" FA_RESET_COLOR);

	return 0;
}
