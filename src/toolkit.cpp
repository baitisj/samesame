
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "toolkit.h"

#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif // HAVE_FCNTL_H
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif // HAVE_LOCALE_H
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif // HAVE_STDINT_H
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif // HAVE_STDLIB_H
#ifdef HAVE_STRING_H
#include <string.h>
#endif // HAVE_STRING_H
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif // HAVE_SYS_STAT_H
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#include <new>

#include "buffer.h"

int fcpy(int dst, int src, size_t size) throw()
{
	if (dst <= 0) return FILE_OPEN1_ERROR;
	if (src <= 0) return FILE_OPEN2_ERROR;
#if defined(HAVE_MMAP) && defined(HAVE_MUNMAP)
	if (USE_MMAP(size, 0))
	{
		void *m2 = mmap(0, size, PROT_READ, MAP_SHARED, src, 0);
		if (m2 != MAP_FAILED)
		{
			posix_madvise(m2, size, POSIX_MADV_WILLNEED | POSIX_MADV_SEQUENTIAL);
			off_t offset = 0;
			while(size >= 0)
			{
				ssize_t count = pwrite(dst, m2, size, offset);
				if (count > 0)
					size -= count, offset += count;
				else
					break;
			}
			posix_madvise(m2, size, POSIX_MADV_NORMAL);
			munmap(m2, size);
			if (size == 0)
				return 0;
		}
	}
#endif // HAVE_MMAP && HAVE_MUNMAP
	Buffer &buffer = Buffer::getDefault(getBufferSize());
	size_t offset = 0, n = buffer.getCapacity();
	ssize_t count;
	do
	{
		if (offset + n > size)
			n = size - offset;
		if ((count = read(src, buffer[0], n)) < 0)
			return FILE_READ2_ERROR;
		if (write(dst, buffer[0], (size_t)count) < 0)
			return FILE_WRITE1_ERROR;
		offset += (size_t)count;
	}
	while(offset < size);
	return 0;
}

int fcpy(const char *dst, const char *src, struct stat &s) throw()
{
#ifdef O_SHLOCK
	int fd = open(dst, O_WRONLY | O_CREAT | O_SHLOCK, s.st_mode);
#else // O_SHLOCK
	int fd = open(dst, O_WRONLY | O_CREAT, s.st_mode);
#endif // O_SHLOCK
	if (fd < 0)
		return FILE_OPEN1_ERROR;
#ifdef O_SHLOCK
	int fs = open(src, O_RDONLY | O_SHLOCK);
#else // O_SHLOCK
	int fs = open(src, O_RDONLY);
#endif // O_SHLOCK
	if (fs < 0)
	{
		close(fd);
		return FILE_OPEN2_ERROR;
	}

	int status = fcpy(fd, fs, s.st_size);
	close(fd);
	close(fs);
	return status;
}

int fcmp(int fd1, int fd2, off_t s1, off_t s2) throw()
{
	if (fd1 <= 0) return FILE_OPEN1_ERROR;
	if (fd2 <= 0) return FILE_OPEN2_ERROR;

	if (s1 != s2) // diffent sizes means different 
		return FILE_DIFFERENT;
	else if (s1 == 0) // identical files if both file sizes are 0
		return FILE_IDENTICAL;

#if defined(HAVE_MMAP) && defined(HAVE_MUNMAP)
	if (USE_MMAP2(s1, 0))
	{
		void *m1 = mmap(0, s1, PROT_READ, MAP_SHARED, fd1, 0);
		if (m1 != MAP_FAILED)
		{
			void *m2 = mmap(0, s2, PROT_READ, MAP_SHARED, fd2, 0);
			if (m1 == MAP_FAILED)
				munmap(m1, s1);
			else
			{
				posix_madvise(m1, s1, POSIX_MADV_WILLNEED | POSIX_MADV_SEQUENTIAL);
				posix_madvise(m2, s2, POSIX_MADV_WILLNEED | POSIX_MADV_SEQUENTIAL);
				if (memcmp(m1, m2, s1))
				{
					posix_madvise(m1, s1, POSIX_MADV_NORMAL);
					posix_madvise(m2, s2, POSIX_MADV_NORMAL);
					munmap(m1, s1);
					munmap(m2, s2);
					return FILE_DIFFERENT;
				}
				posix_madvise(m1, s1, POSIX_MADV_NORMAL);
				posix_madvise(m2, s2, POSIX_MADV_NORMAL);
				munmap(m1, s1);
				munmap(m2, s2);
				return FILE_IDENTICAL;
			}
		}
	}
#endif // HAVE_MMAP && HAVE_MUNMAP
	off_t offset = 0;
	Buffer &buffer = Buffer::getDefault(2 * getBufferSize());
	buffer.setAmountPages(2);
	size_t max = buffer.getPageSize();
	size_t n = 4096 > buffer.getPageSize() ? buffer.getPageSize() : 4096;
	int count;
	do
	{
		if (offset + n > s1)
			n = s1 - offset;
		if ((count = read(fd1, buffer[0], n)) < 0)
			return FILE_READ1_ERROR;
		if (count != read(fd2, buffer[1], (size_t)count))
			return FILE_READ2_ERROR;
		if (memcmp(buffer[0], buffer[1], (size_t)count))
			return FILE_DIFFERENT;
		offset += (size_t)count;
		if (n < max)
		{
			n *= 2;
			if (n > max)
				n = max;
		}
	}
	while(offset < s1);
	return FILE_IDENTICAL;
}

