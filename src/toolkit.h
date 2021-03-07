
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_TOOLKIT_H
#define AK_TOOLKIT_H

#include "configure.h"

#include <stdio.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif // HAVE_STDDEF_H
#ifdef HAVE_STRING_H
#include <string.h>
#endif // HAVE_STRING_H
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif // HAVE_SYS_STAT_H

#include <new>

#define FILE_OPEN1_ERROR		-1
#define FILE_OPEN2_ERROR		-2
#define FILE_READ1_ERROR		-3
#define FILE_READ2_ERROR		-4
#define FILE_WRITE1_ERROR		-5
#define FILE_WRITE2_ERROR		-6
#define FILE_UNKOWN     		0
#define FILE_IDENTICAL  		1
#define FILE_DIFFERENT 			2

// for external use
#define FILE_USER1			4
#define FILE_USER2			8
#define FILE_USER3			16
#define FILE_USER4			32
#define FILE_USER5			64
#define FILE_USER6			128
#define FILE_USER7			256
#define FILE_USER8			512

#ifdef _SC_PHYS_PAGES
#define PHYSPAGES_THRESHOLD 32768 // more pages than this means a larger buffer
#ifndef MAXPHYS
#define MAXPHYS 131072 // 128K
#endif // MAXPHYS
#endif
#ifndef MAXBSIZE
#define MAXBSIZE 65536 // 64K
#endif

/**
 * Compares two files using there meta data and then the regular data.
 *
 * @param	*fd1 - file discriptor 1
 * @param	*fd2 - file discriptor 2
 * @param	s1 - size of file1
 * @param	s2 - size of file2
 */
int fcmp(int fd1, int fd2, off_t s1, off_t s2) throw();
int fcmp(int fd1, int fd2, struct stat &s1, struct stat &s2) throw();

/**
 * Compares two files using there meta data and then the regular data.
 *
 * @param    f1 - file1
 * @param    f2 - file2
 * @param    s1 - size of file1
 * @param    s2 - size of file2
 * @returns  FILE_OPEN1_ERROR    when f1 couldn't be opend
 *           FILE_OPEN2_ERROR    when f2 couldn't be opend
 *           FILE_UNKOWN         when some error accured.
 *           FILE_IDENTICAL      when the files are identical
 *           FILE_DIFFERENT      when the files are different
 */
int fcmp(const char *f1, const char *f2, off_t s1, off_t s2) throw();
int fcmp(const char *f1, const char *f2, struct stat &s1, struct stat &s2) throw();

/**
 * Makes a copy of src to dst.
 *
 * @param dst - destination file
 * @param src - source file
 * @param s - stat of src
 */
int fcpy(int dst, int src, struct stat &s) throw();

/**
 * Makes a copy of src to dst.
 *
 * @param dst - destination file
 * @param src - source file
 * @param s - stat of src
 */
int fcpy(const char *dst, const char *src, struct stat &s) throw();

#define DIRECTORY_EXISTED	2
#define DIRECTORY_CREATED	1
#define DIRECTORY_FAILED	0

/**
 * Creates the directory in path if the don't exists and base the
 * premissions set on the directories in src. The offset indicates
 * at what point the two strings have the same substring.
 *
 * @returns DIRECTORY_CREATED if the directory was created,
 *          DIRECTORY_EXISTED if the directory existed and
 *          DIRECTORY_FAILED if creation failed.
 */
int createDirectory(const char *path, size_t offset, const char *src);

/**
 * Returns the numberof digits the given number has.
 */
int digits(ulongest_t number) throw();
int digitsHumanReadable(ulongest_t number) throw();

/**
 * Reads a line from the input stream. If the capacity is to low the string
 * is enlarged and the old string is deleted.
 *
 * @param str - the location where the line goes
 * @param size - the capacity of the storage string
 * @param input - the stream to read from
 * @param eol - end of line
 */
char *fgetline(char *&str, size_t &size, FILE *file, int eol = '\n', 
				char *pos = NULL) throw (std::bad_alloc);

/**
 * Writes the size in a human friendly way to the stream out.
 */
void fprintsize(FILE *out, ulongest_t size) throw();

/**
 * Write the time difference between before and after to the stream out.
 * @param humanReable - do this in a human friendly way if non-0
 */
void fprinttime(FILE *out, struct timeval &after, struct timeval &before,
    int humanReadble = 0) throw();

#ifndef HAVE_STRSTR
char *strstr(const char *, const char *);
#endif // HAVE_STRSTR

inline void outputHardLinked(const char *f1, const char *f2,
	nlink_t links, size_t size, const char *sep) throw()
{
	printf("%zu%s%s%s%s%s[%zu]\n", size, sep, f1, sep, f2,
		sep, links);
}

inline void outputSamefile(const char *f1, const char *f2,
	unsigned int l1, unsigned int l2,
	size_t size, int sameDevice, const char *sep) throw()
{
	printf("%zu%s%s%s%s%s%c%s%u%s%u\n", size, sep, f1, sep, f2,
		sep, sameDevice ? '=' : 'X', sep, l1, sep, l2);
}

inline void
fprintLink(FILE *stream, const char *dst, const char *src, int hardlink)
{
	fprintf(stream, "%s %c> %s", src, hardlink ? '=' : '-', dst);
}

inline int inputSamefile(const char *line, size_t &size, char *&f1, char *&f2,
	const char *sep, size_t len) throw()
{
	char *str, *tmp;
// TODO XXX invalid conversion from 'const char*' to 'char*' ???
	if ((str = strstr((char *)line, (char *)sep)) == NULL || sscanf((char *)line, "%zu", &size) != 1)
		return 1;

	if ((str = strstr(tmp = str + len, sep)) == NULL)
		return 2;
	strncpy(f1, tmp, str - tmp);
	f1[str - tmp] = 0;

	if ((str = strstr(tmp = str + len, sep)) == NULL)
		return 3;
	strncpy(f2, tmp, str - tmp);
	f2[str - tmp] = 0;
	return 0;
}

inline size_t getBufferSize()
{
#ifdef _SC_PHYS_PAGES
	return MAXPHYS * (sysconf(_SC_PHYS_PAGES) > PHYSPAGES_THRESHOLD ? 8 : 1);
#else // _SC_PHYS_PAGES
	return MAXBSIZE;
#endif // _SC_PHYS_PAGES
}

#endif // AK_TOOLKIT_H

