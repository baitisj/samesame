
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include "mainAction.h"

#if defined(BSD)
#include <sys/sysctl.h>
#include <sys/types.h>
#endif // BSD
#if defined(HAVE_SYS_TIME_H) && defined(HAVE_GETTIMEOFDAY)
#include <sys/time.h>
#endif // HAVE_SYS_TIME_H && HAVE_GETTIMEOFDAY
#include <err.h>
#include <errno.h>

// Retrieved from actionProcessOptions
static char *program = NULL;
static const char *sep = "\t";
static off_t minSize = 0, maxSize = OFF_MAX;
static int stopping = 0;
unsigned flags = VERBOSE_LEVEL1 | MATCH_LEFT;
static FILE *pipeLeafs = NULL, *outputLeafs = NULL;

// Retrieved from actionProcessInput
static size_t processed = 0, untouched = 0, lines = 0;
static ulongest_t bytesSaved = 0;
static struct timeval time0, time1;

// Extended apps store there errno in this variable
int same_errno = 0;

static void ignore(int sig) { }

static void quit(int sig)
{
	stopping = 1;
}

size_t getPath(char *&path, size_t pathOffset, size_t capacity,
		const char *src, size_t srcOffset)
{
	size_t srcLen = strlen(src);
	if (pathOffset + srcLen - srcOffset >= capacity)
	{
		while(pathOffset + srcLen - srcOffset >= capacity)
			capacity <<= 1;
		char *tmp = new char[capacity];
		memcpy(tmp, path, pathOffset);
		delete[] path;
		path = tmp;
	}
	if (srcLen >= srcOffset + 2 &&
		src[srcOffset] == '.' && src[srcOffset + 1] == '/')
		++srcOffset;
	if (path[strlen(path) - 1] == '/' && src[srcOffset] == '/')
		++srcOffset; 
	memcpy(path + pathOffset, src + srcOffset, srcLen - srcOffset);
	return capacity;
}

size_t getParameter(char *&str, char *param, size_t capacity)
{
	size_t len = strlen(str), paral = strlen(param);
	if (len + paral + 1 >= capacity)
	{
		while(len + paral + 1 >= capacity)
			capacity <<= 1;
		char *tmp = new char[capacity];
		memcpy(tmp, str, len);
		tmp[len] = 0;
		delete[] str;
		str = tmp;
	}
	memcpy(str + len, param, paral + 1);
	return capacity;
}

void actionUsage(const char *command) throw()
{
	fprintf(stderr, "\n%s reads the samefile output and links", program); 
	fprintf(stderr, "\nidentical files together.\n");
	fprintf(stderr, "\nusage: %s [-A  |  -At  |  -L  |  -Z | -Zt] [-g size] \\", program);
	size_t len = strlen(program);
	fprintf(stderr, "\n        ");
	for (size_t i = 0; i < len; ++i)
		fprintf(stderr, " ");
	fprintf(stderr, "[-m size] [-S sep] [-o file | -p command] [-HnqstVvw]\n");

	fprintf(stderr, "example: find <dir> | samefile | %s\n", program);
	fprintf(stderr, "\n");
	fprintf(stderr, "\n  options: -A %s based on the first filename", command);

	fprintf(stderr, "\n           -d work on the directory instead of the file (experimental)");
	fprintf(stderr, "\n           -g only process files greater than size (0)");
	fprintf(stderr, "\n           -H human readable statistics");
	fprintf(stderr, "\n           -L %s based on the number of links (default)", command);
	fprintf(stderr, "\n           -m only process files less or equal than size (0)");
	fprintf(stderr, "\n           -n perform a trial run with no changes made");
	fprintf(stderr, "\n           -o output consumed match lines to this file");
	fprintf(stderr, "\n           -p pipe consumed match lines to to this program");
	fprintf(stderr, "\n           -q suppress non-error messages");
	fprintf(stderr, "\n           -S use sep as separator string for files (tab)");
	fprintf(stderr, "\n           -s symlink files if its not posible to hard the files");
	fprintf(stderr, "\n           -t match based on the modiciation time instead of the path");
	fprintf(stderr, "\n           -V output version information and exit");
	fprintf(stderr, "\n           -v increase verbosity");
	fprintf(stderr, "\n           -w don't check files contence");
	fprintf(stderr, "\n           -Z %s based on the last filename", command);
	fprintf(stderr, "\n");
	exit(EXIT_SUCCESS);
}

