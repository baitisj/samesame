
/* ************************************************************************ *
 * samearchive - Reads the paths from the input and output the identical    *
 *               files. This program is written for the special case        *
 *               where each directory acts as an archive or backup. The     *
 *               The output will only contain filename pairs that have the  *
 *               same relative path from the archive base.                  *
 *                                                                          *
 * Example:      find /backup/ | samearchive <dir1> <dir2> [<dir 3> ...]    *
 * ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "toolkit.h"
#include "stats.h"
#ifdef DEBUG
#include "debug.cpp"
#endif // DEBUG

// This file holds the engine code
#include "main.h"

#include <err.h>

static int argc;
static size_t *length;
static char **argv;
static const char *sep;

/**
 * Prints the usage of this program.
 */
static void usage(const char *program) throw()
{
	fprintf(stderr, "\n%s: read alist of filenames from stdin and", program);
	fprintf(stderr, "\nwrites a list of identical files on stdout.\n");

	fprintf(stderr, "\nusage: %s [-a | -A | -L | -Z | -At | -Az] [-g size] \\", program);
	size_t len = strlen(program);
	fprintf(stderr, "\n        ");
	for (size_t i = 0; i < len; ++i)
		fprintf(stderr, " ");
	fprintf(stderr, "[-l | -r] [-m size] [-S sep] [-0HiqVv] <dir1> <dir2> [...]");

	fprintf(stderr, "\nexample: find <dir> | %s [options] <dir1> <dir2> [...]", program);
	fprintf(stderr, "\n");
	fprintf(stderr, "\n  options: -A match based on the first filename (default)");
	fprintf(stderr, "\n           -a don't sort files with same size alphabetically");
	fprintf(stderr, "\n           -g only output files greater than size (0)");
	fprintf(stderr, "\n           -H human readable statistics");
	fprintf(stderr, "\n           -i add files with identical inodes");
	fprintf(stderr, "\n           -L match based on the number of links");
	fprintf(stderr, "\n           -l don't report hard linked filenames (default)");
	fprintf(stderr, "\n           -m only output files less or equal than size (0)");
	fprintf(stderr, "\n           -q suppress non-error messages");
	fprintf(stderr, "\n           -r report hard linked filenames");
	fprintf(stderr, "\n           -S use sep as separator string for files (tab)");
#ifndef LOW_MEMEMORY_PROFILE
	fprintf(stderr, "\n           -t match based on the modiciation time \\");
#endif // LOW_MEMEMORY_PROFILE
	fprintf(stderr, "\n              instead of the alphabethical order");
	fprintf(stderr, "\n           -V output version information and exit");
	fprintf(stderr, "\n           -v increase verbosity");
	fprintf(stderr, "\n           -Z match based on the last filename");
	fprintf(stderr, "\n");
	exit(EXIT_SUCCESS);
}

inline int passCombination(const char *a, const char *b,
	size_t len1, size_t len2)
{
	for (int i = 0; i < argc; ++i) if (length[i] <= len1)
		for (int j = i + 1; j < argc; ++j) if (length[j] <= len2)
			if (!memcmp(argv[i], a, length[i]) &&
				!memcmp(argv[j], b, length[j]) &&
				!strcmp(a + length[i], b + length[j]))
				return 0;
	return 1;
}

static int preCheck(const SizeGroup &parent,
const FileGroup &left, const FileGroup &right) throw()
{
	Iterator<Filename> &a = *left.createIterator();
	Iterator<Filename> &b = *right.createIterator();
	for (; !a.end(); ++a) if (a.getItem() != NULL)
	{
		const char *aData = a.getItem()->data();
		size_t len1 = strlen(aData);
		for  (b.reset(); !b.end(); ++b) if (b.getItem() != NULL)
			if (!passCombination(aData, b.getItem()->data(),
				len1, strlen(b.getItem()->data())))
			{
				delete &a;
				delete &b;
				return 0;
			}
	}
	delete &a;
	delete &b;
	return 1;
}

static int selectResults(int flags, const char *sep)
{
	::flags = flags;
	::sep = sep;
	return FILE_IDENTICAL | FILE_BY_LOGIC;
}

static int printFileCompare(const SizeGroup &parent,
	const FileGroup &left, const Filename &leftChild,
	const FileGroup &right, const Filename &rightChild,
	int result) throw()
{
	if (passCombination(leftChild.data(), rightChild.data(),
		strlen(leftChild.data()), strlen(rightChild.data())))
		return 0;

	switch(result)
	{
		case FILE_IDENTICAL:
		case FILE_IDENTICAL | FILE_BY_LOGIC:
			outputSamefile(leftChild.data(), rightChild.data(),
				left.nlink, right.nlink,
				parent.getFileSize(), left.isOnSameDevice(right), sep);
			break;
		case FILE_OPEN1_ERROR:
			if (S_VERBOSE_LEVEL1(flags))
				warn("inaccessible: %s\n", leftChild.data());
			break;
		case FILE_OPEN2_ERROR:
			if (S_VERBOSE_LEVEL1(flags))
				warn("inaccessible: %s\n", rightChild.data());
			break;
		case FILE_READ1_ERROR:
			if (S_VERBOSE_LEVEL1(flags))
				warn("unreadable: %s\n", leftChild.data());
			break;
		case FILE_READ2_ERROR:
				if (S_VERBOSE_LEVEL1(flags))
				warn("unreadable: %s\n", rightChild.data());
	}
	return 0;
}

static int printHardLinked(const char *a, const char *b, nlink_t nlink,
	off_t fileSize, const char *sep)
{
	if (passCombination(a, b, strlen(a), strlen(b)))
		return 0;

        outputHardLinked(a, b, nlink, fileSize, sep);
	return 0;
}

int main(int argc, char **argv)
{
#if HAVE_MALLOC
#if __BSD_VISIBLE
	_malloc_options = "H";
#endif // __BSD_VISIBLE
#else // HAVE_MALLOC
#error malloc function required, see file config.h.in
#endif // HAVE_MALLOC
	setlinebuf(stdout);
	int offset = processOptions(argc, argv, usage);
	::argc = argc - offset;
	::argv = argv + offset;
	length = new size_t[::argc];
	for (int i = 0; i < ::argc; ++i)
		length[i] = strlen(::argv[i]);
	Stats stats = processInput(printFileCompare, printHardLinked,
		selectResults, preCheck);
	processStats(stats);
	delete[] length;
#ifdef DEBUG
	checkDynamic();
#endif // DEBUG

}

