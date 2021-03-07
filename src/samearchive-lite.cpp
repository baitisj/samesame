
/* ************************************************************************ *
 * samearchive-lite - Reads the paths from the input and output the         *
 *                    identical files. This program is written for the      *
 *                    special case where each directory acts as an archive  *
 *                    or backup. This program is written for the special    *
 *                    case where each directory acts as an archive or       *
 *                    backup. The output will only contain filename pairs   *
 *                    that have the same relative path from the archive     *
 *                    base.                                                 *
 *                                                                          *
 *                    This version uses a lot less memory then samefile and *
 *                    is faster, but only find a partial set of identical   *
 *                    files. It basicaly does 80% of the job, but does this *
 *                    in 50% of the time while using 10% of the resources   *
 *                    compared to samearchive.                              *
 *                                                                          *
 * Example:           find <dir1> | samearchive-lite <dir1> <dir2> [...]    *
 * ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "cache.h"
#include "toolkit.h"

#include <err.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif // HAVE_FCNTL_H
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif // HAVE_LIMITS_H
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif // HAVE_STRING_H
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif // HAVE_STDLIB_H
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif // HAVE_SYS_STAT_H
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

static Cache cache;

#define VERBOSE_LEVEL1	1
#define VERBOSE_LEVEL2	2
#define VERBOSE_LEVEL3	3
#define VERBOSE_MAX		3
#define VERBOSE_MASK	3

#define S_VERBOSE(m)			((m) & VERBOSE_MASK)
#define S_VERBOSE_LEVEL1(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL1)
#define S_VERBOSE_LEVEL2(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL2)
#define S_VERBOSE_LEVEL3(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL3)

// Retrieved from processOptions
static char *program = NULL;
static const char *sep = "\t";
static off_t minSize = 0, maxSize = OFF_MAX;
static unsigned int flags = VERBOSE_LEVEL1;
static int eol = '\n';

/**
 * Prints the usage of this program.
 */
static void usage()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "%s read a list of filenames from stdin and\n", program);
	fprintf(stderr, "archives (directories) from the paramters and\n");
	fprintf(stderr, "writes a list of identical files on stdout.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "usage: %s [-g size] [-s sep] [-aqVv] <dir1> <dir2> [...]\n", program);
	fprintf(stderr, "exampe: find <dir1> | %s [options] <dir1> <dir2> [...]\n", program);
	fprintf(stderr, "\n");
	fprintf(stderr, "  Options: -g : only output files greater than size (0)\n");
	fprintf(stderr, "           -m : only output files less or equal than size (0)\n");
	fprintf(stderr, "           -q : suppress non-error messages\n");
	fprintf(stderr, "           -S : use sep as separator string for files (tab)\n");
	fprintf(stderr, "           -V : output version information and exit\n");
	fprintf(stderr, "           -v : more verbose output on stderr\n");
	fprintf(stderr, "\n");
	exit(EXIT_SUCCESS);
}

static int processOptions(int argc, char **argv)
{
	(program = rindex(argv[0], '/')) ? ++program : program = argv[0];

	int c;
	while((c = getopt(argc, argv, "h?g:S:qVv")) != -1)
		switch(c)
		{
			default: case 'h': case '?': usage();						break;
			case '0': eol = 0;											break;
			case 'g':
				if (sscanf(optarg, "%ju", &minSize) != 1)
					minSize = 0,
							warn("can't convert -g %s, using -g 0 instead",
									optarg);
				break;
			case 'm':
				if (sscanf(optarg, "%ju", &maxSize) != 1)
					minSize = 0,
							warn("can't convert -m %s, using -g 0 instead",
									optarg);
				break;

			case 'S':sep = optarg;										break;

			case 'q': flags &= ~VERBOSE_MASK;							break;
			case 'V':
				printf(COPYRIGHT, PACKAGE_STRING, program);
				exit(EXIT_SUCCESS);
			break;

			case 'v': if ((S_VERBOSE(flags)) < VERBOSE_MAX) ++flags;	break;
		}
	return optind;
}

/**
 * Checks all files on the standerd input with there counter parts in the other
 * archives.
 */
