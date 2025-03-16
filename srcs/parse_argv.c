#include "stockholm.h"

void print_help() {
    printf("Usage: ./stockholm [-h|-v|-s|-r <KEY>] \n"
           "Options:\n"
           "  -v, --version          Show version information\n"
           "  -s, --silent           Run in silent mode (non-verbose)\n"
           "  -r, --reverse <KEY>    Decrypt using the provided decryption key\n"
           "  -h, --help             Show this help message and exit\n");
}

void fa_print_version()
{
	printf(
		FA_PROG_NAME " " FA_PROG_VERSION " | Copyright (c) " FA_PROG_AUTHOR "\n"
	);
	exit(EXIT_SUCCESS);
}

void fa_parse_argv(fa_t_env *env, int argc, char *argv[])
{
    const char			*short_opts = "hvsr";
    const struct option	long_opts[] = {
        { "help",		no_argument,		NULL, 'h' },
        { "version",	no_argument,		NULL, 'v' },
        { "silent",		no_argument,		NULL, 's' },
        { "reverse",	required_argument,	NULL, 'r' },
        { NULL, 0, NULL, 0 }
    };
    int					opt;
	bool				silent_mode = false;

    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (opt) {
			case 'r':
				env->g_modes |= FA_REVERSE;
				if (optarg == NULL) { // Check if the argument is provided
					fprintf(
						stderr,
						"The -r option requires the decryption key as an argument.\n"
					);
					exit(EXIT_FAILURE);
				}
				env->g_decryption_key = optarg;
				break;
			case 's':
				silent_mode = true;
				break;
			case 'v':
				fa_print_version();
				break;
			case 'h':
				print_help();
				exit(EXIT_SUCCESS);
			default:
				fprintf(stderr, "Invalid arguments. Use -h or --help for usage.\n");
				exit(EXIT_FAILURE);
        }
    }

	if (!silent_mode) {
		env->g_modes |= FA_VERBOSE;
		printf("In verbose mode...\n");
	}
}
