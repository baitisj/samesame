
/* ************************************************************************ *
 * This is samefile driver. Programs can use this by including this source  *
 * file and implement the following functions:                              *
 * ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_MAIN_H
#define AK_MAIN_H

#include "configure.h"
#include "holder.h"

#include <new>

#define VERBOSE_LEVEL1		1
#define VERBOSE_LEVEL2		2
#define VERBOSE_LEVEL3		3
#define VERBOSE_MAX			3
#define VERBOSE_MASK		3
#define ADD_HARDLINKED		4
#define FULL_LIST			8
#define HUMAN_READABLE		16
#define MATCH_LEFT			32
#define MATCH_RIGHT			64
#define MATCH_TIME			128
#define REPORT_HARDLINKS	256
#define SIGNAL_TERM			512
#define SKIP_SORT			4096

#define MATCH_AUTO			(MATCH_LEFT | MATCH_RIGHT)
#define MATCH_MASK			(MATCH_LEFT | MATCH_RIGHT | MATCH_TIME)

#define S_ADD_HARDLINKED(m)		((m) & ADD_HARDLINKED)
#define S_FULL_LIST(m)			((m) & FULL_LIST)
#define S_HUMAN_READABLE(m)		((m) & HUMAN_READABLE)
#define S_MATCH(m)				((m) & (MATCH_LEFT | MATCH_RIGHT))
#define S_MATCH_LEFT(m)			((m) & MATCH_LEFT)
#define S_MATCH_RIGHT(m)		((m) & MATCH_RIGHT)
#define S_MATCH_TIME(m)			((m) & MATCH_TIME)
#define S_MATCH_MASK(m)			((m) & MATCH_MASK)
#define S_REPORT_HARDLINKS(m)	((m) & REPORT_HARDLINKS)
#define S_SKIP_SORT(m)			((m) & SKIP_SORT)
#define S_VERBOSE(m)			((m) & VERBOSE_MASK)
#define S_VERBOSE_LEVEL1(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL1)
#define S_VERBOSE_LEVEL2(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL2)
#define S_VERBOSE_LEVEL3(m)		(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL3)

extern unsigned flags;

class Filename;
class FileGroup;
class SizeGroup;
class Stats;

int processOptions(int argc, char **argv, void (&usage)(const char *)) throw();

Stats processInput(
	int (&printFileCompare)(const SizeGroup &, const FileGroup &,
		const Filename &, const FileGroup &, const Filename &,
		int result),
	int (&printHardLinked)(const char *a, const char *b, nlink_t nlink,
		off_t fileSize, const char *sep),
	int (&selectResults)(int flags, const char *sep),
	int (*preCheck)(const SizeGroup &,
		const FileGroup &, const FileGroup &) = NULL) throw();

void processStats(Stats &stats) throw();

#endif // AK_MAIN_H