static void processInput(int argc, char **argv)
{
	if (argc < 2)
		usage();
	size_t len0 = strlen(argv[0]);
	if (argv[0][len0 - 1] == '/')
		argv[0][--len0] = 0;
	if (argv[1][strlen(argv[1]) - 1] == '/')
		argv[1][strlen(argv[1]) - 1] = 0;

	// filther out doubles
	int *skip = new int[argc];
	{
		int counter = 0;
		for (int i = 1; i < argc; ++i)
		{
			skip[i] = 0;
			for (int j = 0; j < i; ++j)
				if (!strcmp(argv[i], argv[j]))
				{
					skip[i] = 1;
					++counter;
					continue;
				}
		}

		// Print usage if there are not at least two archives remaining
		if (argc < 2)
		{
			usage();
			delete[] skip;
			exit(EXIT_SUCCESS);
		}
	}

	// How much extra space do we need in f2 compaired to f1?
	size_t diff = strlen(argv[1]) - strlen(argv[0]);
   	for (int i = 0; i < argc; ++i)
   	{
		if (argv[i][strlen(argv[i]) - 1] == '/')
			argv[i][strlen(argv[i])] = 0;
		if (diff < strlen(argv[i]) - strlen(argv[0]))
			 diff = strlen(argv[i]) - strlen(argv[0]);
	}

	int len = 0;
	size_t f1n = 256, f2n = 256;
	char *f1 = new char[f1n], *f2 = new char[f2n];

	// Shortcut: the first part of f2 is constant if argc == 2
	if (argc == 2)
		memcpy(f2, argv[1], len = strlen(argv[1]));

	struct match_t m;
	while(fgetline(f1, f1n, stdin, eol) != NULL)
	{
		if (S_VERBOSE_LEVEL2(flags))
			fprintf(stderr, "%s\n", f1);

		// Skip unlinkble lines
		if (lstat(f1, &m.s1) < 0 || S_ISREG(m.s1.st_mode) == 0 ||
			m.s1.st_size <= minSize || m.s1.st_size > maxSize)
				continue;

		// Skip lines that do not start with argv[0]
		if (strlen(f1) < len0 || memcmp(f1, argv[0], len0))
		{
			warn("Skipped %s because it didn't start with %s.\n", f1, argv[0]);
			warn("%s %zi < %zi\n", f1, strlen(f1), len0);
			continue;
		}

		// check on all other archives
		char *f = f1 + strlen(argv[0]);
		for (int i = 1; i < argc; ++i)
		{
			// filther out doubles
			if (skip[i])
				continue;

			// enlage f2 if needed
			if (f2n < strlen(f1) + diff)
			{
				while ((f2n <<= 1) < strlen(f1) + diff);
				delete[] f2;
				f2 = new char[f2n];
				if (argc == 2)
					memcpy(f2, argv[1], len);
			}

			// f2 = argv[i] + f
			if (argc > 2)
				memcpy(f2, argv[i], len = strlen(argv[i]));
			strcpy(f2 + len, f);

			// Skip unlinkble lines, diffent sizes or same file 
			if (lstat(f2, &m.s2) < 0 || S_ISREG(m.s2.st_mode) == 0 ||
				m.s1.st_dev == m.s2.st_dev && m.s1.st_ino == m.s2.st_ino ||
				m.s1.st_size != m.s2.st_size)
			{
				continue;
			}

			// check f1 with f2
			if (cache != m)
			{
				m.result = fcmp(f1, f2, m.s1, m.s2);
				cache += m;
			}
			switch(cache[m].result)
			{
				case FILE_OPEN1_ERROR:
					if (S_VERBOSE_LEVEL1(flags))
						warn("inaccessible: %s\n", f1);
					break;
				case FILE_OPEN2_ERROR:
					if (S_VERBOSE_LEVEL1(flags))
						warn("inaccessible: %s\n", f2);
					break;
				case FILE_READ1_ERROR:
					if (S_VERBOSE_LEVEL1(flags))
						warn("unreadable: %s\n", f1);
					break;
				case FILE_READ2_ERROR:
					if (S_VERBOSE_LEVEL1(flags))
						warn("unreadable: %s\n", f2);
					break;
				case FILE_IDENTICAL:
					outputSamefile(f1, f2, m.s1.st_nlink, m.s2.st_nlink,
						m.s1.st_size, m.s1.st_dev == m.s2.st_dev, sep);
					break;
			}
		}
	}
	delete[] skip;
	delete[] f1;
	delete[] f2;
}

int main(int argc, char **argv)
{
	setlinebuf(stdout);
	int offset = processOptions(argc, argv);
	processInput(argc - offset, argv + offset);
	return 0;
}