int fcmp(const char *f1, const char *f2, off_t s1, off_t s2) throw()
{
	if (s1 != s2) // diffent sizes means different 
		return FILE_DIFFERENT;
	else if (s1 == 0) // identical files if both file sizes are 0
		return FILE_IDENTICAL;

	// open files f1 and f2
#ifdef O_SHLOCK
	int fd1 = open(f2, O_RDONLY | O_SHLOCK);
#else // O_SHLOCK
	int fd1 = open(f1, O_RDONLY);
#endif // O_SHLOCK
	if (fd1 < 0)
		return FILE_OPEN1_ERROR;
#ifdef O_SHLOCK
	int fd2 = open(f2, O_RDONLY | O_SHLOCK);
#else // O_SHLOCK
	int fd2 = open(f2, O_RDONLY);
#endif // O_SHLOCK
	if (fd2 < 0)
	{
		close(fd1);
		return FILE_OPEN2_ERROR;
	}

	// check file input
	int status = fcmp(fd1, fd2, s1, s2);

	// Files are identical.
	close(fd1);
	close(fd2);
	return status;
}

int fcmp(int fd1, int fd2, struct stat &s1, struct stat &s2) throw()
{
	if (s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino)
		return FILE_IDENTICAL;
	return fcmp(fd1, fd2, s1.st_size, s2.st_size);
}

int fcmp(const char *f1, const char *f2, struct stat &s1, struct stat &s2) throw()
{
	if (s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino)
		return FILE_IDENTICAL;
	return fcmp(f1, f2, s1.st_size, s2.st_size);
}

