#include "stockholm.h"

int pc_is_process_running(const char *process_name)
{
    DIR *dir;             // Directory stream type
    struct dirent *entry; // Directory entry type

    // Open the /proc directory
    dir = opendir("/proc");
    // Quit silently in case of error
    if (!dir)
        exit(0);

    // Iterate through the entries in /proc
    while ((entry = readdir(dir)) != NULL)
    {
        // Check if the entry is a directory and is a number (=PID)
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0)
        {
            // Get the cmdline file path from the directory
            char cmdline_path[270];
            snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%s/cmdline", entry->d_name);

            // Open the cmdline file to read the command line of the process
            FILE *cmdline_file = fopen(cmdline_path, "r");
            if (cmdline_file)
            {
                char cmdline[256];
                fgets(cmdline, sizeof(cmdline), cmdline_file);
                fclose(cmdline_file);

                // For testing
                // printf("found cmdline: %s\n", cmdline);

                // Check if the process name matches
                if (strstr(cmdline, process_name))
                {
                    closedir(dir);
                    return 1; // Process is running
                }
            }
        }
    }

    closedir(dir); // Close "/proc" directory
    return 0;      // Process is not running
}

int pc_is_debugger_attached(void)
{
    FILE *status_file = fopen("/proc/self/status", "r");
    if (!status_file)
    {
        perror("fopen");
        return 0; // Assume no debugger if we can't open the file
    }

    char line[256];
    while (fgets(line, sizeof(line), status_file))
    {
        if (strncmp(line, "TracerPid:", 10) == 0)
        {
            int tracer_pid = atoi(line + 10);
            fclose(status_file);
            return tracer_pid != 0; // If TracerPid is not 0, a debugger is attached
        }
    }

    fclose(status_file);
    return 0; // No debugger found
}
