
/* ************************************************************************ *
 *                         This is samefile driver.                         *
 * ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "toolkit.h"
#include "storage.h"
#include "write2disk.h"
#include "printhardlinked.h"
#include "stats.h"
#include "matchmatrix.h"
#include "holder.h"
#include "main.h"

#include <err.h>
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
#include <sysexits.h>
#include <sys/types.h>
#if defined(HAVE_SYS_TIME_H) && defined(HAVE_GETTIMEOFDAY)
#include <sys/time.h>
#endif // HAVE_SYS_TIME_H && HAVE_GETTIMEOFDAY
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#include <new>

// Retrieved from processOptions
static const char *program = NULL;
static const char *sep = "\t";
static off_t minSize = 0, maxSize = OFF_MAX;
unsigned flags = VERBOSE_LEVEL1 | MATCH_LEFT;
static int eol = '\n';

// Retrieved from processInput
#ifdef DEBUG
static struct timeval time0, time1, time2, time3, time4;
#else
static struct timeval time0, time2, time3, time4;
#endif

// Retrieved from deleteEarly
static time_t deleteEarly_t = 0;

static ulongest_t waisted = 0;

Stats *quitStats;

static void quit(int sig)
{
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time4, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY
	processStats(*quitStats);
	exit(EXIT_SUCCESS);
}

/*
static void term(int sig)
{
	flags |= SIGNAL_TERM;
}
*/

static void registerQuit(Stats &stats)
{
	quitStats = &stats;
#ifdef SIGALRM
	signal(SIGALRM, quit);
#endif // SIGALRM
#ifdef SIGHUP
	signal(SIGHUP, quit);
#endif // SIGHUP
#ifdef SIGINT
	signal(SIGINT, quit);
#endif // SIGINT
#ifdef SIGPIPE
	signal(SIGPIPE, quit);
#endif // SIGPIPE
#ifdef SIGPROF
	signal(SIGPROF, quit);
#endif // SIGPROF
#ifdef SIGQUIT
	signal(SIGQUIT, quit);
#endif // SIGQUIT
#ifdef SIGTERM
	signal(SIGTERM, quit);
#endif // SIGTERM
#ifdef SIGTHR
	signal(SIGTHR, quit);
#endif // SIGTHR
#ifdef SIGUSR1
	signal(SIGUSR1, quit);
#endif // SIGUSR1
#ifdef SIGUSR2
	signal(SIGUSR2, quit);
#endif // SIGUSR2 
#ifdef SIGVTALRM 
	signal(SIGVTALRM, quit);
#endif // SIGVTALRM
#ifdef SIGXCPU
	signal(SIGXCPU, quit);
#endif // SIGXCPU
#ifdef SIGXFSZ
	signal(SIGXFSZ, quit);
#endif // SIGVTALRM
#ifdef SIGPIPE
	signal(SIGPIPE, quit);
#endif // SIGPIPE
}


int processOptions(int argc, char **argv, void (&usage)(const char *)) throw()
{
	(program = rindex(argv[0], '/')) ? ++program : program = argv[0];

	int c;
	while((c = getopt(argc, argv, "h?g:m:s:aH0xALZtilrqVv")) != -1)
		switch(c)
		{
			default: case 'h': case '?': usage(program);				break;

			case 'g':
				if (sscanf(optarg, "%ji",  &minSize) !=  1)
					minSize = 0, fprintf(stderr,
						"warning: can't convert -g %s, using -g 0 instead\n",
						optarg);
				break;
			case 'm':
				if (sscanf(optarg, "%ji",  &maxSize) !=  1)
					maxSize = 0, fprintf(stderr,
						"warning: can't convert -m %s, using -g 0 instead\n",
						optarg);
				break;

			case 's':
				fprintf(stderr, "this option is obsolite use -S instead\n");
			case 'S': sep = optarg; 									break;
			case 'H': flags |= HUMAN_READABLE; 							break;
			case '0': eol = 0; 											break;

			case 'a': flags &= ~MATCH_MASK;		 						break;
			case 'A': flags &= ~MATCH_RIGHT;	flags |= MATCH_LEFT;	break;
			case 'L': flags &= ~MATCH_MASK; 	flags |= MATCH_AUTO; 	break;
			case 'Z': flags &= ~MATCH_LEFT;		flags |= MATCH_RIGHT;  	break; 	
#ifndef LOW_MEMEMORY_PROFILE
			case 't': 							flags |= MATCH_TIME; 	break;
#endif // LOW_MEMEMORY_PROFILE
			case 'x': 						flags |= FULL_LIST; 		break;
			case 'i':						flags |= ADD_HARDLINKED;	break;
			case 'r':						flags |= REPORT_HARDLINKS;	break;
			case 'l': flags &= ~REPORT_HARDLINKS;						break;

			case 'q': flags &= ~VERBOSE_MASK; 							break;
			case 'V':
				printf(COPYRIGHT, PACKAGE_STRING, program);
				exit(EXIT_SUCCESS);
			break;
			case 'v': if ((S_VERBOSE(flags)) < VERBOSE_MAX) ++flags; 	break;
		}

	return optind;
}

