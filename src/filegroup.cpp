
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
#include "filename.h"
#include "filegroup.h"

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif // HAVE_FCNTL_H
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#include <new>

size_t tmpCapacity = 256;
WrapperFilename FileGroup::tmp(tmpCapacity);

void deleteFilename(void *ptr)
{
	delete[] (char *)ptr;
}

int FileGroup::compareNlink(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
	if (fb.nlink - fa.nlink)
		return fb.nlink - fa.nlink;
	if (fa.device - fb.device)
		return fa.device - fb.device;
	return fa.inode - fb.inode;
}

int FileGroup::compareFirst(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
#ifdef DEBUG
	if (fa.list.first() == NULL || fb.list.first() == NULL)
	{
		fprintf(stderr, "%s:%d can not compare NULL\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	return fa.list.first()->compare(*fb.list.first());
}

int FileGroup::compareLast(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
#ifdef DEBUG
	if (fa.list.first() == NULL || fb.list.first() == NULL)
	{
		fprintf(stderr, "%s:%d can not compare NULL\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	return fb.list.first()->compare(*fa.list.first());
}

#ifndef LOW_MEMORY_PROFILE
int FileGroup::compareOldest(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
	if (fa.mtime - fb.mtime)
		return fa.mtime - fb.mtime;
	if (fa.device - fb.device)
		return fa.device - fb.device;
	return fa.inode - fb.inode;
}

int FileGroup::compareYoungest(const void *a, const void *b) throw()
{
	FileGroup &fa = **(FileGroup **)a;
	FileGroup &fb = **(FileGroup **)b;
	if (fb.mtime - fa.mtime)
		return fb.mtime - fa.mtime;
	if (fa.device - fb.device)
		return fa.device - fb.device;
	return fa.inode - fb.inode;
}
#endif // LOW_MEMORY_PROFILE

/* ************************************************************************ */

FileGroup::FileGroup() throw (std::bad_alloc)
{
}

FileGroup::FileGroup(const struct stat &s, size_t capacity)
throw (std::bad_alloc)
{
	device	= s.st_dev;
	inode	= s.st_ino;
	nlink	= s.st_nlink;
#ifndef LOW_MEMORY_PROFILE
	mtime	= s.st_mtime;
#endif // LOW_MEMORY_PROFILE
}

void FileGroup::accept(SamefileVisitor &v)
{
	if (v.visit(*this))
		return;
	List<Filename> *l = &list;
	while(l != NULL)
	{
		v.visit(*l->first());
		l = l->next();
	}
}

int FileGroup::open(int flags) const throw()
{
	const List<Filename> *l = &list;
	while(l != NULL)
	{
		int fd = ::open(l->first()->data(), flags);
		if (fd > 0)
			return fd;
		l = l->next();
	}
	return -1;
}

int FileGroup::fcmp(const FileGroup &obj, off_t fileSize) const throw()
{
	// Open the two files
	int fd1 = open(O_RDONLY);
	if (fd1 < 0)
		return FILE_OPEN1_ERROR;

	int fd2 = obj.open(O_RDONLY);
	if (fd2 < 0)
	{
		close(fd1);
		return FILE_OPEN2_ERROR;
	}

	struct stat s1, s2;
	int status = (fstat(fd1, &s2) < 0 || fstat(fd1, &s2) < 0)
		? ::fcmp(fd1, fd2, fileSize, fileSize)
		: ::fcmp(fd1, fd2, s1, s2);

	close(fd1);
	close(fd2);
	return status;
}

int FileGroup::operator!=(const char *path) const throw (std::bad_alloc)
{
	size_t len = strlen(path);
	if (tmpCapacity < len)
	{
		while(tmpCapacity < len)
			tmpCapacity <<= 1;
		tmp.renew(tmpCapacity);
	}
	tmp = path;
	return list != tmp.getFilename();
}

void FileGroup::operator+=(const char *path) throw (std::bad_alloc)
{
	size_t len = strlen(path);
	if (!len)
		return;
	char *ptr = new char[++len];
	memcpy(ptr, path, len);
	try
	{
		list += (Filename *)ptr; // throws bad_alloc
	}
	catch(std::bad_alloc &e)
	{
		delete ptr;
		throw(e);
	}
}

