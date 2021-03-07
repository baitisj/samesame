
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mainAction.h"

static void *set;

/**
 * Removes one of the files and replaces it with a hard- of symlink to the
 * other.
 *
 * @returns PRINT_AGAIN          - always need to continue
 */
int mychmod(const char *left, const char *right,
	const struct stat &s1, const struct stat &s2,
	const char *dst, const char *src)
{
	if (S_DRYRUN(flags))
		return PRINT_AGAIN;
	mode_t m1 = getmode(set, s1.st_mode);
	mode_t m2 = getmode(set, s2.st_mode);
	if ((m1 & ALLPERMS) != (s1.st_mode & ALLPERMS))
		lchmod(left, m1);
	if ((m2 & ALLPERMS) != (s2.st_mode & ALLPERMS))
		lchmod(right, m2);
	return PRINT_AGAIN;
}

int main(int argc, char **argv)
{
	flags |= SYMLINK | WITHOUT_FILE_CHECK;
	actionProcessOptions(argc, argv, "chmod");

	int argi = actionProcessOptions(argc, argv, "chmod");
	if (argc <= argi)
		actionUsage("chmod");

	// get arguments
	if ((set = setmode(argv[argi])) == NULL)
		errx(1, "invalid file mode: %s", argv[argi]);

	// work
	actionProcessInput(mychmod);
	actionProcessStats();
}

