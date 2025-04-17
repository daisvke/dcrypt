#include "dcrypt.h"

#ifdef _WIN32
# include <windows.h>

void* map_file(const char* filename) {
    HANDLE hFile = CreateFileA(
        filename, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    // DWORD fileSize = GetFileSize(hFile, NULL);
    // if (size_out) *size_out = (size_t)fileSize;

    HANDLE hMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (!hMap) {
        CloseHandle(hFile);
        return NULL;
    }

    void* data = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (!data) {
        CloseHandle(hMap);
        CloseHandle(hFile);
        return NULL;
    }

    // Hold the data needed for unmaping the file data 
    win_env.hFile_out = hFile;
    win_env.hMap_out = hMap;

    CloseHandle(hMap);
    CloseHandle(hFile);

    return data;
}

void unmap_file(void* addr, HANDLE hMap, HANDLE hFile) {
    UnmapViewOfFile(addr);
    CloseHandle(hMap);
    CloseHandle(hFile);
}

# else

# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>

void* map_file(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return NULL;

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return NULL;
    }

    size_t size = st.st_size;

    /* Map the file into memory
        - PROT_READ: read-only access
        - PROT_WRITE: write-only access
            We use both READ and WRITE since we are going to encrypt the
            mapped region directly.
        - MAP_PRIVATE: creates a private copy of the mapped data, so any
        modifications made to the mapped memory will not be visible
        to other processes mapping the same file
    */

    void* data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return NULL;
    }
    close(fd); // No need to keep the fd since the file is mapped

    return data;
}

void unmap_file(void* addr, size_t size, int fd) {
    munmap(addr, size);
    close(fd);
}
#endif

// Verify that the file header's magic number is ours
bool is_magic_nbr_correct(const unsigned char *data)
{
    return strncmp((const char *)data, DC_SIGNATURE, DC_MAGICNBR_SIZE) == 0;
}

int map_file_into_memory(t_env *env, const char *filename)
{
    // Open the target binary file
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return DC_ERROR;
    
    // Determine the file size by moving the cursor till the end
    off_t res = lseek(fd, 0, SEEK_END);
    // Check that lseek didn't fail and not returning > int max
    if (res < 0 || res > 2147483647) return DC_ERROR;

    // Put back the cursor at the beginning of the file
    if (lseek(fd, 0, SEEK_SET) < 0) return DC_ERROR;

    env->mapped_data = map_file(filename);
    if (!env->mapped_data) return DC_ERROR;

    if (env->modes & DC_REVERSE)
    {
        if (!is_magic_nbr_correct(env->mapped_data)) {
            if (env->modes & DC_VERBOSE)
                fprintf(
                    stderr,
                    FMT_ERROR " Signature not found in the file header, abort decryption...\n"
                );
            return DC_ERROR;
        }

        // Copy the custom header of the encrypted file to the env->global variable
        memcpy(&env->dcrypt_header, env->mapped_data, DC_DCRYPT_HEADER_SIZE);
        // Save the encrypted file size without the added custom header size
        env->encrypted_filesize = res - DC_DCRYPT_HEADER_SIZE;
    } else {
        if (is_magic_nbr_correct(env->mapped_data)) {
            if (env->modes & DC_VERBOSE)
                fprintf(
                    stderr,
                    FMT_ERROR " Signature found in the file header, abort encryption...\n"
                );
            return DC_ERROR;
        }
        env->dcrypt_header.original_filesize = res;
    }

    // // Abort if the current target already contains our signature
    // if (search_binary((char *)mapped_data, dcrypt_header.original_filesize, DC_SIGNATURE))
    //     return 1;

    return DC_SUCCESS;
}

bool write_encrypted_data_to_file(t_env *env, const char *target_path)
{
    char    new_target_path[1024]; // Create a buffer to hold the full path
    int     outfilefd;

    // Use snprintf() to safely concatenate the strings and get the file path
    //  with our custom extension
    snprintf(
        new_target_path, sizeof(new_target_path),
        "%s.%s", target_path, DC_DCRYPT_EXT
    );

    outfilefd = open(new_target_path, O_CREAT | O_RDWR | O_TRUNC, 0755);        

    // Check if open() has failed
    if (outfilefd == 1) return DC_ERROR;

    // Write the custom header first to the outfile
    ssize_t bytes_written = write(outfilefd, &env->dcrypt_header, DC_DCRYPT_HEADER_SIZE);
    if (bytes_written < 0) return DC_ERROR;

    // Then write the processed data to the outfile
    bytes_written = write(outfilefd, env->mapped_data, env->encrypted_filesize);
    if (bytes_written < 0) return DC_ERROR;

    close(outfilefd);
    return DC_SUCCESS;
}

bool write_decrypted_data_to_file(t_env *env, char *target_path)
{
    int     outfilefd;

    // We remove our custom extension by terminating the filename earlier.
    target_path[strlen(target_path) - DC_DCRYPT_EXT_SIZE] = '\0';

    // 0755: rwx for owner, rx for group and others
    outfilefd = open(target_path, O_CREAT | O_RDWR | O_TRUNC, 0755);

    // Check if open() has failed
    if (outfilefd == 1) return DC_ERROR;

    // Write the processed data to the outfile
    ssize_t bytes_written = write(
        outfilefd,
        env->mapped_data + DC_DCRYPT_HEADER_SIZE, // Don't keep the custom header
        env->dcrypt_header.original_filesize
    );
    if (bytes_written < 0) return DC_ERROR;

    close(outfilefd);
    return DC_SUCCESS;
}

// Write the processed file data back to a new file
int write_processed_data_to_file(t_env *env, const char *target_path)
{
    char    temp_path[strlen(target_path) - 1];
    // Copy the original string to the temporary variable
    strcpy(temp_path, target_path);

    if (env->modes & DC_REVERSE)
    {
        if (write_decrypted_data_to_file(env, (char *)target_path) == DC_ERROR)
            return DC_ERROR;
        munmap(env->mapped_data, env->encrypted_filesize + DC_DCRYPT_HEADER_SIZE);
    } else
    {
        if (write_encrypted_data_to_file(env, target_path) == DC_ERROR)
            return DC_ERROR;
        munmap(env->mapped_data, env->dcrypt_header.original_filesize);
    }

    // Remove the old file from the system
    if (remove(temp_path) != 0) return DC_ERROR;

    return DC_SUCCESS;
}
