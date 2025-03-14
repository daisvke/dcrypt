#include "stockholm.h"

/* Get the filename from argv. It should correspond to the only arg
 *  doesn't begin with '-'.
 */
char *fa_get_filename(char *argv[])
{
	char *filename = NULL;
	for (int i = 1; argv[i] != NULL; i++)
	{
		if (argv[i][0] == '-')
			continue;
		else if (filename == NULL)
			filename = argv[i];
	}
	return filename;
}

// Check if the option given as 'arg' corresponds to the option given as 'opt'
static bool fa_check_opt(char *arg, char *opt)
{
	int opt_len = strlen(opt);
	int arg_len = strlen(arg);

	int len;
	if (opt_len > arg_len)
		len = opt_len;
	else
		len = arg_len;

	if (strncmp(arg, opt, len) == 0)
		return true;
	return false;
}

void fa_print_help()
{
	printf("HELP\n");
}

// Parre the given arguments and activate options
void fa_parse_argv(char *argv[])
{
	// We begin at index = 1 as index 0 contains the program name
	for (int i = 1; argv[i] != NULL; i++)
	{
		if (fa_check_opt(argv[i], "--help") || fa_check_opt(argv[i], "-h"))
			fa_print_help();
		if (fa_check_opt(argv[i], "--verbose") || fa_check_opt(argv[i], "-v"))
			g_modes |= FA_VERBOSE; // TODO --silent
		if (fa_check_opt(argv[i], "--reverse") || fa_check_opt(argv[i], "-r"))
			g_modes |= FA_REVERSE;
	}
}