#ifdef DISK_STORAGE
static void
solveMemoryProblem(Holder &holder, Write2Disk &write2Disk, size_t flags)
#else // DISK_STORAGE
static void solveMemoryProblem(Holder &holder, size_t flags)
#endif // DISK_STORAGE
{
	Stats stats;
	holder.accept(stats);
	off_t max = stats.getMaxFileSize();

#ifdef DISK_STORAGE
	size_t maxDiskFileSize = 0;
	size_t min = stats.getMinFileSize();
	size_t avg = stats.getFilenames() ? stats.getTotalSize() / stats.getFilenames() : 0;

	// would give problems with sorting.
	switch(S_MATCH_MASK(::flags))
	{
		case MATCH_LEFT:								// -A
		case MATCH_RIGHT:								// -Z
			write2Disk.reset(0);
			flags &= ~1;
	}

	// try to write to disk
	if (flags & 1)
	{
		do
		{
			if (maxDiskFileSize < avg / 2 + min / 2)
				maxDiskFileSize = avg / 2 + min / 2;
			else if (maxDiskFileSize < avg)
				maxDiskFileSize = avg;
			else
				maxDiskFileSize = max;
			holder.accept(write2Disk.reset(maxDiskFileSize));
		}
		while(maxDiskFileSize < max && write2Disk.done() <= 0);
		if (write2Disk.done())
		{
			fprintf(stderr, "%s: Written %u paths to disk\n",
				program, write2Disk.done());
			return;
		}
	}

#endif // DISK_STORAGE
	// try to remove from memory if we fail
	if (flags & 2)
	{
		unsigned long counter = 0;
		do
		{
			if (minSize == 0)				minSize = 32;
			else if (minSize < max / 2)		minSize <<= 1;
			else if (minSize >= max / 2)	minSize += (max - minSize) / 2;
			counter += holder.remove(0, minSize);
		}
		while(counter == 0 && minSize < max - 1);
	}

	// abort if we fail
	if (minSize >= max - 1) // TODO: It would nice to include the highest unhandled file size.
		err(EXIT_FAILURE, "Try using -m and -g or changing the file set");
	else if (S_VERBOSE_LEVEL1(flags))
		fprintf(stderr, "%s: Changed minimum file size to %zu\n",
			program, minSize);
}

#ifdef DISK_STORAGE
static void readInput(Holder &holder, Write2Disk &write2Disk) throw()
#else // DISK_STORAGE
static void readInput(Holder &holder) throw()
#endif // DISK_STORAGE
{
	size_t capacity = PATH_INIT;
	int continueRoutine = 1;
	char *pos = NULL, *path = new char[capacity]; // intentional
	struct stat s;
	do
	{
		pos = NULL;
		do
		{
			continueRoutine = 1;
			try
			{
				pos = fgetline(path, capacity, stdin, eol, pos);
				continueRoutine = 0;
			}
			catch(std::bad_alloc &e)
			{
				pos = path + ((pos == NULL) ? capacity : strlen(path));
#ifdef DISK_STORAGE
				solveMemoryProblem(holder, write2Disk, 3);
#else // DISK_STORAGE
				solveMemoryProblem(holder, 3);
#endif // DISK_STORAGE
			}
		}
		while(continueRoutine);

		do
		{
			continueRoutine = 1;
			try
			{
				// Skip file if its unlinkable, non-regular file,
				// to small or to big
				if (lstat(path, &s) < 0 || S_ISREG(s.st_mode) == 0 ||
					s.st_size <= minSize || maxSize && s.st_size > maxSize)
				{
					continueRoutine = 0;
					continue;
				}

				FileGroup &filegroup = holder[s][s];
				if (filegroup != path &&
					(S_ADD_HARDLINKED(flags) || filegroup.isEmpty()))
					filegroup += path;
//				pos = NULL;
				continueRoutine = 0;
			}
			catch(std::bad_alloc &e)
			{
#ifdef DISK_STORAGE
				solveMemoryProblem(holder, write2Disk, 3);
#else // DISK_STORAGE
				solveMemoryProblem(holder, 3);
#endif // DISK_STORAGE
			}
		}
		while(continueRoutine);
	}
	while(pos != NULL);
	delete[] path;
}

