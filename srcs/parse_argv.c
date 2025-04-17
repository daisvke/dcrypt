#include "dcrypt.h"

void print_help() {
    printf(" Usage: ./dcrypt [-h|-v|-s|-k <KEY>|-r <KEY>] \n\n"
           " Options:\n"
           "  -v, --version          Show version information\n"
           "  -s, --silent           Run in silent mode (non-verbose)\n"
           "  -k, --key <KEY>    	 Provide an encryption key\n"
           "  -r, --reverse <KEY>    Decrypt using the provided decryption key\n"
           "  -h, --help             Show this help message and exit\n");
}

void print_version()
{
	printf(
		DC_PROG_NAME " " DC_PROG_VERSION " | Copyright (c) " DC_PROG_AUTHOR "\n"
	);
	exit(EXIT_SUCCESS);
}

void check_arg_key(const char opt, bool verbose)
{
	// Check if the argument is provided and that the key has required length
	if ((!optarg || (strlen(optarg) != DC_AES_KEY_SIZE)))
	{
        if (verbose)
        {
            fprintf(
                stderr,
                FMT_ERROR
                " The -%c option require a 128 bits encryption key as an argument.\n",
                opt
            );
        }
		exit(EXIT_FAILURE);
	}
}

void parse_argv(t_env *env, int argc, char *argv[])
{
    int                 opt;
    const char          *short_opts = "shvr:k:"; // Moved 's' to the end
    const struct option long_opts[] = {
        { "help",    no_argument,       NULL, 'h' },
        { "version", no_argument,       NULL, 'v' },
        { "silent",  no_argument,       NULL, 's' },
        { "key",     required_argument, NULL, 'k' },
        { "reverse", required_argument, NULL, 'r' },
        { NULL, 0, NULL, 0 }
    };

    // Reset optind to re-parse the arguments
    optind = 1;

    // Second pass: Process other options
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (opt) {
            case 'k':
                check_arg_key(opt, env->modes & DC_VERBOSE);
                env->encryption_key = (unsigned char *)optarg;
                break;
            case 'r':
                env->modes |= DC_REVERSE;
                check_arg_key(opt, env->modes & DC_VERBOSE);
                env->decryption_key = (unsigned char *)optarg;
				if (env->modes & DC_VERBOSE)
					printf(FMT_MODE_ON " REVERSE mode enabled\n");
                break;
            case 'v':
                print_version();
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            case 's':
                break;
            default:
				if (env->modes & DC_VERBOSE)
					fprintf(
						stderr,
						FMT_ERROR "Invalid arguments. Use -h or --help for usage.\n"
					);
				exit(EXIT_FAILURE);
		}
    }

	if (env->modes & DC_VERBOSE)
        printf(FMT_MODE_ON " VERBOSE mode enabled\n");
}

void detect_silent_mode(t_env *env, int argc, char *argv[])
{
    int                 opt;
    bool                silent_mode = false;
    const char          *short_opts1 = "s";
    const struct option long_opts1[] = {
        { "silent",  no_argument,       NULL, 's' },
        { NULL, 0, NULL, 0 }
    };

	int original_opterr = opterr;   // Save the original value of opterr
	opterr = 0;                     // Disable error messages

    // First pass: check for silent mode
    while ((opt = getopt_long(argc, argv, short_opts1, long_opts1, NULL)) != -1) {
        if (opt == 's') {
            silent_mode = true;
            break; // Exit the loop once silent mode is detected
        }
    }

	// If silent mode not enabled, activate the verbose mode
	if (!silent_mode) {
		env->modes |= DC_VERBOSE;
		opterr = original_opterr;
	}
}
