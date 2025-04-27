#include "dcrypt.h"

void print_hex(const char *label, const unsigned char *data, size_t out_len) {
    if (!data || !out_len) return;

    printf("%s: ", label);
    for (size_t i = 0; i < out_len; ++i) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

void hexstr_to_bytes(const unsigned char *hexstr, unsigned char *out, size_t out_len) {
    if (hexstr && out_len)
        for (size_t i = 0; i < out_len; ++i)
            sscanf((char *)hexstr + 2 * i, "%2hhx", &out[i]);
}
