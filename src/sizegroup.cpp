
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "hash.h"
#include "toolkit.h"
#include "visitor.h"
#include "storage.h"
#include "filegroup.h"
#include "matchmatrix.h"
#include "sizegroup.h"
#include "list.h"
#include "buffer.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#if defined(BSD)
#include <sys/sysctl.h>
#endif // BSD
#include <sys/uio.h>
#include <unistd.h>

#ifdef DEBUG
#if defined(HAVE_SYS_TIME_H) && defined(HAVE_GETTIMEOFDAY)
#include <sys/time.h>
#endif // HAVE_SYS_TIME_H && HAVE_GETTIMEOFDAY
#endif // DEBUG

#include <new>

#ifndef FILE_BY_LOGIC
#define FILE_BY_LOGIC FILE_USER1
#endif

/* If READ_ONCES is defined the code will read each file only ones.
 * If READ_ONCES is not defined the code will use less memory.
 */

FileGroup SizeGroup::tmp;

#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
timeval timeAvoidDiskAccess;
#ifdef CHECKSUM
timeval timeChecksum;
#endif // CHECKSUM
#ifdef PREREAD
timeval timePreread;
#endif // PREREAD
#ifdef READ_ONCES
timeval timeReadOnlyOnces;
#endif // READ_ONCES
timeval timeProcessResults, timeA, timeB;

struct SizeGroupTime
{
	SizeGroupTime()
	{
		timeAvoidDiskAccess.tv_sec = 0, timeAvoidDiskAccess.tv_usec = 0;
#ifdef CHECKSUM
		timeChecksum.tv_sec = 0, timeChecksum.tv_usec = 0;
#endif // CHECKSUM
#ifdef PREREAD
		timePreread.tv_sec = 0, timePreread.tv_usec = 0;
#endif // PREREAD
#ifdef READ_ONCES
		timeReadOnlyOnces.tv_sec = 0, timeReadOnlyOnces.tv_usec = 0;
#endif // READ_ONCES
		timeProcessResults.tv_sec = 0, timeProcessResults.tv_usec = 0;
	}

	~SizeGroupTime()
	{
		fprintf(stderr, "%s:%d %u seconds spend in avoidDiskAccess()\n",
			__FILE__, __LINE__, timeAvoidDiskAccess.tv_sec);
#ifdef CHECKSUM
		fprintf(stderr, "%s:%d %u seconds spend in checksum()\n",
			__FILE__, __LINE__, timeChecksum.tv_sec);
#endif // CHECKSUM
#ifdef PREREAD
		fprintf(stderr, "%s:%d %u seconds spend in preread()\n",
			__FILE__, __LINE__, timePreread.tv_sec);
#endif // PREREAD
#ifdef READ_ONCES
		fprintf(stderr, "%s:%d %u seconds spend in readOnlyOnces()\n",
			__FILE__, __LINE__, timeReadOnlyOnces.tv_sec);
#endif // READ_ONCES
		fprintf(stderr, "%s:%d %u seconds spend in processResults()\n",
			__FILE__, __LINE__, timeProcessResults.tv_sec);
	}
} sizeGroupTime;
#endif // DEBUG

int SizeGroup::compare(const void *a, const void *b) throw()
{
	return (**(SizeGroup **)b).fileSize - (**(SizeGroup **)a).fileSize;
}

/* ************************************************************************ */

hash_t SizeGroup::hashFunction(const SizeGroup &obj) throw()
{
	return hashword((hash_t *)&obj.fileSize, sizeof(off_t) / sizeof(hash_t));
}

FileGroup &SizeGroup::operator[](const struct stat &key) throw (std::bad_alloc)
{
	tmp = key;
	if (hash[tmp] != NULL)
		return *hash[tmp];
	FileGroup *ptr = new FileGroup(key); // throws bad_alloc
	try
	{
		hash += *ptr; // throws bad_alloc
		return *ptr;
	}
	catch(std::bad_alloc &e)
	{
		delete ptr;
		throw(e);
	}
}

void SizeGroup::accept(SamefileVisitor &v)
{
	if (v.visit(*this))
		return;
	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
		if (hash[i] != NULL)
			hash[i]->accept(v);
}

