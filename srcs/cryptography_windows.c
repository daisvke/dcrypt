#include "dcrypt.h"

#ifdef _WIN32

# include <windows.h>
# include <wincrypt.h>
# include <stdio.h>

/*
 * Import raw AES key to to get a key handle linked to the key.
 *
 * Raw AES key cannot be used directly for encryption due to the way
 * CryptoAPI manages cryptographic keys.
 * CryptoAPI requires keys to be imported into a secure key container,
 * which handles key storage, access control, and cryptographic operations securely.
 *
 * Using the raw key directly bypasses these security mechanisms 
 * and can lead to vulnerabilities, such as exposing the key 
 * in memory or logs, which compromises the security of the 
 * encryption process.
 */

HCRYPTKEY import_raw_aes_key(HCRYPTPROV hProv, BYTE *raw_key, DWORD key_len) {
    struct {
        BLOBHEADER	hdr;
        DWORD		key_size;
        BYTE		key[16]; // max 16 bytes for AES-128
    } blob;

    blob.hdr.bType		= PLAINTEXTKEYBLOB;
    blob.hdr.bVersion	= CUR_BLOB_VERSION;
    blob.hdr.reserved	= 0;
    blob.hdr.aiKeyAlg	= CALG_AES_128;
    blob.key_size		= key_len;
    memcpy(blob.key, raw_key, key_len);

	DWORD		blob_len = sizeof(BLOBHEADER) + sizeof(DWORD) + key_len;
    HCRYPTKEY	hKey;

	// Acquire a crypto context
	if (!CryptAcquireContext(
		&win_env.hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT
	))
		if (win_env.hProv)
			CryptReleaseContext(win_env.hProv, 0);
		return -1;

	// Import the raw key
    if (!CryptImportKey(hProv, (BYTE *)&blob, blob_len, 0, 0, &hKey)) {
        printf("CryptImportKey (PLAINTEXTKEYBLOB) failed: %lu\n", GetLastError());
        return 0;
    }

    return hKey;
}

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
	if (!CryptAcquireContext(
		&win_env.hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT
		) ||
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

		// Acquire a crypto context
	if (!CryptAcquireContext(
		&win_env.hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) ||
		!CryptSetKeyParam(key, KP_IV, iv, 0)) return -1;

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