int actionProcessOptions(int argc, char **argv, const char *command) throw()
{
#ifdef SIGALRM
	signal(SIGALRM, quit);
#endif // SIGALRM
#ifdef SIGINT
	signal(SIGINT, quit);
#endif // SIGINT
#ifdef SIGPIPE
	signal(SIGPIPE, ignore);
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
#endif // SIGXFSZ
	(program = rindex(argv[0], '/')) ? ++program : program = argv[0];

	int c;
	setlinebuf(stdout);
	while ((c = getopt(argc, argv, "h?g:m:p:o:S:dHALZtisnqVvw")) != -1)
		switch(c)
		{
			default: case 'h': case '?': actionUsage(command);				break;

			case 'g':
				if (sscanf(optarg, "%ju", &minSize) != 1)
					minSize = 0,
							warn("can't convert -g %s, using -g 0 instead\n",
									optarg);
				break;
			case 'm':
				if (sscanf(optarg, "%ju", &maxSize) !=  1)
					maxSize = 0,
							warn("can't convert -m %s, using-g 0 instead\n",
									optarg);
				break;
			case 'p':
				if (pipeLeafs)
					pclose(pipeLeafs);
//				char *p = new char[strlen(optarg) + 1];
//				strcpy(p, optarg);
				pipeLeafs = popen(optarg, "w");
				setlinebuf(pipeLeafs);
				if (pipeLeafs < 0)
					warn("coudn't open pipe to: %s\n", optarg);
				else if (outputLeafs)
				{
					fclose(outputLeafs);
					outputLeafs = NULL;
				}
				break;
			case 'o':
				if (outputLeafs)
					fclose(outputLeafs);
				outputLeafs = fopen(optarg, "w");
				setlinebuf(outputLeafs);
				if (outputLeafs < 0)
					warn("open file: %s\n", optarg);
				else if (pipeLeafs)
				{
					pclose(pipeLeafs);
					pipeLeafs = NULL;
				}
				break;

			case 'S': sep = optarg;										break;
			case 'H': flags |= HUMAN_READABLE;							break;

			case 'A': flags |=  MATCH_LEFT;		flags &= ~MATCH_RIGHT;	break;
			case 'L': flags |=  MATCH_LEFT;		flags |=  MATCH_RIGHT;  break;
			case 'Z': flags &= ~MATCH_LEFT;		flags |=  MATCH_RIGHT;	break;
			case 't': flags |= MATCH_TIME;								break;

			case 'd': flags |= DIRECTORY;								break;
			case 's': flags |= SYMLINK;									break;
			case 'n': flags |= DRYRUN;									break;

			case 'q': flags &= ~VERBOSE_MASK;							break;
			case 'V':
				printf(COPYRIGHT, PACKAGE_STRING, program);
				exit(EXIT_SUCCESS);
			break;
			case 'v': if ((S_VERBOSE(flags)) < VERBOSE_MAX) ++flags;	break;

			case 'w': flags |= WITHOUT_FILE_CHECK;						break;
		}
	return optind;
}

static void actionProcessInputSkip(const char *line) throw()
{
	printf("%s\n", line);
}

static int (*func)(const char *f1, const char *f2,
		const struct stat &s1, const struct stat &s2,
		const char *dst, const char *src) = NULL;
static void (*skip)(const char *) = NULL;

static int printLeafs(const char *line) throw()
{
	if (pipeLeafs)
	{
		if (strlen(line) != fwrite(line, 1, strlen(line), pipeLeafs))
		{
			fclose(outputLeafs);
			outputLeafs = NULL;
			return FAILED_PIPE_OPTION;
		}
		fwrite("\n", 1, 1, pipeLeafs);
	}
	if (outputLeafs)
	{
		if (strlen(line) != fwrite(line, 1, strlen(line), outputLeafs))
		{
			fclose(outputLeafs);
			outputLeafs = NULL;
			return FAILED_FILE_OPTION;
		}
		fwrite("\n", 1, 1, outputLeafs);
	}
	return 0;
}

