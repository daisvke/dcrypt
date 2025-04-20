#include "dcrypt.h"

#ifdef _WIN32

# include <wincrypt.h>

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

HCRYPTKEY import_raw_aes_key(t_env *env, HCRYPTPROV hProv, BYTE *raw_key, DWORD key_len)
{
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
		&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT
	)) return 0;

	// Import the raw key
    if (!CryptImportKey(hProv, (BYTE *)&blob, blob_len, 0, 0, &hKey)) {
		if (env->modes & DC_VERBOSE)
            printf("CryptImportKey (PLAINTEXTKEYBLOB) failed: %lu\n", GetLastError());
		if (hProv)
			CryptReleaseContext(hProv, 0);
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

char *get_aes_key_as_ascii(HCRYPTKEY hKey) {
    DWORD blobLen = 0;
    if (!CryptExportKey(hKey, 0, PLAINTEXTKEYBLOB, 0, NULL, &blobLen)) {
        printf("CryptExportKey (get size) failed: %lu\n", GetLastError());
        return NULL;
    }

    BYTE *blob = malloc(blobLen);
    if (!blob) return NULL;

    if (!CryptExportKey(hKey, 0, PLAINTEXTKEYBLOB, 0, blob, &blobLen)) {
        printf("CryptExportKey failed: %lu\n", GetLastError());
        free(blob);
        return NULL;
    }

    // Key starts after header + length
    BYTE *keyData = blob + sizeof(BLOBHEADER) + sizeof(DWORD);
    DWORD keyLen = *(DWORD *)(blob + sizeof(BLOBHEADER));

    // Allocate string buffer (add 1 for null terminator)
    char *asciiKey = malloc(keyLen + 1);
    if (!asciiKey) {
        free(blob);
        return NULL;
    }

    memcpy(asciiKey, keyData, keyLen);
    asciiKey[keyLen] = '\0'; // null-terminate

    free(blob);
    return asciiKey;
}
char *get_aes_key_as_hex_string(HCRYPTKEY hKey) {
    DWORD blobLen = 0;
    if (!CryptExportKey(hKey, 0, PLAINTEXTKEYBLOB, 0, NULL, &blobLen)) {
        printf("CryptExportKey (get size) failed: %lu\n", GetLastError());
        return NULL;
    }

    BYTE *blob = malloc(blobLen);
    if (!blob) return NULL;

    if (!CryptExportKey(hKey, 0, PLAINTEXTKEYBLOB, 0, blob, &blobLen)) {
        printf("CryptExportKey failed: %lu\n", GetLastError());
        free(blob);
        return NULL;
    }

    // Get pointer to key bytes (after BLOBHEADER + DWORD)
    BYTE *keyData = blob + sizeof(BLOBHEADER) + sizeof(DWORD);
    DWORD keyLen = *(DWORD *)(blob + sizeof(BLOBHEADER));

    // Each byte -> 2 chars, +1 for null terminator
    char *hexStr = malloc(keyLen * 2 + 1);
    if (!hexStr) {
        free(blob);
        return NULL;
    }

    for (DWORD i = 0; i < keyLen; i++)
        sprintf(&hexStr[i * 2], "%02X", keyData[i]);

    hexStr[keyLen * 2] = '\0'; // null-terminate
    free(blob);
    return hexStr;
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
    char *asciiKey = get_aes_key_as_ascii(hKey);
    char *hexKey = get_aes_key_as_hex_string(hKey);
    if (asciiKey) {
        printf("AES Key (ASCII): %s\n", asciiKey);
        printf("HEX Key (ASCII): %s\n", hexKey);
        free(hexKey);
        free(asciiKey);
    }
    
    
    return hKey;
}

int aes_encrypt_data(
    unsigned char       *data,
    unsigned char       **encrypted_data,
    DWORD				data_len,
    HCRYPTKEY			key,
    unsigned char       *iv
) {
    // Set Initialization Vector
    if (!CryptSetKeyParam(key, KP_IV, iv, 0)) return -1;

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

    // Overwrite original data with encrypted content
    *encrypted_data = buffer;

    print_hex("Encrypted", *encrypted_data, data_len);

    // Cleanup
    CryptDestroyKey(key); 						// Key securely destroyed
	if (win_env.hProv)
    	CryptReleaseContext(win_env.hProv, 0);	// Free context

	return data_len;
}

// Function to handle AES-128 CBC decryption
int aes_decrypt_data(
    unsigned char       *data,
    DWORD				data_len,
    HCRYPTKEY			key,
    unsigned char       *iv
) {
    if (!key) return -1;

    // Set cipher mode
    DWORD   mode = CRYPT_MODE_CBC;
	if (!CryptSetKeyParam(key, KP_MODE, (BYTE *)&mode, 0) ||
		!CryptSetKeyParam(key, KP_IV, iv, 0))
        return -1;

    if (!CryptDecrypt(key, 0, TRUE, 0, data, &data_len)) {
        printf("CryptDecrypt failed: %lu\n", GetLastError());
		return -1;
    } else {
        printf("Decrypted: %s\n", data);
    }

    // Cleanup
    CryptDestroyKey(key); 						// Key securely destroyed
	if (win_env.hProv)
    	CryptReleaseContext(win_env.hProv, 0);	// Free context

	return data_len;
}

#endif