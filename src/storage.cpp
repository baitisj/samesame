
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "toolkit.h"
#include "filename.h"
#include "filegroup.h"
#include "storage.h"

#include <new>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif // HAVE_FCNTL_H
#include <fts.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif // HAVE_SYS_TYPES_H
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif // HAVE_SYS_STAT_H
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

/* ************************************************************************ */

void removeDirectoryContence(char *storageFile)
{
	// Remove any regular files that may exist in the directory
	char *const paths[2] = {storageFile, NULL};
	FTS *ftsp = fts_open(paths, FTS_PHYSICAL, NULL);
	FTSENT *ftsent;
	while((ftsent = fts_read(ftsp)) != NULL)
		if (ftsent->fts_info & FTS_F)
			unlink(ftsent->fts_accpath);
	fts_close(ftsp);
}

Storage::Storage(const char *program) throw (std::bad_alloc)
: stream(NULL), device(0), inode(0)
{
	// Initial saving space
	int error;
	pid_t storagePid = getpid();
	storageBase = strlen(TEMP_STORAGE_DIR);
	storageBase += strlen(program);
	storageBase += digits(storagePid);
	storageBase += 2;
	storageFile = new char[storageBase + 11]; // throws bad_alloc
	try
	{
		line = new char[lineCapacity = PATH_INIT]; // throws bad_alloc
	}
	catch(const std::bad_alloc &e)
	{
		delete[] storageFile;
		throw(e);
	}

	// Populating storageFile
	sprintf(storageFile, "%s/%s", TEMP_STORAGE_DIR, program);
#ifdef HAVE_MKDIR
	mkdir(storageFile, S_IRWXU | S_IRWXG | S_IRWXO);
#else // HAVE_MKDIR
#error mkdir function required, see file config.h.in
#endif // HAVE_MKDIR
	if ((error = chmod(storageFile, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		return;
	sprintf(storageFile + strlen(storageFile), "/%u", storagePid);
#ifdef HAVE_MKDIR
	if (mkdir(storageFile, S_IRWXU | S_IRWXG | S_IRWXO))
		removeDirectoryContence(storageFile);
#else // HAVE_MKDIR
#error mkdir function required, see file config.h.in
#endif // HAVE_MKDIR
	if ((error = chmod(storageFile, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		return;
	sprintf(storageFile + strlen(storageFile), "/");
	storageBase = strlen(storageFile);
}

Storage::~Storage() throw()
{
	storageFile[storageBase] = 0;
	removeDirectoryContence(storageFile);
#ifdef HAVE_RMDIR
	rmdir(storageFile);
#else // HAVE_RMDIR
#error rmdir function required, see file config.h.in
#endif // HAVE_RMDIR
	delete[] storageFile;
	delete[] line;
}

int Storage::open(size_t filesize) throw()
{
	size_t n = digits(filesize);
	char *end = storageFile + storageBase;
	char *ptr = end + n;
#ifdef DEBUG
	if (ptr >= storageFile + storageBase + 11)
	{
		fprintf(stderr, "%s:%d pointer out of range\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif

	*ptr = 0;
	while(end != ptr)
	{
		*--ptr = filesize % 10;
		filesize /= 10;
	}
	if (stream != NULL)
		close();
	else
		counter = 0;
	stream = fopen(storageFile, "r");
	if (stream == NULL)
		return -1;
	return 0;
}

const char *Storage::read(__uint16_t device, ino_t inode) throw (std::bad_alloc)
{
	fgetline(line, lineCapacity, stream); // throws bad_alloc

	if (sscanf(line, "%hu", &device) == EOF)
		return NULL;
	char *str = strstr(line, " ");

	if (sscanf(++str, "%zu", &inode) == EOF)
		return NULL;
	str = strstr(str, " ");
	++counter;
	return ++str;
}

size_t Storage::close() throw()
{
	fclose(stream);
	stream = NULL;
	return counter;
}

void Storage::clean() const throw()
{
	unlink(storageFile);
}

int Storage::visit(Holder &holder) { return 0; }

int Storage::visit(SizeGroup &sizegroup)
{
	sprintf(storageFile + storageBase, "%ju", sizegroup.getFileSize());
	if (stream != NULL)
		close();
	else
		counter = 0;
	stream = fopen(storageFile, "a");
	return 0;
}

int Storage::visit(FileGroup &filegroup)
{
	device = filegroup.device;
	inode = filegroup.inode;
	return 0;
}

void Storage::visit(Filename &filename)
{
	fprintf(stream, "%u %zu %s\n", device, inode, filename.data());
	++counter;
}

