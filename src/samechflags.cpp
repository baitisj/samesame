
/* ************************************************************************ *
 * ************************************************************************ * 
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "mainAction.h"

static int digital = 0, value;
static unsigned long set, clear;

/**
 * Removes one of the files and replaces it with a hard- of symlink to the
 * other.
 *
 * @returns PRINT_AGAIN          - always need to continue
 */
int mychflags(const char *left, const char *right,
	const struct stat &s1, const struct stat &s2,
	const char *dst, const char *src)
{
	if (S_DRYRUN(flags))
		return PRINT_AGAIN;

	if (digital)
	{
		lchflags(left, value);
		lchflags(right, value);
	}
	else
	{
		lchflags(left, (s1.st_flags | set) & clear);
		lchflags(right, (s2.st_flags | set) & clear);
	}
	return PRINT_AGAIN;
}

int main(int argc, char **argv)
{
	flags |= SYMLINK | WITHOUT_FILE_CHECK;
	actionProcessOptions(argc, argv, "chflags");

	int argi = actionProcessOptions(argc, argv, "chflags");
	if (argc <= argi)
		actionUsage("chflags");

	// get arguments
	if (*argv[argi] >= '0' && *argv[argi] <= '7')
	{
		char *end;
		errno = 0;
		value = strtol(argv[argi], &end, 8);
		if (value < 0)
			errno = ERANGE;
		if (errno)
			err(1, "invalid flags: %s", argv[argi]);
		if (*end)
			errx(1, "invalid flags: %s", argv[argi]);
		digital = 1;
	}
	else
	{
		char *flags = argv[argi];
		if (strtofflags(&flags, &set, &clear))
			errx(1, "invalid flag: %s", argv[argi]);
		clear = ~clear;
	}

	// work
	actionProcessInput(mychflags);
	actionProcessStats();
}

