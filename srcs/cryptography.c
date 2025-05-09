#include "dcrypt.h"

#ifndef _WIN32
# include <openssl/aes.h>   // For AES key generation
# include <openssl/rand.h>
# include <openssl/evp.h>

/*
 * Perform AESDC_CRYPT_ERROR28 CBC encryption
 * -------------------------------
 *
 * Chaining:
 * --------
 * In CBC mode, each plaintext block is XORed with the previous ciphertext
 *  block before being encrypted.
 * This means that the encryption of each block depends on the previous
 *  block's ciphertext, which enhances security.
 * 
 * IV (Initialization Vector):
 * --------------------------
 * The IV is a random value used to ensure that identical plaintext blocks
 *  produce different ciphertexts.
 * This prevents patterns from appearing in the encrypted data.
 * 
 * Without IV (= NULL IV) the process will be faster, but less secure.
 * Identical plaintexts will produce identical ciphertexts.
 *
 * Padding:
 * -------
 * When using OpenSSL's EVP functions for encryption, the default padding
 *  scheme used is PKCS#7 padding.
 * 
 * How PKCS#7 Padding Works:
 *  If the plaintext data is not a multiple of the block size (16 bytes for
 *      AES), PKCS#7 padding adds bytes to the end of the plaintext.
 */

int aes_encrypt_data(
    t_env               *env,
    unsigned char       *data,
    unsigned char       **encrypted_data,
    size_t              data_len,
    const unsigned char *key,
    unsigned char       *iv
) {
    // A structure that holds the context for the encryption operation
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        if (env->modes & DC_VERBOSE)
            perror("Context creation error");
        return DC_CRYPT_ERROR;
    }

    // Initialize the encryption operation
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        if (env->modes & DC_VERBOSE)
            perror("Initialization error");
        return DC_CRYPT_ERROR;
    }

    // Allocate a buffer that's big enough for data + padding
    int buf_len = data_len + EVP_CIPHER_block_size(EVP_aes_128_cbc());
    unsigned char *buffer = malloc(buf_len);
    if (!buffer) {
        EVP_CIPHER_CTX_free(ctx);
        if (env->modes & DC_VERBOSE)
            perror("Failed to allocate memory");
        return DC_CRYPT_ERROR;
    }

    int out_len;
    // Perform the encryption of data in 16 bytes chunk
    if (EVP_EncryptUpdate(ctx, buffer, &out_len, data, data_len) != 1) {
        dc_free((void **)&buffer);
        EVP_CIPHER_CTX_free(ctx);
        if (env->modes & DC_VERBOSE)
            perror("Encryption error");
        return DC_CRYPT_ERROR;
    }

    int final_len;
    // Write any remaining encrypted data, adding paddings if needed
    if (EVP_EncryptFinal_ex(ctx, buffer + out_len, &final_len) != 1) {
        dc_free((void **)&buffer);
        EVP_CIPHER_CTX_free(ctx);
        if (env->modes & DC_VERBOSE)
            perror("Finalization error");
        return DC_CRYPT_ERROR;
    }

    *encrypted_data = buffer;
    EVP_CIPHER_CTX_free(ctx);

    // Return the total length of the encrypted data
    return out_len + final_len; 
}