static int actionProcessInputAction(Cache &cache, match_t &m,
		const char *line, const char *f1, const char *f2) throw()
{
	// compare if files are realy equal.
	if (!S_WITHOUT_FILE_CHECK(flags))
	{
		if (cache != m) // check f1 with f2
		{
			m.result = fcmp(f1, f2, m.s1, m.s2);
			cache += m;
		}
		switch(cache[m].result)
		{
			case FILE_IDENTICAL:
				break;
			case FILE_OPEN1_ERROR:
				fprintf(stderr, "Coudn't open file %s\n", f1);
				break;
			case FILE_OPEN2_ERROR:
				fprintf(stderr, "Coudn't open file %s\n", f2);
				break;
			case FILE_READ1_ERROR:
				fprintf(stderr, "Coudn't read file %s\n", f1);
				break;
			case FILE_READ2_ERROR:
				fprintf(stderr, "Coudn't read file %s\n", f2);
				break;
#ifdef DEBUG
			case FILE_UNKOWN:
				fprintf(stderr, "DEBUG: file unknown %s %s\n", f1, f2);
				break;
			case FILE_DIFFERENT:
				fprintf(stderr, "DEBUG: files where diffrent %s %s\n", f1, f2);
				break;
			default:
				fprintf(stderr, "DEBUG: something else %s %s\n", f1, f2);
#endif
		}
		switch(cache[m].result)
		{
			case FILE_OPEN1_ERROR:
			case FILE_OPEN2_ERROR:
			case FILE_READ1_ERROR:
			case FILE_READ2_ERROR:
			case FILE_UNKOWN:
			case FILE_DIFFERENT:
			default:
				++untouched;
				return 1;
			case FILE_IDENTICAL:
				break;
		}
	}

	const char *dst, *src;
	switch(S_MATCH(flags))
	{
		default:
		case MATCH_LEFT:						// -A
			if (strcmp(f1, f2) < 0)				dst = f1, src = f2;
			else								dst = f2, src = f1;
			break;
		case MATCH_LEFT | MATCH_TIME:			// -At
			if (m.s1.st_mtime < m.s2.st_mtime)	dst = f1, src = f2;
			else								dst = f2, src = f1;
			break;
		case MATCH_RIGHT:						// -Z
			if (strcmp(f1, f2) < 0)				dst = f2, src = f1;
			else								dst = f1, src = f2;
			break;
		case MATCH_RIGHT | MATCH_TIME:			// -Zt
			if (m.s1.st_mtime < m.s2.st_mtime)	dst = f2, src = f1;
			else								dst = f1, src = f2;
			break;
		case MATCH_LEFT | MATCH_RIGHT:			// -L
			if (m.s1.st_nlink <= m.s2.st_nlink)	dst = f2, src = f1;
			else								dst = f1, src = f2;
			break;
	}

	if (S_DIRECTORY(flags))
	{
		char *ptr = (char *)rindex(f1, '/');
		if (ptr == NULL)
		{
			if (skip != NULL)
				skip(line);
#ifdef DEBUG
			fprintf(stderr, "Skipping 1...\n");
#endif
			++untouched;
			return 1;
		}
		*ptr = 0;
		ptr = (char *)rindex(f2, '/');
		if (ptr == NULL)
		{
			if (skip != NULL)
				skip(line);
#ifdef DEBUG
			fprintf(stderr, "Skipping 2...\n");
#endif
			++untouched;
			return 1;
		}
		*ptr = 0;
	}

	int result;
	switch(result = func(f1, f2, m.s1, m.s2, dst, src))
	{
		case SUCCES_LEFT:
			if (m.s1.st_nlink == 1)
				bytesSaved += m.s1.st_size;
			result = printLeafs(line);
			++processed;
		break;

		case SUCCES_RIGHT:
			if (m.s2.st_nlink == 1)
				bytesSaved += m.s2.st_size;
			result = printLeafs(line);
			++processed;
		break;

		case DISMISS_SILENTLY:
			result = printLeafs(line);
			++processed;
		break;

		default:
			if (skip != NULL)
				skip(line);
			++untouched;
	}

	if (S_VERBOSE_LEVEL1(flags))
	{
		errno = same_errno;
		switch(result)
		{
			case FAILED_FILE_OPTION:
				warn("coun't write to file, dropping option -o");
				break;
			case FAILED_PIPE_OPTION:
				warn("coun't write to pipe, dropping option -p");
				break;
			case FAILED_BACKUP_CREATE:
				warn("failed to create a backup for %s",  f1);
				break;
			case FAILED_BACKUP_DELETE:
				warn("failed to delete a backup for %s",  f1);
				break;
			case FAILED_REMOVE_LEFT:
				warn("failed to remove %s",  f1);
				break;
			case FAILED_REMOVE_RIGHT:
				warn("failed to remove %s",  f2);
				break;
			case FAILED_LINK_LEFT:
				warn("failed to link %s -> %s",  f1, f2);
				break;
			case FAILED_LINK_RIGHT:
				warn("failed to link %s -> %s",  f2, f1);
				break;
		}
		same_errno = 0;
	}
	return 0;
}

#if defined(BSD)
static size_t getSystemData(int &adjustOption) throw()
{
	size_t miblen = sizeof(int);
#if defined(__OpenBSD__)
	int mib[2], int max, inuse;
	mib[0] = CTL_KERN;
	mib[1] = KERN_MAXVNODES;
	sysctl(mib, 2, &max, &miblen, NULL, 0);
	mib[1] = KERN_NUMVNODES;
	sysctl(mib, 2, &inuse, &miblen, NULL, 0);
	adjustOption = 5 * (max - inuse)/max;
	return max - inuse;
#else // FreeBSD based?
	int free, want;
	sysctlbyname("vfs.freevnodes", &free, &miblen, NULL, 0);
	sysctlbyname("vfs.wantfreevnodes", &want, &miblen, NULL, 0);
	adjustOption = 5 * free / want;
	return free;
#endif
}
#endif // BSD

