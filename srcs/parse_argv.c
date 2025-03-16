#include "stockholm.h"

void print_help() {
    printf("Usage: ./ft_otp [OPTIONS] <key file>\n"
           "Options:\n"
           "  -g, --generate     Generate and save the encrypted key\n"
           "  -k, --key          Generate password using the provided key\n"
           "  -q, --qrcode       Generate a QR code containing the key (requires -g)\n"
           "  -v, --verbose      Enable verbose output\n"
           "  -h, --help         Show this help message and exit\n");
	exit(EXIT_SUCCESS);
}

void fa_print_version()
{
	printf("Version 0.0.1\n");
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
			case 'h':
				print_help();
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
