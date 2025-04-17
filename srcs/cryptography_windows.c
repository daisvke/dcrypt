#include "dcrypt.h"

#ifdef _WIN32

# include <windows.h>
# include <wincrypt.h>
# include <stdio.h>

void print_hex(const char *label, BYTE *data, DWORD len) {
    printf("%s: ", label);
    for (DWORD i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

HCRYPTKEY generate_encryption_key(void)
{
	HCRYPTKEY	hKey = 0;
	DWORD		mode = CRYPT_MODE_CBC; // Set AES key to use CBC mode

	// Acquire a crypto context
	if (!CryptAcquireContext(&win_env.hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) ||
		!CryptGenKey(win_env.hProv, CALG_AES_128, CRYPT_EXPORTABLE, &hKey) ||
		!CryptSetKeyParam(hKey, KP_MODE, (BYTE *)&mode, 0))
	{
		if (win_env.hProv)
			CryptReleaseContext(win_env.hProv, 0);
	}

	return hKey;
}

int aes_encrypt_data(
    unsigned char       *data,
    DWORD				data_len,
    HCRYPTKEY			key,
    unsigned char       *iv
) {
    // Set Initialization Vector
    if (!CryptSetKeyParam(key, KP_IV, iv, 0)) return -1;

    data_len = data_len + 1;  // Include null terminator

    // Padding to make space for encryption (CBC needs padding)
    DWORD	buf_len = data_len + DC_AES_BLOCK_SIZE; // Ensure buffer is large enough
    BYTE	*buffer = malloc(buf_len);
    memcpy(buffer, data, data_len);

    // Encrypt
    if (!CryptEncrypt(key, 0, TRUE, 0, buffer, &data_len, buf_len)) {
        printf("CryptEncrypt failed: %lu\n", GetLastError());
        free(buffer);
        CryptDestroyKey(key);
        CryptReleaseContext(win_env.hProv, 0);
        return -1;
    }

    print_hex("Encrypted", buffer, data_len);

	return buf_len;
}

// Function to handle AES-128 CBC decryption
int aes_decrypt_data(
    unsigned char       *data,
    DWORD				data_len,
    HCRYPTKEY			key,
    unsigned char       *iv
) {
    data_len = data_len + 1;  // Include null terminator

    // Padding to make space for encryption (CBC needs padding)
    DWORD	buf_len = data_len + DC_AES_BLOCK_SIZE; // Ensure buffer is large enough
    BYTE	*buffer = malloc(buf_len);
    memcpy(buffer, data, data_len);

	if (!CryptSetKeyParam(key, KP_IV, iv, 0)) return -1;

    if (!CryptDecrypt(key, 0, TRUE, 0, buffer, &data_len)) {
        printf("CryptDecrypt failed: %lu\n", GetLastError());
		return -1;
    } else {
        printf("Decrypted: %s\n", buffer);
    }

    // Cleanup
    CryptDestroyKey(key); 						// Key securely destroyed
	if (win_env.hProv)
    	CryptReleaseContext(win_env.hProv, 0);	// Free context
    free(buffer);

	return buf_len;
}

#endif