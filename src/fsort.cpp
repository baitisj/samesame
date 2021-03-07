
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "toolkit.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define CHUNK_SIZE	102462
#define VERBOSE_LEVEL1	1
#define VERBOSE_LEVEL2	2
#define VERBOSE_LEVEL3	3
#define VERBOSE_MAX		3
#define VERBOSE_MASK	3
#define HUMAN_READABLE	32

#define S_VERBOSE(m)		((m) & VERBOSE_MASK)
#define S_VERBOSE_LEVEL1(m)	(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL1)
#define S_VERBOSE_LEVEL2(m)	(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL2)
#define S_VERBOSE_LEVEL3(m)	(((m) & VERBOSE_MASK) >= VERBOSE_LEVEL3)
#define S_HUMAN_READABLE(m)	((m) & HUMAN_READABLE)

static char *program;
static FILE *outputFile = NULL;
static size_t lines = 0, flags = VERBOSE_LEVEL1;
static struct timeval time0, time1, time2, time3;

struct entry
{
	off_t	size;	// file size, in bytes
	__dev_t	dev;	// inode's device
	ino_t	ino;	// inode's number

	char *path() const throw() { return ((char *)this) + sizeof(entry); }
};

static int entry_cmp(const void *a, const void *b) throw()
{
	entry *A = *(entry **)a;
	entry *B = *(entry **)b;
	if (A->size != B->size)	return A->size - B->size;
	if (A->dev != B->dev)	return A->dev - B->dev;
	if (A->ino != B->ino)	return A->ino - B->ino;
	if (int ret = strcmp(B->path(), A->path()))	return ret;
	return (char *)a - (char *)b;
}

static void actionUsage(const char *command)
{
	fprintf(stderr, "\n%s reads paths from stdin and sorts them", program);
	fprintf(stderr, "\nusage: %s [-o file] [-Hqv]", program);
	fprintf(stderr, "\n           -H human readable statistics");
	fprintf(stderr, "\n           -o output sorted result to file and unsorted to stdout");
	fprintf(stderr, "\n           -q suppress non-error messages");
	fprintf(stderr, "\n           -v increase verbosity");
	fprintf(stderr, "\n");
	exit(EXIT_SUCCESS);
}

static int processOptions(int argc, char **argv, const char *command)
{
	(program = rindex(argv[0], '/')) ? ++program : program = argv[0];
	int c;
	setlinebuf(stdout);
	while ((c = getopt(argc, argv, "h?o:qv")) != -1)
		switch(c)
		{
			default: case 'h': case '?': actionUsage(command); 			break;
			case 'H': flags |= HUMAN_READABLE;							break;
			case 'o':
				if (outputFile)
					fclose(outputFile);
				outputFile = fopen(optarg, "w");
				break;
			case 'q': flags &= ~VERBOSE_MASK;                           break;
			case 'v': if ((S_VERBOSE(flags)) < VERBOSE_MAX) ++flags;	break;
		}
	return optind;
}

void processInput()
{
	struct stat s;
	size_t len, lineCapacity = 1024, arrCapacity = 32*1024,
		chunkLeft = CHUNK_SIZE;
	entry **arr = new entry *[arrCapacity];
	char *line = new char[lineCapacity],
		*current = (char *)malloc(CHUNK_SIZE);
	if (outputFile)
		setlinebuf(stdout);

	// read paths from stdin
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "info: reading output\n");
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time0, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY
	while(fgetline(line, lineCapacity, stdin) != NULL)
	{
		if (lstat(line, &s) < 0)
		{
			printf("%s\n", line);
			continue;
		}
		len = strlen(line) + 1;
		if (lines == arrCapacity)
		{
			entry **tmp = new entry *[arrCapacity << 1];
			memcpy(tmp, arr, arrCapacity * sizeof(entry *));
			delete arr;
			arr = tmp;
			arrCapacity <<= 1;
		}
		if (len + sizeof(entry) > chunkLeft)
			chunkLeft = CHUNK_SIZE,
			current = (char *)malloc(CHUNK_SIZE);
		if (current == NULL || len + sizeof(entry) > chunkLeft)
			if ((current = (char *)malloc(len + sizeof(entry))) == NULL)
				exit(-1);
		arr[lines] = (entry *)current,
			current += len + sizeof(entry),
			chunkLeft -= len + sizeof(entry);
		memcpy(arr[lines]->path(), line, len);
		arr[lines]->size = s.st_size,
			arr[lines]->dev = s.st_dev,
			arr[lines]->ino = s.st_ino;
		++lines;
		if (outputFile)
			printf("%s\n", line);
	}

	// sort
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "info: sorting\n");
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time1, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY
	qsort(arr, lines, sizeof(entry *), entry_cmp);

	// print paths to stdout
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "info: print result\n");
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time2, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY
	while(lines)
		if (outputFile)
			fprintf(outputFile, "%s\n", arr[--lines]->path());
		else
			printf("%s\n", arr[--lines]->path());

#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time3, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY
}

void processStats()
{
	if (!S_VERBOSE_LEVEL3(flags))
		return;
	fprintf(stderr, "info: stats");
	fprintf(stderr, "\nprocessed lines: %u", lines);

	fprinttime(stderr, time1, time0, S_HUMAN_READABLE(flags));
#ifdef HAVE_GETTIMEOFDAY
	if (time1.tv_sec !=  time0.tv_sec || time1.tv_usec !=  time0.tv_usec)
	{
		fprintf(stderr, "\n    reading input.:");
		fprinttime(stderr, time1, time0, S_HUMAN_READABLE(flags));
	}
	if (time2.tv_sec !=  time1.tv_sec || time2.tv_usec !=  time1.tv_usec)
	{
		fprintf(stderr, "\n    sorting.......:");
		fprinttime(stderr, time2, time1, S_HUMAN_READABLE(flags));
	}
	if (time3.tv_sec !=  time2.tv_sec || time3.tv_usec !=  time2.tv_usec)   
	{
		fprintf(stderr, "\n    writing result:");
		fprinttime(stderr, time3, time2, S_HUMAN_READABLE(flags));
	}
	if (time3.tv_sec !=  time0.tv_sec || time3.tv_usec !=  time0.tv_usec)
	{
		fprintf(stderr, "\n                   ");
		if (!S_HUMAN_READABLE(flags))
			fprintf(stderr, "----");
		fprintf(stderr, "-------+");
		fprintf(stderr, "\n        Total execution time:");
		fprinttime(stderr, time3, time0, S_HUMAN_READABLE(flags));
		fprintf(stderr, "\n");
	}
	else
		fprintf(stderr, "0s\n");
#endif // HAVE_GETTIMEOFDAY
}

int main(int argc, char **argv)
{
	processOptions(argc, argv, "filesort");
	processInput();
	processStats();
}

