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

HCRYPTKEY import_raw_aes_key(t_env *env, const unsigned char *key, DWORD key_len)
{
    struct {
        BLOBHEADER	hdr;
        DWORD		key_size;
        BYTE		key[key_len];
    } blob;

    blob.hdr.bType		= PLAINTEXTKEYBLOB;
    blob.hdr.bVersion	= CUR_BLOB_VERSION;
    blob.hdr.reserved	= 0;
    blob.hdr.aiKeyAlg	= CALG_AES_128;
    blob.key_size		= key_len;
    
    memcpy(blob.key, key, key_len);

	DWORD		blob_len = sizeof(BLOBHEADER) + sizeof(DWORD) + key_len;
    HCRYPTKEY	hKey;

	// Acquire a crypto context
	if (!CryptAcquireContext(
		&win_env.hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT
	)) return DC_ERROR;

	// Import the raw key
    if (!CryptImportKey(win_env.hProv, (BYTE *)&blob, blob_len, 0, 0, &hKey)) {
		if (env->modes & DC_VERBOSE)
            printf("CryptImportKey (PLAINTEXTKEYBLOB) failed: %lu\n", GetLastError());
		if (win_env.hProv)
			CryptReleaseContext(win_env.hProv, 0);
        return DC_ERROR;
    }

    // Save the key handle: it contains the link to the actual key
    win_env.hKey = hKey;

    return DC_SUCCESS;
}

int save_aes_key_as_bytes(t_env *env, HCRYPTKEY hKey) {
    DWORD blob_len = 0;
    if (!CryptExportKey(hKey, 0, PLAINTEXTKEYBLOB, 0, NULL, &blob_len)) {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR
                "CryptExportKey (get size) failed: %lu\n",
                GetLastError()
            );
        return DC_ERROR;
    }

    BYTE *blob = malloc(blob_len);
    if (!blob) {
        if (env->modes & DC_VERBOSE)
            fprintf(stderr, FMT_ERROR "malloc failed: %lu\n", GetLastError());
        return DC_ERROR;
    }

    if (!CryptExportKey(hKey, 0, PLAINTEXTKEYBLOB, 0, blob, &blob_len)) {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR "CryptExportKey failed: %lu\n",
                GetLastError()
            );
        dc_free((void **)&blob);
        return DC_ERROR;
    }

    // Get pointer to key bytes (after BLOBHEADER + DWORD)
    BYTE    *key_data = blob + sizeof(BLOBHEADER) + sizeof(DWORD);
    DWORD   key_len = *(DWORD *)(blob + sizeof(BLOBHEADER));

    env->encryption_key = malloc(key_len + 1);
    if (!env->encryption_key) {
        if (env->modes & DC_VERBOSE)
            fprintf(stderr, FMT_ERROR "malloc failed: %lu\n", GetLastError());
        dc_free((void **)&blob);
        return DC_ERROR;
    }

    // Copy key bytes to the global variable for later use
    memcpy(env->encryption_key, key_data, key_len);
    env->encryption_key[key_len] = '\0';

    dc_free((void **)&blob);
    return DC_SUCCESS;
}

HCRYPTKEY generate_encryption_key(t_env *env)
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

    // Print the AES key (this will be the only way for the user to get it)
    if (save_aes_key_as_bytes(env, hKey) == DC_ERROR || !env->encryption_key)
    {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR "Failed to get string formatted AES key.\n"
            );
        return 0;
    }
    if (env->encryption_key)
        print_hex(FMT_DONE "Generated random key", env->encryption_key, DC_AES_KEY_SIZE);
    
    return hKey;
}

