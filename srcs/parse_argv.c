#include "stockholm.h"

void print_help() {
    printf(FMT_INFO " Usage: ./stockholm [-h|-v|-s|-k <KEY>|-r <KEY>] \n\n"
           FMT_INFO " Options:\n"
           "  -v, --version          Show version information\n"
           "  -s, --silent           Run in silent mode (non-verbose)\n"
           "  -k, --key <KEY>    	 Provide an encryption key\n"
           "  -r, --reverse <KEY>    Decrypt using the provided decryption key\n"
           "  -h, --help             Show this help message and exit\n");
}

void print_version()
{
	printf(
		SH_PROG_NAME " " SH_PROG_VERSION " | Copyright (c) " SH_PROG_AUTHOR "\n"
	);
	exit(EXIT_SUCCESS);
}

void print_arg_key_error_msg(void)
{
	fprintf(
		stderr,
		FMT_ERROR
		" The -k/-r options require a 128 bits encryption key as an argument.\n"
	);
}

bool check_arg_key(void)
{
	// Check if the argument is provided and that the key has required length
	if (!optarg || (strlen(optarg) != SH_AES_KEY_SIZE))
		return SH_ERROR;
	return SH_SUCCESS;
}

void parse_argv(t_env *env, int argc, char *argv[])
{
    const char			*short_opts = "hvsr:k:";
    const struct option	long_opts[] = {
        { "help",		no_argument,		NULL, 'h' },
        { "version",	no_argument,		NULL, 'v' },
        { "silent",		no_argument,		NULL, 's' },
        { "key",		required_argument,	NULL, 'k' },
        { "reverse",	required_argument,	NULL, 'r' },
        { NULL, 0, NULL, 0 }
    };
    int					opt;
	bool				silent_mode = false;
	bool				invalid_opt = false;
	bool				key_missing = false;

    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (opt) {
			case 'k':
				key_missing = key_missing == true ? true : check_arg_key();
				env->encryption_key = (unsigned char*)optarg;
				break;
			case 'r':
				env->modes |= SH_REVERSE;
				key_missing = key_missing == true ? true : check_arg_key();
				env->decryption_key = (unsigned char*)optarg;
				break;
			case 's':
				silent_mode = true;
				break;
			case 'v':
				print_version();
				break;
			case 'h':
				print_help();
				exit(EXIT_SUCCESS);
			default:
				invalid_opt = true;
        }
    }

	if (!silent_mode) {
		env->modes |= SH_VERBOSE;
		printf(FMT_MODE_ON " VERBOSE mode enabled\n");

		if (env->modes & SH_REVERSE)
			printf(FMT_MODE_ON " REVERSE mode enabled\n");

		if (invalid_opt) {
			fprintf(
				stderr,
				FMT_ERROR " Invalid arguments. Use -h or --help for usage.\n"
			);
		}
	}

	if (key_missing) {
		if (!silent_mode) print_arg_key_error_msg();
		exit(EXIT_FAILURE);
	}

	if (invalid_opt) exit(EXIT_FAILURE);
}