static int addingAllowed(const char *path, const FileGroup &obj)
{
	return obj != path && (S_ADD_HARDLINKED(flags) || obj.getSize() == 0);
}

static int deleteEarly(SizeGroup &obj, size_t i, size_t n)
{
	if (S_VERBOSE_LEVEL3(flags))
		if (time(NULL) >= deleteEarly_t)
		{
			// Skip the first time
			if (deleteEarly_t == 0)
			{
				deleteEarly_t = 60 + time(NULL);
				return 1;
			}

			// Print a info line
			fprintf(stderr, "\nInfo: FileSize ");
			fprintsize(stderr, obj.getFileSize());
			fprintf(stderr, "B | %lu%% (%zu/%zu)\n",
				100 * (i + 1) / n, (i + 1), n);

			// Return here in 60s
			deleteEarly_t = 60 + time(NULL);
		}
		else // print a dot
			fprintf(stderr, ".");
	return 1;
}

void compareFiles(MatchMatrix &match, Holder &holder,
	int (&printFileCompare)(const SizeGroup &,
		const FileGroup &, const Filename &,
		const FileGroup &, const Filename &,
		int result),
	int flags,
	int (*preCheck)(const SizeGroup &,
		const FileGroup &, const FileGroup &)
#ifdef DISK_STORAGE
	, Write2Disk &write2Disk
#endif // DISK_STORAGE
	) throw()
{
	int continueRoutine = 1;
	do
	{
		try
		{
			waisted += holder.compareFiles(match, printFileCompare,
				flags, addingAllowed, deleteEarly, preCheck);
			continueRoutine = 0;
		}
		catch(std::bad_alloc &e)
		{
#ifdef DISK_STORAGE
			solveMemoryProblem(holder, write2Disk, 3);
#else // DISK_STORAGE
			solveMemoryProblem(holder, 3);
#endif // DISK_STORAGE
		}
	} while(continueRoutine);
	if (S_VERBOSE_LEVEL3(flags))
		fprintf(stderr, ".\n");
}

Stats processInput(
	int (&printFileCompare)(const SizeGroup &, const FileGroup &,
		const Filename &, const FileGroup &, const Filename &,
		int result),
	int (&printHard)(const char *a, const char *b, nlink_t nlink,
		off_t fileSize, const char *sep),
	int (&selectResults)(int flags, const char *sep),
	int (*preCheck)(const SizeGroup &,
		const FileGroup &, const FileGroup &)) throw()
{
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time0, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY
	size_t oldMinSize = minSize;
	Holder holder;

	// Reserve memory for later (better memory management)
	MatchMatrix match(EXPECTED_MAX_GROUP);

	// Stage1 - reading the input
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "%s: Info: Reading input\n", program);
#ifdef DISK_STORAGE
	Storage storage(program);
	Write2Disk write2Disk(storage);
	readInput(holder, write2Disk);
#else // DISK_STORAGE
	readInput(holder);
#endif // DISK_STORAGE

#ifdef DEBUG
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time1, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY
#endif

	// Stage2 - mandatory sorting
