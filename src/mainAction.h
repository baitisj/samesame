
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include "configure.h"
#include "cache.h"
#include "toolkit.h"

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif // HAVE_LIMITS_H
#include <signal.h>
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
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#define VERBOSE_LEVEL1		1
#define VERBOSE_LEVEL2		2
#define VERBOSE_LEVEL3		3
#define VERBOSE_MAX			3
#define VERBOSE_MASK		3
#define DIRECTORY			4
#define DRYRUN				8
#define HARDLINK			16
#define HUMAN_READABLE		32
#define MATCH_LEFT			64
#define MATCH_RIGHT	 		128
#define MATCH_TIME			256
#define SYMLINK				512
#define WITHOUT_FILE_CHECK	1024

#define MATCH_AUTO			(MATCH_LEFT | MATCH_RIGHT)
#define MATCH_MASK			(MATCH_LEFT | MATCH_RIGHT | MATCH_TIME)

#define S_DIRECTORY(m)			((m) & DIRECTORY)
#define S_DRYRUN(m)				((m) & DRYRUN)
#define S_HARDLINK(m)			((m) & HARDLINK)
#define S_HUMAN_READABLE(m)		((m) & HUMAN_READABLE)
#define S_MATCH(m)				((m) & MATCH_MASK)
#define S_MATCH_TIME(m)			((m) & MATCH_TIME)
#define S_SYMLINK(m)			((m) & SYMLINK)
#define S_VERBOSE(m)			((m) & VERBOSE_MASK)
#define S_VERBOSE_LEVEL1(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL1)
#define S_VERBOSE_LEVEL2(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL2)
#define S_VERBOSE_LEVEL3(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL3)
#define S_WITHOUT_FILE_CHECK(m)	((m) & WITHOUT_FILE_CHECK)

#define FAILED_PIPE_OPTION		-9
#define FAILED_FILE_OPTION		-10
#define FAILED_LINK_LEFT		-8
#define FAILED_LINK_RIGHT		-7
#define FAILED_RESTORE_LEFT		-6
#define FAILED_RESTORE_RIGHT	-5
#define FAILED_REMOVE_LEFT		-4
#define FAILED_REMOVE_RIGHT		-3
#define FAILED_BACKUP_CREATE	-2
#define FAILED_BACKUP_DELETE	-1
#define PRINT_AGAIN				0
#define SUCCES_LEFT				1
#define SUCCES_RIGHT			2
#define DISMISS_SILENTLY		3

// Retrieved from actionProcessOptions
extern unsigned int flags;

// Extended apps store there errno in this variable
extern int same_errno;

/**
 * Append to path starting from pathOffset from src starting at srcOffset.
 * @param pathCapacity - maximum capacity of path.
 * @param srcLen - size of src.
 */
size_t getPath(char *&path, size_t pathOffset, size_t pathCapacity,
	const char *src, size_t srcOffset);

/**
 * Appends param to the end of str.
 * @param capacity - the maximum capacity of str.
 */
size_t getParameter(char *&str, char *param, size_t capacity);

/**
 * Prints the usage of this program.
 */
void actionUsage(const char *command) throw();

int actionProcessOptions(int argc, char **argv, const char *command) throw();

/**
 * Read each line from stdin and perform some form of action
 */
void actionProcessInput(void (&skip)(const char *line),
	int (&action)(Cache &, match_t &,
		const char *line, const char *f1, const char *f2));

/**
 * Read each line from stdin and perform some action
 */
void actionProcessInput(int (&func)(const char *f1, const char *f2,
	const struct stat &s1, const struct stat &s2, 
	const char *dst, const char *src));

void actionProcessStats() throw();

