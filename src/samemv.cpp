
/* ************************************************************************ *
 * samelink - Read the output from samefile output and links identical      *
 *            files together. The matches that coudn't be processed are     *
 *            printed on the output.                                        *
 *                                                                          *
 * Hard link identical files, keepin the last modified files:               *
 *            find / | samefile -iZt | samelink -Zt                         *
 *                                                                          *
 * Create symlinks to between files that are not on the same filesystem     *
 *            find / | samefile -i | samelink -s                            *
 * ************************************************************************ * 
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include "mainAction.h"

#include <err.h>

static char *path, *src;
static size_t pathOffset, srcOffset, pathCapacity = 1024;

/**
 * Removes one of the files and replaces it with a hard- of symlink to the
 * other.
 *
 * @returns PRINT_AGAIN          - files didn't meet the requirements
 *          FAILED_LINK          - coudn't create a link
 *          FAILED_MOVE          - coudn't create a move
 *          SUCCES_LEFT          - on succes and the left file was removed
 *          SUCCES_RIGHT         - on succes and the right file was removed
 */
int move(const char *left, const char *right,
	const struct stat &s1, const struct stat &s2,
	const char *dst, const char *src)
{
	// check if ::src is a substring of src
	size_t srcLen = strlen(src);
	if (srcOffset < srcLen && memcmp(::src, src, srcOffset))
		return PRINT_AGAIN; // no it is not

	// create path
	pathCapacity = getPath(path, pathOffset, pathCapacity, src, srcOffset);

	// create directories
	if (!S_DRYRUN(flags) && createDirectory(path, pathOffset, src) < 0)
		return PRINT_AGAIN;

	// chek if path exists and if it does print again.
	struct stat &p;
	if (lstat(path, &p) >= 0)
	{
		switch(fcmp(path, src, p, s))
		{
			case FILE_OPEN1_ERROR:
				if (S_VERBOSE_LEVEL1(flags))
					warn("file already exists but coudn't open %s\n",
							path);
				return PRINT_AGAIN;

			case FILE_READ1_ERROR:
				if (S_VERBOSE_LEVEL1(flags))
					warn("file already exists but coudn't read %s\n",
							path);
				return PRINT_AGAIN;

			case FILE_OPEN2_ERROR:
				if (S_VERBOSE_LEVEL1(flags))
					warn("coudn't open %s\n", src);
				return PRINT_AGAIN;

			case FILE_READ2_ERROR:
				if (S_VERBOSE_LEVEL1(flags))
					warn("coudn't read %s\n", src);
				return PRINT_AGAIN;

			case FILE_UNKOWN:
				if (S_VERBOSE_LEVEL2(flags))
					warn("file %s existed, but differs from %s\n",
							path, src);
				return PRINT_AGAIN;

			case FILE_DIFFERENT:
				if (S_VERBOSE_LEVEL2(flags))
					warn("file %s existed, but differs from %s\n",
							path, src);
				return PRINT_AGAIN;
		}
	}

	// rename of make a copy and remove if that fails.
	if (!S_DRYRUN(flags) && rename(src, path) < 0)
	{
		struct stat s;
		lstat(src, &s);
		mode_t oumask = umask(p.st_mode);
		fcpy(path, src, s);
		unlink(src);
		umask(oumask);
	}
	if (S_VERBOSE_LEVEL2(flags))
	{
		fprintLink(stdout, dst, src, 0);
		fprintf(stdout, "\n");
	}
	return SUCCES_LEFT;
}

// user ... | samecp dst [src]
int main(int argc, char **argv)
{
	int argi = actionProcessOptions(argc, argv, "move");
	if (argc <= argi)
		actionUsage("move");

	// get dst
	path = new char[pathCapacity], pathOffset = strlen(argv[argi]);
	path[0] = 0;
	pathCapacity = getParameter(path, argv[argi], pathCapacity);

	// get src
	if (++argi < argc)
	{
		src = new char[(srcOffset = strlen(argv[argi])) + 1];
		memcpy(src, argv[argi], srcOffset + 1);
	}

	// work
	actionProcessInput(move);
	actionProcessStats();
}