#ifdef DEBUG
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "%s: Info: Sorting\n", program);
#endif
	switch(S_MATCH_MASK(flags))
	{
		case MATCH_LEFT:								// -A
			holder.sort(FileGroup::compareFirst, Filename::compareFirst);
			break;
		case MATCH_RIGHT:								// -Z
			holder.sort(FileGroup::compareLast, Filename::compareLast);
			break;
#ifndef LOW_MEMORY_PROFILE
		case MATCH_LEFT | MATCH_TIME:					// -At
			holder.sort(FileGroup::compareOldest, Filename::compareFirst);
			break;
		case MATCH_RIGHT | MATCH_TIME:					// -Zt
			holder.sort(FileGroup::compareYoungest, Filename::compareLast);
			break;
#endif // LOW_MEMORY_PROFILE
		case MATCH_LEFT | MATCH_RIGHT:					// -L
			holder.sort(FileGroup::compareNlink, Filename::compareFirst);
	}

#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time2, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY

	// Stage3 - print hard linked files
	if (S_REPORT_HARDLINKS(flags))
	{
		if (S_VERBOSE_LEVEL2(flags))
			fprintf(stderr, "%s: Info: Hard linked filenames\n", program);
		PrintHardLinked printHardLinked(printHard, sep);
		holder.accept(printHardLinked);
#ifdef HAVE_GETTIMEOFDAY
		gettimeofday(&time3, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY
	}
	else
		time3 = time2;

	// Get stats
	Stats stats;
	holder.accept(stats);
	registerQuit(stats);

	// Stage4 - checkfiles & print identical
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "%s: Info: Comparing files\n", program);
	compareFiles(match, holder, printFileCompare,
		selectResults(flags, sep),
#ifdef DISK_STORAGE
		preCheck, write2Disk);
#else // DISK_STORAGE
		preCheck);
#endif // DISK_STORAGE
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time4, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY

	if (minSize != oldMinSize && S_VERBOSE_LEVEL1(flags))
	{
		fprintf(stderr,
			"%s: Changed mimimum file size from %zu to %zu\n",
			program, oldMinSize, minSize);
		fprintf(stderr, "%s: Continue with: ... | %s -g %zu -m %zu\n",
			program, program, oldMinSize, minSize);
	}
	return stats;
}

static void fprintStatsStart(const char *str)
{
	fprintf(stderr, "\n%s: %s", program, str);
	for (size_t i = strlen(str); i < 30; ++i)
	   fprintf(stderr, ".");	
	fprintf(stderr, ": ");
}

