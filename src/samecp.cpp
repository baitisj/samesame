
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
#include <errno.h>

static char *path, *src;
static size_t pathOffset, srcOffset, pathCapacity = 1024;

/**
 * Removes one of the files and replaces it with a hard- of symlink to the
 * other.
 *
 * @returns PRINT_AGAIN          - files didn't meet the requirements
 *          FAILED_LINK          - coudn't create a link
 *          FAILED_COPY          - coudn't create a copy
 *          SUCCES_LEFT          - on succes and the left file was removed
 *          SUCCES_RIGHT         - on succes and the right file was removed
 */
int copy(const char *left, const char *right,
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

	// create a link or a copy
	if (S_HARDLINK(flags) && s1.st_dev == s2.st_dev || S_SYMLINK(flags))
	{
		int (*linkf)(const char *dst, const char *src) =
			s1.st_dev == s2.st_dev ? link : symlink;
		if (!S_DRYRUN(flags) && linkf(path, left) < 0)
		{
			same_errno = errno;
			if (S_VERBOSE_LEVEL1(flags))
				warn("g: coudn't create the link %s %c> %s",
						src, s1.st_dev == s2.st_dev ? '=' : '-', dst);
			return FAILED_LINK_RIGHT;
		}
		if (S_VERBOSE_LEVEL2(flags))
		{
			fprintLink(stdout, dst, src, s1.st_dev == s2.st_dev);
			fprintf(stdout, "\n");
		}
	}
	else // make a copy
	{
		if (!S_DRYRUN(flags))
		{
			struct stat p, s;
			if (lstat(path, &p) < 0 && errno == ENOENT)
			{
				lstat(src, &s);
				mode_t oumask = umask(p.st_mode);
				fcpy(path, src, s);
				umask(oumask);
			}
			else
			{
				lstat(path, &p);
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
		}
		if (S_VERBOSE_LEVEL2(flags))
		{
			fprintLink(stdout, dst, src, 0);
			fprintf(stdout, "\n");
		}
	}
	return SUCCES_LEFT;
}

// user ... | samecp dst [src]
int main(int argc, char **argv)
{
	int argi = actionProcessOptions(argc, argv, "copy");
	if (argc <= argi)
		actionUsage("copy");

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
	actionProcessInput(copy);
	actionProcessStats();
}