int aes_encrypt_data(
    t_env               *env,
    unsigned char       *data,
    unsigned char       **encrypted_data,
    DWORD				data_len,
    HCRYPTKEY			key,
    unsigned char       *iv
) {
    DWORD   key_len = 0;
    DWORD   size = sizeof(DWORD);
    if (!CryptGetKeyParam(key, KP_KEYLEN, (BYTE *)&key_len, &size, 0)) {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR " Key handle is invalid before encryption: %lu\n",
                GetLastError()
            );
        return DC_CRYPT_ERROR;
    }

    if (env->modes & DC_VERBOSE)
        print_hex(FMT_INFO " IV used for encryption", iv, 16);

    // Set Initialization Vector
    if (!CryptSetKeyParam(key, KP_IV, iv, 0)) {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR " Failed to set cipher mode: %lu\n",
                GetLastError()
            );
        return DC_CRYPT_ERROR;
    }

    // Padding to make space for encryption (CBC needs padding)
    DWORD	buf_len = data_len + DC_AES_BLOCK_SIZE; // Ensure buffer is large enough
    BYTE    *buffer = calloc(1, buf_len);           // Allocate and zero-initialize
    if (!buffer) {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR " Failed to allocate for the buffer: %lu\n",
                GetLastError()
            );
        return DC_CRYPT_ERROR;
    }
    // Copy the data to encrypt to the padded buffer
    memcpy(buffer, data, data_len);    

    // Encrypt with padding (TRUE)
    if (!CryptEncrypt(key, 0, TRUE, 0, buffer, &data_len, buf_len)) {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR " CryptEncrypt failed: %lu\n",
                GetLastError()
            );
        // Clean up
        dc_free((void **)&buffer);
        // TODO del
        // CryptDestroyKey(key);
        // CryptReleaseContext(win_env.hProv, 0);
        return DC_CRYPT_ERROR;
    }

    // Overwrite original data with encrypted content
    *encrypted_data = buffer;

    // For testing: print the encrypted data
    // print_hex(FMT_DONE "Encrypted", *encrypted_data, data_len);

    // TODO del
    // Cleanup
    // CryptDestroyKey(key); 						// Key securely destroyed
	// if (win_env.hProv)
    // 	CryptReleaseContext(win_env.hProv, 0);	// Free context

	return data_len;
}

// Function to handle AESDC_CRYPT_ERROR28 CBC decryption
int aes_decrypt_data(
    t_env               *env,
    unsigned char       *data,
    DWORD				data_len,
    HCRYPTKEY			key,
    unsigned char       *iv
) {
    if (!key) {
        if (env->modes & DC_VERBOSE)
            fprintf(stderr, FMT_ERROR " Failed to import raw AES key\n");
        return DC_CRYPT_ERROR;
    }

	DWORD   key_len = 0;
    DWORD   size = sizeof(DWORD);

    if (!CryptGetKeyParam(key, KP_KEYLEN, (BYTE *)&key_len, &size, 0)) {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR " Key handle is invalid before decryption: %lu\n",
                GetLastError()
            );
        return DC_CRYPT_ERROR;
    }

    if (env->modes & DC_VERBOSE)
        print_hex(FMT_INFO " IV used for decryption", iv, 16);

    // Set cipher modes
    DWORD   mode = CRYPT_MODE_CBC;
	if (!CryptSetKeyParam(key, KP_MODE, (BYTE *)&mode, 0) ||
		!CryptSetKeyParam(key, KP_IV, iv, 0)) {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR " Failed to set cipher modes: %lu\n",
                GetLastError()
            );
        return DC_CRYPT_ERROR;
    }

    if (!CryptDecrypt(key, 0, TRUE, 0, data, &data_len)) {
        if (env->modes & DC_VERBOSE)
            fprintf(
                stderr,
                FMT_ERROR " CryptDecrypt failed: %lu\n",
                GetLastError()
            );
        return DC_CRYPT_ERROR;
    } else {
        // For testing: print the decrypted data
        // printf(FMT_DONE "Decrypted: %s\n", data);
    }

    // // Cleanup
    // CryptDestroyKey(key); 						// Key securely destroyed
	// if (win_env.hProv)
    // 	CryptReleaseContext(win_env.hProv, 0);	// Free context

	return data_len;
}

#endif