void processStats(Stats &stats) throw()
{
	if (!S_VERBOSE_LEVEL2(flags))
		return;

	int ndigits = S_HUMAN_READABLE(flags) ? 4 : digits(stats.getTotalSize());
	int (&funcDigits)(ulongest_t) = S_HUMAN_READABLE(flags)
		? digitsHumanReadable : digits;

	fprintf(stderr, "%s: Info: Stats", program);
	fprintStatsStart("Number of i-nodes");
	{
		for(int i = funcDigits(stats.getFiles()); i < ndigits; ++i)
			fprintf(stderr, " ");
		if (S_HUMAN_READABLE(flags))
			fprintsize(stderr, stats.getFiles());
		else
			fprintf(stderr, "%lu", stats.getFiles());
	}

	fprintStatsStart("Number of filenames");
	{
		for(int i = funcDigits(stats.getFilenames()); i < ndigits; ++i)
			fprintf(stderr, " ");
		if (S_HUMAN_READABLE(flags))
			fprintsize(stderr, stats.getFilenames());
		else
			fprintf(stderr, "%lu", stats.getFilenames());
	}

	fprintStatsStart("Largest group of files");
	{
		unsigned long largest = stats.getSizeLargestGroupFiles();
		for(int i = funcDigits(largest); i < ndigits; ++i)
			fprintf(stderr, " ");
		if (S_HUMAN_READABLE(flags))
			fprintsize(stderr, largest);
		else
			fprintf(stderr, "%lu", largest);
	}

	fprintStatsStart("Smallest file size");
	{
		size_t size = stats.getMinFileSize() < stats.getMaxFileSize() ?
			stats.getMinFileSize() : stats.getMaxFileSize();
		for(int i = funcDigits(size); i < ndigits; ++i)
			fprintf(stderr, " ");
		if (S_HUMAN_READABLE(flags))
		{
			fprintsize(stderr, size);
			fprintf(stderr, "B");
		}
		else
			fprintf(stderr, "%lu", size);
	}

	fprintStatsStart("Average file size");
	{
		size_t avg = stats.getFiles() ?
			stats.getTotalSize() / stats.getFiles() : 0;
		for(int i = funcDigits(avg); i < ndigits; ++i)
			fprintf(stderr, " ");
		if (S_HUMAN_READABLE(flags))
		{
			fprintsize(stderr, avg);
			fprintf(stderr, "B");
		}
		else
			fprintf(stderr, "%lu", avg);
	}

	fprintStatsStart("Largest file size");
	{
		for(int i = funcDigits(stats.getMaxFileSize()); i < ndigits; ++i)
			fprintf(stderr, " ");
		if (S_HUMAN_READABLE(flags))
		{
			fprintsize(stderr, stats.getMaxFileSize());
			fprintf(stderr, "B");
		}
		else
			fprintf(stderr, "%lu", stats.getMaxFileSize());
	}

	fprintStatsStart("Grand total file size");
	{
		for(int i = funcDigits(stats.getTotalSize()); i < ndigits; ++i)
			fprintf(stderr, " ");
		if (S_HUMAN_READABLE(flags))
		{
			fprintsize(stderr, stats.getTotalSize());
			fprintf(stderr, "B");
		}
#ifdef __LONG_LONG_SUPPORTED
		else					fprintf(stderr, "%llu", stats.getTotalSize());
#else // __LONG_LONG_SUPPORTED
		else					fprintf(stderr, "%lu", stats.getTotalSize());
#endif // __LONG_LONG_SUPPORTED
	}

	fprintStatsStart("Waisted disk space");
	{
		for(int i = funcDigits(waisted); i < ndigits; ++i)
			fprintf(stderr, " ");
		if (S_HUMAN_READABLE(flags))
		{
			fprintsize(stderr, waisted);
			fprintf(stderr, "B");
		}
		else
		{
#ifdef __LONG_LONG_SUPPORTED
			fprintf(stderr, "%llu", waisted);
#else // __LONG_LONG_SUPPORTED
			fprintf(stderr, "%lu", waisted);
#endif // __LONG_LONG_SUPPORTED
		}
		fprintf(stderr, " (%u%%)", stats.getTotalSize() ?
				100 * waisted / stats.getTotalSize() : 0);
	}

	fprintf(stderr, "\n%s: Execution time:", program);
#ifdef HAVE_GETTIMEOFDAY
#ifdef DEBUG
	if (time1.tv_sec !=  time0.tv_sec || time1.tv_usec !=  time0.tv_usec)
	{
		fprintf(stderr, "\n%s:    reading input............:", program);
		fprinttime(stderr, time1, time0, S_HUMAN_READABLE(flags));
	}
	if (time2.tv_sec !=  time1.tv_sec || time2.tv_usec !=  time1.tv_usec)
	{
		fprintf(stderr, "\n%s:    sorting..................:", program);
		fprinttime(stderr, time2, time1, S_HUMAN_READABLE(flags));
	}
#else
	if (time2.tv_sec !=  time0.tv_sec || time2.tv_usec !=  time0.tv_usec)
	{
		fprintf(stderr, "\n%s:    reading input............:", program);
		fprinttime(stderr, time2, time0, S_HUMAN_READABLE(flags));
	}
#endif
	if (time3.tv_sec !=  time2.tv_sec || time3.tv_usec !=  time2.tv_usec)
	{
		fprintf(stderr, "\n%s:    report hard linked files.:", program);
		fprinttime(stderr, time3, time2, S_HUMAN_READABLE(flags));
	}
	if (time4.tv_sec !=  time3.tv_sec || time4.tv_usec !=  time3.tv_usec)
	{
		fprintf(stderr, "\n%s:    report identical files...:", program);
		fprinttime(stderr, time4, time3, S_HUMAN_READABLE(flags));
	}
	if (time4.tv_sec !=  time0.tv_sec || time4.tv_usec !=  time0.tv_usec)
	{
		fprintf(stderr, "\n%s:                              ", program);
		if (S_HUMAN_READABLE(flags))
			fprintf(stderr, "-----------------+");
		else
			fprintf(stderr, "------------+");
		fprintf(stderr, "\n%s:        Total execution time.:", program);
		fprinttime(stderr, time4, time0, S_HUMAN_READABLE(flags));
		fprintf(stderr, "\n");
	}
	else
		fprintf(stderr, "0s\n");
#endif // HAVE_GETTIMEOFDAY
}

