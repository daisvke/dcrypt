#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>

#define AES_BLOCK_SIZE 16

void print_hex(const char *label, BYTE *data, DWORD len) {
    printf("%s: ", label);
    for (DWORD i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main() {
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    BYTE iv[AES_BLOCK_SIZE] = {0}; // Initialization vector (can be random)
    DWORD ivLen = AES_BLOCK_SIZE;

    // Acquire a crypto context
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        printf("CryptAcquireContext failed: %lu\n", GetLastError());
        return 1;
    }

    // Generate AES-128 key
    if (!CryptGenKey(hProv, CALG_AES_128, CRYPT_EXPORTABLE, &hKey)) {
        printf("CryptGenKey failed: %lu\n", GetLastError());
        CryptReleaseContext(hProv, 0);
        return 1;
    }

    // Set AES key to use CBC mode
    DWORD mode = CRYPT_MODE_CBC;
    if (!CryptSetKeyParam(hKey, KP_MODE, (BYTE *)&mode, 0)) {
        printf("CryptSetKeyParam (mode) failed: %lu\n", GetLastError());
    }

    // Set Initialization Vector
    if (!CryptSetKeyParam(hKey, KP_IV, iv, 0)) {
        printf("CryptSetKeyParam (IV) failed: %lu\n", GetLastError());
    }

    // Data to encrypt
    BYTE data[64] = "Sensitive data goes here. Needs to be encrypted securely!";
    DWORD dataLen = strlen((char *)data) + 1;  // Include null terminator

    // Padding to make space for encryption (CBC needs padding)
    DWORD bufLen = dataLen + AES_BLOCK_SIZE; // Ensure buffer is large enough
    BYTE *buffer = malloc(bufLen);
    memcpy(buffer, data, dataLen);

    // Encrypt
    if (!CryptEncrypt(hKey, 0, TRUE, 0, buffer, &dataLen, bufLen)) {
        printf("CryptEncrypt failed: %lu\n", GetLastError());
        free(buffer);
        CryptDestroyKey(hKey);
        CryptReleaseContext(hProv, 0);
        return 1;
    }

    print_hex("Encrypted", buffer, dataLen);

    // --- Optional: Decrypt ---
    if (!CryptSetKeyParam(hKey, KP_IV, iv, 0)) {
        printf("CryptSetKeyParam (reset IV for decryption) failed.\n");
    }

    if (!CryptDecrypt(hKey, 0, TRUE, 0, buffer, &dataLen)) {
        printf("CryptDecrypt failed: %lu\n", GetLastError());
    } else {
        printf("Decrypted: %s\n", buffer);
    }

    // Cleanup
    CryptDestroyKey(hKey);           // Key securely destroyed
    CryptReleaseContext(hProv, 0);   // Free context
    free(buffer);

    return 0;
}