void actionProcessInput(void (&skip)(const char *line),
		int (&action)(Cache &c, match_t &m,
			const char *line, const char *f1, const char *f2))
{
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time0, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY
	int len = strlen(sep);
	size_t fs = PATH_MAX, capacity = PATH_MAX;
	char *line = new char[capacity];
	size_t size = 0;
	match_t m;
	Cache cache;
#if defined(BSD)
	int gotoSleep, adjustOption;
	size_t current, previous = 0;
#endif // BSD
	char *f1 = new char[fs], *f2 = new char[fs];
	::skip = skip;

	while(fgetline(line, capacity, stdin) != NULL)
	{
#if defined(BSD)
		current = getSystemData(adjustOption);
		if (lines >= 32)
		{
				// calculate how much time we need to sleep
				if (adjustOption >= 4 || previous >= current)	gotoSleep = 0;
				else if (gotoSleep == 0)						gotoSleep = 1;
				else	gotoSleep = (previous - current) / gotoSleep;
				if (gotoSleep < 0)								gotoSleep = 1;

				// adjust the time we will go to sleep
				switch(adjustOption)
				{
					case 3: gotoSleep *=  75; break;
					case 2: gotoSleep *=  90; break;
					case 1: gotoSleep *= 110; break;
					case 0: gotoSleep *= 125; break;
				}
				gotoSleep /= 100;

			 	// goto sleep until time run out or resources are free
				for (size_t i = 0; i < (size_t)gotoSleep / 60; ++i)
				{
					getSystemData(adjustOption);
					if (adjustOption >= 4)
						gotoSleep = 0;
					else
						sleep(60);
				}
				sleep(gotoSleep % 60);
		}
		previous = current;
#endif // BSD

		++lines;
		if (stopping)
			break;

		// Are f1 and f2 to small?
		if (strlen(line) > fs + 21)
		{
			delete[] f1;
			delete[] f2;
			while(strlen(line) > fs)
				fs <<= 1;
			f1 = new char[fs];
			f2 = new char[fs];
		}

		// read input
		if (inputSamefile(line, size, f1, f2, sep, len))
		{
			if (S_VERBOSE_LEVEL1(flags))
				warn("coudn't decode line: %s\n", line);
			skip(line);
			++untouched;
			continue;
		}

		// get meta data
		if (lstat(f1, &m.s1) < 0)
		{
			if (S_VERBOSE_LEVEL1(flags))
				warn("coudn't fetch meta data for: %s\n", f1);
			skip(line);
			++untouched;
			continue;
		}
		if (lstat(f2, &m.s2) < 0)
		{
			if (S_VERBOSE_LEVEL1(flags))
				warn("coudn't fetch meta data for: %s\n", f2);
			skip(line);
			++untouched;
			continue;
		}

		// check the file size
		if (m.s1.st_size <= minSize || m.s1.st_size > maxSize || 
		    m.s2.st_size <= minSize || m.s2.st_size > maxSize)
		{
			if (S_VERBOSE_LEVEL1(flags))
				warn("files don't have the same size: %s\n", line);
			skip(line);
			++untouched;
			continue;
		}

		if (action(cache, m, line, f1, f2))
			continue;
	}
	::skip = NULL;
	delete[] line;
	delete[] f1;
	delete[] f2;
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time1, (struct timezone *)NULL);
#endif // HAVE_GETTIMEOFDAY

	// Close stuff we opened in actionProcessOptions
	if (pipeLeafs)
		pclose(pipeLeafs);
	if (outputLeafs)
		fclose(outputLeafs);
}

void actionProcessInput(int (&func)(const char *f1, const char *f2,
		const struct stat &s1, const struct stat &s2,
		const char *dst, const char *src))
{
	::func = func;
	actionProcessInput(actionProcessInputSkip, actionProcessInputAction);
	::func = NULL;
}

void actionProcessStats() throw()
{
	if (!S_VERBOSE_LEVEL3(flags))
		return;
	fprintf(stderr, "\nProcessed files: %u", (unsigned)processed);
	fprintf(stderr, "\nUntouched files: %u", (unsigned)untouched);
#ifdef __LONG_LONG_SUPPORTED
	fprintf(stderr, "\nBytes saved....: %llu", bytesSaved);
#else // __LONG_LONG_SUPPORTED
	fprintf(stderr, "\nBytes saved....: %lu", bytesSaved);
#endif // __LONG_LONG_SUPPORTED
	fprintf(stderr, "\nExecution time:");
#ifdef HAVE_GETTIMEOFDAY
	fprinttime(stderr, time1, time0, S_HUMAN_READABLE(flags));
#endif // HAVE_GETTIMEOFDAY
	fprintf(stderr, "\n");
}