void SizeGroup::sort(int (&compare)(const void *a, const void *b),
	int (&compareFilename)(const void *a, const void *b)) throw()
{
	size_t n = hash.getSize();
	hash.convert(CONTAINER_VECTOR);
#ifdef DEBUG
	for (size_t i = 0; i < n; ++i)
		if (hash[i]->getSize() == 0)
		{
			fprintf(stderr, "%s:%d FileGroup child doesn't have any File(s)\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
	for (size_t i = 0; i < n; ++i)
		hash[i]->sort(compareFilename);
#ifdef DEBUG
	for (size_t i = 0; i < n; ++i)
		if (hash[i]->getSize() == 0)
		{
			fprintf(stderr, "%s:%d FileGroup child doesn't have any File(s)\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
	hash.sort(compare);
}

#ifdef DISK_STORAGE
size_t SizeGroup::diskWrite(Storage &storage) throw()
{
	accept(storage);
	this->storage = &storage;

	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
		if (hash[i] != NULL)
			hash[i]->empty();
	return storage.close();
}

int SizeGroup::diskRead(
	int (&addingAllowed)(const char *, const FileGroup &))
	throw (std::bad_alloc)
{
	if (storage == NULL)
		return 0;
	if (storage->open(fileSize) < 0)
		return -1;
	const char *path;
	while((path = storage->read(tmp.device, tmp.inode)) != NULL)
		if (addingAllowed(path, *hash[tmp]))
			*hash[tmp] += path; // throws bad_alloc
	size_t counter = storage->close();
	storage->clean();
	return counter;
}
#endif // DISK_STORAGE

/* ************************************************************************ */

void SizeGroup::avoidDiskAccess(MatchMatrix &match,
	int (*preCheck)(const SizeGroup &, const FileGroup &, const FileGroup &)
	) const throw()
{
	size_t n = hash.getSize();
	for (size_t i = 0; i < n; ++i)
		for (size_t j = i + 1; j < n; ++j)
			if (preCheck != NULL && preCheck(*this, *hash[i], *hash[j]))
			{
				match.set(i, j, FILE_DIFFERENT);
#ifdef READ_ONCES
				match.increaseEqual(i);
				match.increaseEqual(j);
#endif // READ_ONCES
			}
}

#ifdef CHECKSUM
void SizeGroup::checksum(MatchMatrix &match) const throw()
{
	int fd;
	size_t n = hash.getSize();
	Buffer &buffer = Buffer::getDefault(n * CHECKSUM_LEN);
	buffer.setAmountPages(n);
	for (size_t i = 0; i < n; ++i)
	{
#ifdef DEBUG
		if (hash[i] == NULL) {
			fprintf(stderr, "%s:%d filegroup doesn't exist\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
		if (!hash[i]->getSize())
		{
			fprintf(stderr, "%s:%d filegroup is empty\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
		if ((fd = hash[i]->open(O_RDONLY)) < 0)
		{
			for (size_t j = 0; j < n; ++j)
			{
				if (j == i)	continue;
				if (j < i)	match.set(j, i, FILE_OPEN1_ERROR);
				else		match.set(i, j, FILE_OPEN2_ERROR);
#ifdef READ_ONCES
				match.increaseEqual(j);
#endif // READ_ONCES
			}
			continue;
		}
		if (read(fd, buffer[i], CHECKSUM_LEN) < 0)
			for (size_t j = 0; j < n; ++j)
			{
				if (j == i)	continue;
				if (j < i)	match.set(j, i, FILE_READ1_ERROR);
				else		match.set(i, j, FILE_READ2_ERROR);
#ifdef READ_ONCES
				match.increaseEqual(j);
#endif // READ_ONCES
			}
		close(fd);
	}
/*
	for (size_t i = 0; i < n; ++i)
		for (size_t j = i + 1; j < n; ++j)
			if (memcmp(buffer[i], buffer[j], CHECKSUM_LEN))
			{
				match.set(i, j, FILE_DIFFERENT);
#ifdef READ_ONCES
				match.increaseEqual(i);
				match.increaseEqual(j);
#endif // READ_ONCES
			}
*/
}
#endif // CHECKSUM

#ifdef PREREAD
void SizeGroup::preread(MatchMatrix &match) const throw()
{
	int fd;
	ssize_t count;
        Buffer &buffer = Buffer::getDefault(getBufferSize());
	size_t offset, n = hash.getSize(), size = buffer.getCapacity();
	for (size_t i = 0; i < n; ++i)
	{
#ifdef DEBUG
		if (hash[i] == NULL) {
			fprintf(stderr, "%s:%d filegroup doesn't exist\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
		if (!hash[i]->getSize())
		{
			fprintf(stderr, "%s:%d filegroup is empty\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
		if (match.get(i, n - 1) < 0)
			continue;
#ifdef READ_ONCES
		if (match.getEqual(i) >= n - 1)
			continue;
#endif // READ_ONCES
		if ((fd = hash[i]->open(O_RDONLY)) < 0)
		{
			for (size_t j = 0; j < n; ++j)
			{
				if (j == i)	continue;
				if (j < i)	match.set(j, i, FILE_OPEN1_ERROR);
				else		match.set(i, j, FILE_OPEN2_ERROR);
#ifdef READ_ONCES
				match.increaseEqual(j);
#endif // READ_ONCES
			}
			continue;
		}
		for (offset = 0; offset < fileSize; )
		{
			if (offset + size > fileSize)
				size = fileSize - offset;
			if ((count = read(fd, buffer[0], size)) < 0)
			{
				for (size_t j = 0; j < n; ++j)
				{
					if (j == i)	continue;
					if (j < i)	match.set(j, i, FILE_READ1_ERROR);
					else		match.set(i, j, FILE_READ2_ERROR);
#ifdef READ_ONCES
					match.increaseEqual(j);
#endif // READ_ONCES
				}
				break;
			}
			offset += (size_t)count;
		}
		close(fd);
	}
}
#endif // PREREAD

#ifdef READ_ONCES
size_t SizeGroup::maxfiles() const throw()
{
#if defined(BSD)
	size_t len = sizeof(int);
	int maxfiles, openfiles, maxfilesperproc = 0;

#if defined(OpenBSD)
	int mib[2];
	mib[0] = CTL_KERN;
	mib[1] = KERN_MAXFILES;
	sysctl(mib, 2, &maxfiles, &len, NULL, 0);
	mib[1] = KERN_NFILES
	sysctl(mib, 2, &openfiles, &len, NULL, 0);
#else
	sysctlbyname("kern.maxfiles", &maxfiles, &len, NULL, 0);
	sysctlbyname("kern.openfiles", &openfiles, &len, NULL, 0);
	sysctlbyname("kern.maxfilesperproc", &maxfilesperproc, &len, NULL, 0);
#endif
	maxfiles = -16 + maxfilesperproc > 0 &&
			maxfilesperproc < (maxfiles - openfiles) / 2 ?
		maxfilesperproc : (maxfiles - openfiles) / 2;
	if ((size_t)maxfiles < hash.getSize())
		return maxfiles;
#endif // BSD
	return hash.getSize();
}

size_t SizeGroup::readOnlyOncesOpen(MatchMatrix &match, size_t max)
const throw()
{
	int fd;
	size_t n = hash.getSize();
	for (size_t i = 0; i < max; ++i)
	{
#ifdef DEBUG
		if (hash[i] == NULL) {
			fprintf(stderr, "%s:%d filegroup doesn't exist\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
		if (!hash[i]->getSize())
		{
			fprintf(stderr, "%s:%d filegroup is empty\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
		if (match.get(i, n - 1) < 0 || match.getEqual(i) >= n - 1)
			continue;
		if (match.setFd(i, fd = hash[i]->open(O_RDONLY)) < 0 &&
			(errno  == EMFILE || errno == ENFILE))
		{
			// close 32 file descriptors just to be on the safe side.
			if (i > 32 && max > i - 32)
				max = i - 32;
			else
				max = i / 2;
			if (i)
				for (--i; i >= max && i; --i)
					if (match.getFd(i) > 0)
						close(match.getFd(i));
			return max;
		}
		if (fd < 0)
		{
				for (size_t j = 0; j < n; ++j)
				{
					if (j == i)	continue;
					if (j < i)	match.set(i, n - 1, FILE_OPEN1_ERROR);
					else		match.set(j, n - 1, FILE_OPEN2_ERROR);
#ifdef READ_ONCES
					match.increaseEqual(j);
#endif // READ_ONCES
				}
				continue;
		}
	}
	return max;
}

void SizeGroup::readOnlyOncesCompare(MatchMatrix &match, size_t max)
const throw(std::bad_alloc)
{
	size_t n = hash.getSize();
        Buffer &buffer = Buffer::getDefault(n * getBufferSize());
	buffer.setAmountPages(n);
	size_t pageSize = buffer.getPageSize();
	off_t size = 4096 > buffer.getPageSize() ? buffer.getPageSize() : 4096;
	int fd;
	for (off_t offset = 0; offset < fileSize; offset += size)
	{
		if (offset + size > fileSize)
			size = fileSize - offset;

		// read pages
		for (size_t i = 0; i < n; ++i)
		{
			if (match.get(i, n - 1) < 0 || match.getEqual(i) >= n - 1)
				continue;

			fd = match.getFd(i);
			if (fd < 0)
				continue;

			// open file and search for position
			if (i > max)
			{
#ifdef DEBUG
				if (hash[i] == NULL) {
					fprintf(stderr, "%s:%d filegroup doesn't exist\n",
						__FILE__, __LINE__);
					exit(EXIT_FAILURE);
				}
				if (!hash[i]->getSize())
				{
					fprintf(stderr, "%s:%d filegroup is empty\n",
						__FILE__, __LINE__);
					exit(EXIT_FAILURE);
				}
#endif // DEBUG
				if ((fd = hash[i]->open(O_RDONLY)) < 0 &&
					(errno  == EMFILE || errno == ENFILE))
				{
					close(match.getFd(--max));
					fd = hash[i]->open(O_RDONLY);
				}
				if (fd < 0)
				{
					for (size_t j = 0; j < n; ++j)
					{
						if (j == i)	continue;
						if (j < i)	match.set(j, i, FILE_OPEN1_ERROR);
						else		match.set(i, j, FILE_OPEN2_ERROR);
						match.increaseEqual(j);
					}
					continue;
				}
				match.setFd(i, fd);
				if (fd < 0)
					continue;
				lseek(fd, offset, SEEK_SET);
			}
				
			// read the data
			if (read(fd, buffer[i], size) < 0)
				for (size_t j = 0; j < n; ++j)
				{
					if (j == i)	continue;
					if (j < i)	match.set(j, i, FILE_READ1_ERROR);
					else		match.set(i, j, FILE_READ2_ERROR);
					match.increaseEqual(j);
				}

			// close file
			if (i > (size_t) max)
				close(fd);
		}

		// compare pages
		for (size_t i = 0; i < n; ++i)
		{
			if (match.get(i, n - 1) < 0 || match.getEqual(i) >= n - 1)
				continue;
			for (size_t j = i + 1; j < n; ++j)
			{
				if (match.getEqual(j) == n - 1 || (match.get(j, n - 1) < 0 ||
					match.get(i, j) == FILE_DIFFERENT))
					continue;
				if (memcmp(buffer[i], buffer[j], size) != 0)
				{
					match.set(i, j, FILE_DIFFERENT);
					match.increaseEqual(i);
					match.increaseEqual(j);
				}
			}
		}

		// switch to a more effecient io
		if (size < pageSize)
		{
			size *= 2;
			if (size > pageSize) 
				size = pageSize;
		}
	}
}

void SizeGroup::readOnlyOncesClose(MatchMatrix &match, size_t max)
const throw()
{
	// close files
	for (size_t i = 0; i < max; ++i)
		if (match.getFd(i) > 0)
			close(match.getFd(i));
}

size_t SizeGroup::readOnlyOncesMark(MatchMatrix &match) const throw()
{
	size_t n = hash.getSize(), nIdenticals = 0;

	// mark identicals
	for (size_t i = 0; i < n; ++i)
		for (size_t j = i + 1; j < n; ++j)
			if (match.get(i, j) == 0)
			{
				++nIdenticals;
				match.set(i, j, FILE_IDENTICAL);
			}

	// mark found by logic
	int result;
	for (size_t i = 0; i < n; ++i)
		for (size_t j = i + 1; j < n; ++j)
		{
			result = match.get(i, j);
			// (0, 1), (0, 2) (1, 2) i = 1 j =2
			for (size_t k = i + 1; k < j; ++k)
				switch(result | match.get(i, k))
				{
					case FILE_IDENTICAL:
						--nIdenticals;
						match.set(k, j, FILE_IDENTICAL | FILE_BY_LOGIC);
					break;

					case FILE_IDENTICAL | FILE_DIFFERENT:
						match.set(k, j, FILE_DIFFERENT | FILE_BY_LOGIC);
					break;
				}
		}
	return nIdenticals;
}

size_t SizeGroup::readOnlyOnces(MatchMatrix &match) const throw(std::bad_alloc)
{
	size_t max = maxfiles();
	max = readOnlyOncesOpen(match, max);
	readOnlyOncesCompare(match, max);
	readOnlyOncesClose(match, max);
	return readOnlyOncesMark(match);
}
#endif // READ_ONCES

size_t SizeGroup::processResults(MatchMatrix &match,
	int (&f)(const SizeGroup &, const FileGroup &, const Filename &, 
		const FileGroup &, const Filename &, int),
	int flags) const throw()
{
	// Compare the files.
	int result;
	size_t nIdenticals = 0, n = hash.getSize();
	for (size_t i = 0; i < n; ++i)
	{
#ifdef DEBUG
		if (hash[i] == NULL)
		{
			fprintf(stderr, "%s:%d left filegroup doesn't exist\n",
				__FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
		for (size_t j = i + 1; j < n; ++j)
		{
#ifdef DEBUG
			if (hash[j] == NULL)
			{
				fprintf(stderr, "%s:%d right filegroup doesn't exist\n",
					__FILE__, __LINE__);
				exit(EXIT_FAILURE);
			}
#endif // DEBUG

			// do something with the result.
			result = match.get(i, j);
			FileGroup &left = *hash[i], &right = *hash[j];
			Iterator<Filename> &leftIterator = *left.createIterator();
			Iterator<Filename> &rightIterator = *right.createIterator();
#ifdef DEBUG
			if (!left.getSize())
			{
				fprintf(stderr, "%s:%d left filegroup is empty\n",
					__FILE__, __LINE__);
				exit(EXIT_FAILURE);
			}
			if (!right.getSize())
			{
				fprintf(stderr, "%s:%d right filegroup is empty\n",
					__FILE__, __LINE__);
				exit(EXIT_FAILURE);
			}
#endif // DEBUG

#ifndef READ_ONCES
			// Do a physically check if we couldn't use logic
			if (!result)
			{
				result = left.fcmp(right);
				if (result > 0 && result & FILE_IDENTICAL & ~FILE_BY_LOGIC)
					++nIdenticals;
				else if (result < 0)
					f(*this, left, *leftIterator.getItem(),
						right, *rightIterator.getItem(), result);
				match.set(i, j, result);
			}
#endif // READ_ONCES

			switch(result)
			{
				case FILE_IDENTICAL | FILE_BY_LOGIC:
				case FILE_DIFFERENT | FILE_BY_LOGIC:
				case FILE_IDENTICAL:
				case FILE_DIFFERENT:
				{
					if ((flags & FILE_BY_LOGIC) && (result & FILE_BY_LOGIC) ||
						!(result & FILE_BY_LOGIC))
					{
						int skip = 0;
						for(; !leftIterator.end(); ++leftIterator)
						{
							if (leftIterator.getItem() == NULL)
								continue;
							for (rightIterator.reset(); !rightIterator.end(); ++rightIterator)
								if (rightIterator.getItem() != NULL &&
									f(*this, left, *leftIterator.getItem(),
									right, *rightIterator.getItem(), result))
									skip = 1;
							if (skip == 1)
								break;
						}
					}
#ifndef READ_ONCES
					// a==b (00) a==c (01) => b==c (10)  | rowSize = 10
					// a!=d (02) => b!=d (12) c!=d (22)
					for (size_t k = i + 1; k <= j; ++k)
						if (j != k)
							switch(result | match.get(i, k))
							{
								case FILE_IDENTICAL:
									match.set(k, j,
											FILE_IDENTICAL | FILE_BY_LOGIC);
									break;
								case FILE_IDENTICAL | FILE_DIFFERENT:
									match.set(k, j,
											FILE_DIFFERENT | FILE_BY_LOGIC);
									break;
							}
#endif // READ_ONCES
				}
				break;

#ifndef READ_ONCES
				case FILE_OPEN1_ERROR:
				case FILE_READ1_ERROR:
					j = n; 
					break;

				case FILE_OPEN2_ERROR:
				case FILE_READ2_ERROR:
					for (size_t k = i + 1; k < j; ++k)
						match.set(k, j, result);
					break;
#endif // READ_ONCES
			}

			delete &leftIterator;
			delete &rightIterator;
		}
	}
	return nIdenticals;
}

size_t SizeGroup::compareFiles(MatchMatrix &match,
	int (&f)(const SizeGroup &, const FileGroup &, const Filename &, 
		const FileGroup &, const Filename &, int),
	int flags,
	int (*preCheck)(const SizeGroup &, const FileGroup &, const FileGroup &)
	) throw (std::bad_alloc)
{
	size_t n = hash.getSize();
	if (n < 2)
		return 0;
	match.enlargeCapacity(n),
		hash.convert(CONTAINER_VECTOR); // Make sure the container is a vector

	/* If READ_ONCES is defined the code will read each file only ones.
	 * If READ_ONCES is not defined the code will use less memory.
	 */
#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&timeA, (struct timezone *)NULL);
#endif
	avoidDiskAccess(match, preCheck);
#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&timeB, (struct timezone *)NULL);
	timeAvoidDiskAccess.tv_sec += timeB.tv_sec - timeA.tv_sec,
	timeAvoidDiskAccess.tv_usec += timeB.tv_usec - timeA.tv_usec;
	if (timeAvoidDiskAccess.tv_usec < 0)
		--timeAvoidDiskAccess.tv_sec, timeAvoidDiskAccess.tv_usec += 1000000;
#endif

	// Checksum
#ifdef CHECKSUM
#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&timeA, (struct timezone *)NULL);
#endif
	checksum(match);
#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&timeB, (struct timezone *)NULL);
	timeChecksum.tv_sec += timeB.tv_sec - timeA.tv_sec,
	timeChecksum.tv_usec += timeB.tv_usec - timeA.tv_usec;
	if (timeChecksum.tv_usec < 0)
		--timeChecksum.tv_sec, timeChecksum.tv_usec += 1000000;
#endif
#endif // CHECKSUM

	// Preread
#ifdef PREREAD
#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&timeA, (struct timezone *)NULL);
#endif
	preread(match);
#ifdef DEBUG
	gettimeofday(&timeB, (struct timezone *)NULL);
	timePreread.tv_sec += timeB.tv_sec - timeA.tv_sec,
	timePreread.tv_usec += timeB.tv_usec - timeA.tv_usec;
	if (timePreread.tv_usec < 0)
		--timePreread.tv_sec, timePreread.tv_usec += 1000000;
#endif // DEBUG
#endif // PREREAD

	// Read onces
	size_t nIdenticals = 0;
#ifdef READ_ONCES
#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&timeA, (struct timezone *)NULL);
#endif
	nIdenticals = readOnlyOnces(match);
#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&timeB, (struct timezone *)NULL);
	timeReadOnlyOnces.tv_sec += timeB.tv_sec - timeA.tv_sec,
	timeReadOnlyOnces.tv_usec += timeB.tv_usec - timeA.tv_usec;
	if (timeReadOnlyOnces.tv_usec < 0)
		--timeReadOnlyOnces.tv_sec, timeReadOnlyOnces.tv_usec += 1000000;
#endif
#endif // READ_ONCES

	// Process results
#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&timeA, (struct timezone *)NULL);
#endif // DEBUG
	nIdenticals += processResults(match, f, flags);
#if defined(DEBUG) && defined(HAVE_GETTIMEOFDAY)
	gettimeofday(&timeB, (struct timezone *)NULL);
	timeProcessResults.tv_sec += timeB.tv_sec - timeA.tv_sec,
	timeProcessResults.tv_usec += timeB.tv_usec - timeA.tv_usec;
	if (timeProcessResults.tv_usec < 0)
		--timeProcessResults.tv_sec, timeProcessResults.tv_usec += 1000000;
#endif

	match.reset(n);
	return nIdenticals;
}