// Function to handle AESDC_CRYPT_ERROR28 CBC decryption
int aes_decrypt_data(
    t_env               *env,
    unsigned char       *data,
    size_t              data_len,
    const unsigned char *key,
    unsigned char       *iv
) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        if (env->modes & DC_VERBOSE)
            perror("Context creation error");
        return DC_CRYPT_ERROR; // Context creation error
    }

    // Initialize the decryption operation
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        if (env->modes & DC_VERBOSE)
            perror("Initialization error");
        return DC_CRYPT_ERROR;
    }

    int out_len;
    if (EVP_DecryptUpdate(ctx, data, &out_len, data, data_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        if (env->modes & DC_VERBOSE)
            perror("Decryption error");
        return DC_CRYPT_ERROR;
    }

    int final_len;
    if (EVP_DecryptFinal_ex(ctx, data + out_len, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        if (env->modes & DC_VERBOSE)
            perror("Finalization error");
        return DC_CRYPT_ERROR;
    }

    EVP_CIPHER_CTX_free(ctx);
    return out_len + final_len; // Return the total length of the decrypted data
}

/*
 * Using /dev/urandom or /dev/random is generally more secure and
 * 	provides better randomness than using rand() with a time-based seed.
 *
 * Advantages of Using /dev/xrandom:
 * 
 * 	- Cryptographic Security: /dev/xrandom is designed to provide
 * 		cryptographically secure random numbers. It uses environmental
 * 		noise collected from device drivers and other sources to generate
 *		random numbers, making it much harder to predict the output.
 * 
 * 	- Non-Deterministic: Unlike rand(), which is a pseudo-random number
 *		generator (PRNG) that produces a deterministic sequence based on
 *		an initial seed, /dev/xrandom generates non-deterministic random
 *		numbers. This means that even if you read from it multiple times
 *		in quick succession, you will get different values.
 * 
 * 	- Higher Entropy: The randomness quality (entropy) of numbers generated
 *		from /dev/xrandom is significantly higher than that of rand().
 *	  This makes it suitable for applications that require high-quality
 *		randomness, such as cryptographic keys, secure tokens, and other
 *		security-sensitive operations.
 *
 *
 * /dev/urandom or /dev/random:
 * ----------------------------
 *
 * Blocking Behavior: /dev/random is a blocking source of random numbers.
 * This means that if there is not enough entropy (randomness) available
 * 	in the system, reading from /dev/random will block (i.e., wait) until
 * 	sufficient entropy is gathered.
 * This can lead to delays in applications that require random numbers.
 *
 * While it may not provide the same level of entropy as /dev/random,
 * 	/dev/urandom is generally considered secure enough for most applications,
 * 	including cryptographic purposes. This is why we will use it here.
 */

 unsigned char *generate_random_based_key(
	    t_env *env, const char *charset, size_t strength, bool blocking
	) {
	unsigned char *key = malloc((strength + 1) * sizeof(char));
	if (!key) return NULL;

	int charset_length = strlen(charset);

    int fd = open(
		blocking ? "/dev/urandom" : "/dev/random",
		O_RDONLY
		);
    if (fd < 0) return dc_free((void **)&key);

    unsigned char random_bytes[strength];
    if (read(fd, random_bytes, strength) != (int)strength) {
        close(fd);
        return dc_free((void **)&key);
    }
    if (close(fd) < 0 && (env->modes & DC_VERBOSE))
        perror("Failed to close the file");

    for (size_t i = 0; i < strength; ++i) {
        int random_index = random_bytes[i] % charset_length;
        key[i] = charset[random_index];
    }
    key[strength] = '\0'; // Null-terminate the string

	return key;
}

#endif

/*
 * Generates an encryption key using the current time as seed.
 *
 *  - charset: a set of characters that can be used for the key
 *  - strength: the width of the key
 *
 * Setting the seed for the random number generator used by rand()
 *      helps ensuring that the sequence of random numbers generated by rand()
 *      is different each time the program runs.
 *
 * The seed is set according to the current time, which makes it unique, but
 *      not perfectly:
 *
 *      - Using srand(time(NULL)) (like in the first function) seeds the random
 *              number generator with the current time in seconds.
 *      - This means that if you call srand(time(NULL)) multiple times within
 *              the same second, you will get the same sequence of random numbers
 *              because the seed value does not change.
 *      - Using clock_gettime(): This function (used in the second function) can
 *              provide nanosecond precision, depending on the clock used. This will
 *              improve the randomness of the key.
 * 
 * This explains why we will use this keygen only for generating the IV
 *  and not for the AES key.
 */

 unsigned char *generate_time_based_rand_key_nanosec(
    const char *charset, size_t strength
    ) {
    unsigned char *key = malloc((strength + 1) * sizeof(char));
    if (!key) return NULL;

    int     charset_length = strlen(charset);
    struct  timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts); // Get the current time

    // Seed with nanosecond precision
    srand(ts.tv_sec * 1000000000 + ts.tv_nsec);

    for (size_t i = 0; i < strength; ++i) {
        int random_index = rand() % charset_length;
        // Pick a random position from the charset
        key[i] = charset[random_index];
    }
    key[strength] = '\0'; // Null-terminate the string

    return key;
}

unsigned char *get_key(t_env *env)
{
	// In decryption mode, we use the IV saved in the file header
	if (env->modes & DC_REVERSE)
	{
		if (env->modes & DC_VERBOSE)
			printf(
                FMT_INFO
                " Using encryption key => " FMT_YELLOW "%s\n" FMT_RESET,
                env->decryption_key
            );
        return env->decryption_key;
	}
	else // In encryption mode, we generate a new encryption key
	{
        // If the key has already been generated, we stop here
        if (env->encryption_key) return env->encryption_key;
		// Generate the key that will be used for the encryption
        #ifdef _WIN32
		win_env.hKey = generate_encryption_key(env);
		if (!win_env.hKey) return NULL;
        static unsigned char array[2] = {"1"};
        return array; // We have to return an unsigned char *
        # else
		env->encryption_key = generate_random_based_key(
            env, DC_KEYCHARSET, DC_AES_KEY_SIZE, false
        );
		if (!env->encryption_key) return NULL;

        print_hex(FMT_DONE "Generated random key", env->encryption_key, DC_AES_KEY_SIZE);
        #endif

        return env->encryption_key;
	}
}
