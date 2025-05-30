#include "dcrypt.h"

void print_hex(const char *label, const unsigned char *data, size_t data_len)
{
    if (!data || !data_len) return;

    printf("%s: ", label);
    for (size_t i = 0; i < data_len; ++i) {
        printf(FMT_YELLOW "%02X" FMT_RESET, data[i]);
    }
    printf("\n");
}

void hexstr_to_bytes(const unsigned char *hexstr, unsigned char *out, size_t out_len)
{
    if (hexstr && out_len)
        for (size_t i = 0; i < out_len; ++i)
            sscanf((char *)hexstr + 2 * i, "%2hhx", &out[i]);
}

// Free the pointer if is not empty, set the original pointer
//  to NULL and return NULL
void    *dc_free(void **ptr)
{
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }

    return NULL;
}

// Free all allocated memory and destroy the key, then exit
void exit_gracefully(t_env *env, int exit_code)
{
    dc_free((void **)&env->encryption_key);

    #ifdef _WIN32
	if (win_env.hProv) {
    	CryptReleaseContext(win_env.hProv, 0);  // Free context
    }
    if (win_env.hKey) {
        CryptDestroyKey(win_env.hKey);          // Key securely destroyed
    }
    #endif

    exit(exit_code);
}