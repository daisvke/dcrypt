#include "dcrypt.h"

#ifdef _WIN32
# include <windows.h>

void* map_file(const char* filename) {
    win_env.hFile = CreateFile(
        filename, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, // Allow other processes to read/write too
        NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );

    if (win_env.hFile == INVALID_HANDLE_VALUE) return NULL;

    // Create a handle for the mapped file
    win_env.hMap = CreateFileMapping(win_env.hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (!win_env.hMap) {
        CloseHandle(win_env.hFile);
        return NULL;
    }

    void* data = MapViewOfFile(win_env.hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!data) {
        CloseHandle(win_env.hMap);
        CloseHandle(win_env.hFile);
        return NULL;
    }

    return data;
}

void unmap_file(void* data) {
    if (data && !UnmapViewOfFile(data)) {
        return;
    }
    if (win_env.hMap && !CloseHandle(win_env.hMap)) {
        return;
    }
    if (win_env.hFile && !CloseHandle(win_env.hFile)) {
        return;
    }
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

#endif

// Verify that the file header's magic number is ours
bool is_magic_nbr_correct(const unsigned char *data)
{
    return strncmp((const char *)data, DC_SIGNATURE, DC_MAGICNBR_SIZE) == 0;
}

int map_file_into_memory(t_env *env, const char *filename)
{
    // Open the target binary file
    int fd = open(
        filename,
        #ifdef _WIN32
        O_RDONLY | O_BINARY
        # else
        O_RDONLY
        #endif
    );
    if (fd < 0) return DC_ERROR;
    
    // Determine the file size by moving the cursor till the end
    off_t res = lseek(fd, 0, SEEK_END);
    if (res < 0) {
        close(fd);
        return DC_ERROR;
    }

    // Put back the cursor at the beginning of the file
    if (lseek(fd, 0, SEEK_SET) < 0) {
        close(fd);
        return DC_ERROR;
    }

    env->mapped_data = map_file(filename);
    if (!env->mapped_data) {
        #ifdef _WIN32
		if (env->modes & DC_VERBOSE)
            printf("File mapping failed: %lu\n", GetLastError());
        #endif
        close(fd);
        return DC_ERROR;
    }

    if (env->modes & DC_REVERSE)
    {
        if (!is_magic_nbr_correct(env->mapped_data)) {
            if (env->modes & DC_VERBOSE)
                fprintf(
                    stderr,
                    FMT_ERROR " Signature not found in the file header, abort decryption...\n"
                );
            close(fd);
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
            close(fd);
            return DC_ERROR;
        }
        env->dcrypt_header.original_filesize = res;
    }

    close(fd);
    return DC_SUCCESS;
}

bool write_encrypted_data_to_file(t_env *env, const char *target_path)
{
    char    new_target_path[1024]; // Create a buffer to hold the full path

    // Use snprintf() to safely concatenate the strings and get the file path
    //  with our custom extension
    snprintf(new_target_path, sizeof(new_target_path), "%s.%s", target_path, DC_DCRYPT_EXT);

    #ifdef _WIN32
    DWORD   bytes_written;
    HANDLE  hFile = CreateFile(
        new_target_path,
        GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
        return DC_ERROR;

    // Write the header
    if (!WriteFile(hFile, &env->dcrypt_header, DC_DCRYPT_HEADER_SIZE, &bytes_written, NULL) ||
        // Write the encrypted data
        !WriteFile(hFile, env->encrypted_data, env->encrypted_filesize, &bytes_written, NULL))
    {
        CloseHandle(hFile);
        return DC_ERROR;
    }

    CloseHandle(hFile);

    #else
    int     outfilefd;

    outfilefd = open(new_target_path, O_CREAT | O_RDWR | O_TRUNC, 0755);        

    // Check if open() has failed
    if (outfilefd == 1) return DC_ERROR;

    // Write the custom header first to the outfile
    if (write(outfilefd, &env->dcrypt_header, DC_DCRYPT_HEADER_SIZE) < 0 ||
        // Then write the processed data to the outfile
        write(outfilefd, env->encrypted_data, env->encrypted_filesize) < 0)
    {
        close(outfilefd);
        return DC_ERROR;
    }

    close(outfilefd);
    #endif

    return DC_SUCCESS;
}

bool write_decrypted_data_to_file(t_env *env, char *target_path)
{
    int     outfilefd;

    // We remove our custom extension by terminating the filename earlier.
    target_path[strlen(target_path) - DC_DCRYPT_EXT_SIZE] = '\0';

    /*
     * - 0755: rwx for owner, rx for group and others
     * - O_BINARY to avoid new line translations
     */

    outfilefd = open(
        target_path,
        #ifdef _WIN32
        O_CREAT | O_RDWR | O_TRUNC | O_BINARY,
        # else
        O_CREAT | O_RDWR | O_TRUNC,
        #endif
        0755
    );

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
    char    temp_path[strlen(target_path) + 1];
    // Copy the original string to the temporary variable
    if (!strcpy(temp_path, target_path) || !target_path) return DC_ERROR;

    if (env->modes & DC_REVERSE)
    {
        if (write_decrypted_data_to_file(env, (char *)target_path) == DC_ERROR) {
            // Unmap the data from the memory
            #ifdef _WIN32
            unmap_file(env->mapped_data);
            if (env->modes & DC_VERBOSE)
                printf("CreateFile failed: %lu\n", GetLastError());
            # else
            munmap(env->mapped_data, env->dcrypt_header.original_filesize);
            #endif

            return DC_ERROR;
        }

        // Unmap the data from the memory
        #ifdef _WIN32
        unmap_file(env->mapped_data);
        # else
        munmap(env->mapped_data, env->encrypted_filesize + DC_DCRYPT_HEADER_SIZE);
        #endif
    }
    else
    {
        if (write_encrypted_data_to_file(env, (char *)target_path) == DC_ERROR) {
            // Unmap the data from the memory
            #ifdef _WIN32
            unmap_file(env->mapped_data);
            if (env->modes & DC_VERBOSE)
                printf("CreateFile failed: %lu\n", GetLastError());
            # else
            munmap(env->mapped_data, env->dcrypt_header.original_filesize);
            #endif

            return DC_ERROR;
        }

        // Unmap the data from the memory
        #ifdef _WIN32
        unmap_file(env->mapped_data);
        # else
        munmap(env->mapped_data, env->dcrypt_header.original_filesize);
        #endif
    }

    // Remove the old file from the system
    #ifdef _WIN32
    if (!DeleteFile(temp_path)) {
        DWORD error = GetLastError();
        if (env->modes & DC_VERBOSE)
            printf("Error deleting file: %lu\n", error);
        return DC_ERROR;
    }
    # else
    if (remove(temp_path) != 0) return DC_ERROR;
    #endif

    return DC_SUCCESS;
}