char *fgetline(char *&str, size_t &size, FILE *file, int eol, char *ptr)
throw (std::bad_alloc)
{
	if (&size == 0)
		return NULL;
	if (ptr == NULL)
		ptr = str;
	int input = getc(file);
	if (input == EOF)
		return NULL;
//	size_t n = 0;
	char *end = str + size - 1;

#ifdef DEBUG
	if (ptr - str >= (int)size)
	{
		fprintf(stderr, "%s:%d ptr to far\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif // DEBUG

	while(input != eol && input != EOF)
	{
#ifdef DEBUG
		if (ptr - str >= (int)size)
		{
			fprintf(stderr, "%s:%d ptr to far\n", __FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
		*ptr = input;
		if (++ptr == end)
		{
			char *tmp = new char[size << 1]; // throws bad_alloc
			memcpy(tmp, str, size);
			delete[] str;
			ptr = (str = tmp) + size - 1;
			end = ptr + size;
			size <<= 1;
		}
		input = getc(file);
	}
	*ptr = 0;
#ifdef DEBUG
	if (ptr - str >= (int)size)
	{
		fprintf(stderr, "%s:%d ptr to far\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	if (strlen(str) >= size)
	{
		fprintf(stderr, "%s:%d string to large length %zu max_size %zu\n",
			__FILE__, __LINE__, strlen(str), size);
		exit(EXIT_FAILURE);
	}
#endif // DEBUG
	return str;
}

int digits(ulongest_t number) throw()
{
	int counter = number ? 0 : 1;
	while(number)
	{
		++counter;
		number /= 10;
	}
	return counter;
}

int getHumanSize(ulongest_t &size)
{
	int i = 0;
	while(size > 9999 && i < 8)
		size >>= 10, ++i;
	return i;
}

int digitsHumanReadable(ulongest_t number) throw()
{
	getHumanSize(number);
	return digits(number);
}

void fprintsize(FILE *out, ulongest_t size) throw()
{
	int i = getHumanSize(size);
#if __LONG_LONG_SUPPORTED__
	fprintf(out, "%;lu", size);
#else // __LONG_LONG_SUPPORTED__
	fprintf(out, "%lu", size);
#endif // __LONG_LONG_SUPPORTED__
	switch(i)
	{
		case 0: fprintf(out, "   ");	break;
		case 1: fprintf(out, " Ki");	break;
		case 2: fprintf(out, " Mi");	break;
		case 3: fprintf(out, " Gi");	break;
		case 4: fprintf(out, " Ti");	break;
		case 5: fprintf(out, " Pi");	break;
		case 6: fprintf(out, " Ei");	break;
		case 7: fprintf(out, " Zi");	break;
		case 8: fprintf(out, " Yi");	break;
		default: fprintf(out, " ?i");
	}
}

static timeval tmpTime;

void fprinttime(FILE *out, struct timeval &after, struct timeval &before,
	int humanReadble) throw()
{
#ifdef HAVE_LOCALECONV
	char decimal_point = localeconv()->decimal_point[0];
#else // HAVE_LOCALECONV
	char decimal_point = '.';
#endif // HAVE_LOCALECONV

	tmpTime.tv_sec = after.tv_sec - before.tv_sec;
	tmpTime.tv_usec = after.tv_usec - before.tv_usec;
	if (tmpTime.tv_usec < 0)
		--tmpTime.tv_sec, tmpTime.tv_usec += 1000000;
	tmpTime.tv_usec /= 10000;
	if (humanReadble)
	{
		size_t days = tmpTime.tv_sec / 86400;
		tmpTime.tv_sec %= 86400;
		size_t hours = tmpTime.tv_sec / 3600;
		tmpTime.tv_sec %= 3600;
		size_t mins = tmpTime.tv_sec / 60;
		tmpTime.tv_sec %= 60;

		if (days)
			fprintf(out, "%4zud%02zuh%02zum%02ld%c%02lds",
				days, hours, mins,
				tmpTime.tv_sec, decimal_point, tmpTime.tv_usec);
		else if (hours)
			fprintf(out, "     %2zuh%02zum%02ld%c%02lds",
				hours, mins,
				tmpTime.tv_sec, decimal_point, tmpTime.tv_usec);
		else if (mins)
			fprintf(out, "        %2zum%02ld%c%02lds",
				mins,
				tmpTime.tv_sec, decimal_point, tmpTime.tv_usec);
		else
			fprintf(out, "           %2ld%c%02lds",
				tmpTime.tv_sec, decimal_point, tmpTime.tv_usec);
	}
	else
		fprintf(out, "%9jd%c%02ld", (intmax_t)tmpTime.tv_sec,
			decimal_point, tmpTime.tv_usec);
}

#ifndef HAVE_STRSTR
#include <sys/cdefs.h>
#ifndef HAVE_STRING_H
// #error include file string.h required
#endif // HAVE_STRING_H
/*
char *strstr(const char *s, const char*find)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != 0)
	{
		len = strlen(find);
		do
		{
			do
			{
				if ((sc = *s++) == 0)
					return (NULL);
			}
			while (sc != c);
		}
		while (strncmp(s, find, len) != 0);
		s--;
	}
	return (char *)s;
}

*/
#endif // HAVE_STRSTR

int createDirectory(const char *path, size_t pathOffset, const char *src)
{
	struct stat s;
	s.st_mode = 0755;
	mode_t oumask = umask(0);
	char *p = (char *)path, *tmp;
	int status = 0;
	size_t srcOffset = strlen(src) - strlen(path + pathOffset);
	for (; *p != 0; ++p)
	{
		if (*p != '/')
			continue;
		if ((size_t)(p - path) >= pathOffset)
		{
			tmp = (char *)src + srcOffset + (p - path) - pathOffset;
			*tmp = 0;
			stat(src, &s);
			*tmp = '/';
		}
		*p = 0;
		if (mkdir(path, s.st_mode) < 0)
		{
			if (errno != EEXIST)
			{
				*p = '/';
				umask(oumask);
				return DIRECTORY_FAILED;
			}
			else if (p[-1] != '/')
				status = DIRECTORY_EXISTED;
		}
		else
			status = DIRECTORY_CREATED;
		*p = '/';
	}
	umask(oumask);
	return status;
}

