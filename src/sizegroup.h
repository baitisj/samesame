
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_SIZEGROUP_H
#define AK_SIZEGROUP_H

#include "configure.h"
#include "hash.h"
#include "container.h"
// #include "filename.h"
// #include "filegroup.h"
// #include "matchmatrix.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif // HAVE_STDLIB_H
#include <sys/stat.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif // HAVE_STRING_H

#include <new>

class Filename;
class FileGroup;
class MatchMatrix;
class SamefileVisitor;
class Storage;

#define FILE_BY_LOGIC		FILE_USER1

/**
 * A SizeGroup is a group of files (FileGroup(s)) that share the same file size.
 */
class SizeGroup
{
private:
	static FileGroup tmp;

	off_t fileSize;
	Container<FileGroup> hash;
#ifdef DISK_STORAGE
	Storage *storage;
#endif // DISK_STORAGE

public:
	static hash_t hashFunction(const SizeGroup &obj) throw();

	/**
	 * The function compares two SizeGroup objects for reversed
	 * cronologicly order based on the file size.
	 */
	static int compare(const void *a, const void *b) throw();

	/**
	 * Creates a SizeGroup object without a preset shared file size.
	 */
	SizeGroup() throw (std::bad_alloc)
	{ fileSize = 0;
#ifdef DISK_STORAGE
		storage = NULL;
#endif // DISK_STORAGE
	}

	/**
	 * Creates a SizeGroup object with a preset shared file size.
	 * @param s - meta data with the size.
	 * @param capacity - the initial capacity of the group.
	 */
	SizeGroup(const struct stat &s, size_t capacity = 1)
	throw (std::bad_alloc)
#ifdef DISK_STORAGE
	{ fileSize = s.st_size; storage = NULL; }
#else // DISK_STORAGE
	{ fileSize = s.st_size; }
#endif // DISK_STORAGE

 	~SizeGroup() throw() { hash.deleteItems(); }

	/**
	 * Returns the number of FileGroup objects that are in this object.
	 */
	size_t getSize() const throw() { return hash.getSize(); }

	/**
	 * Returns the file size each file within this object have in common.
	 */
	off_t getFileSize() const throw() { return fileSize; }

#ifdef DISK_STORAGE
	/**
	 * Writes all the FileGroup(s) on to the disk.
	 * @return the number of paths written to the disk or -1 on failure.
	 */
	size_t diskWrite(Storage &storage) throw();

	/**
	 * Read all the FileGroup(s) from the disk.
	 * @return the number of paths read to the disk or -1 on failure.
	 */
	int diskRead(int (&addingAllowed)(const char *, const FileGroup &))
	throw (std::bad_alloc);
#endif // DISK_STORAGE

	/**
	 * Is the given SizeGroup the same as this one?
	 */ 
	int operator==(const SizeGroup &obj) const throw()
	{ return fileSize == obj.fileSize; }

	/**
	 * Is the given SizeGroup different as this one?
	 */ 
	int operator!=(const SizeGroup &obj) const throw()
	{ return fileSize != obj.fileSize; }

	/**
	 * Sets the file size
	 * @see tmp
	 */ 
	void operator=(const struct stat &s) throw()
	{ fileSize = s.st_size; }

	/**
	 * Selects the FileGroup for the given key. It is created if it 
	 * doesn't exist jet.
	 */
	FileGroup &operator[](const struct stat &key) throw (std::bad_alloc);

	/**
	 * Sorts all the FileGroup objects in order depinding on the function
	 * compare.
	 */
	void sort(int (&compare)(const void *a, const void *b),
		int (&compareFilename)(const void *a, const void *b)) throw();

	/**
	 * Implements the visitor pattern.
	 */
	void accept(SamefileVisitor &v);


private:
	void avoidDiskAccess(MatchMatrix &match,
		int (*preCheck)(const SizeGroup &, const FileGroup &, const FileGroup &)
		) const throw();

#ifdef CHECKSUM
	void checksum(MatchMatrix &match) const throw();
#endif // CHECKSUM
#ifdef PREREAD
	void preread(MatchMatrix &match) const throw();
#endif // PREREAD

#ifdef READ_ONCES
	size_t maxfiles() const throw();
	size_t readOnlyOncesOpen(MatchMatrix &match, size_t max) const throw();
	void readOnlyOncesCompare(MatchMatrix &match, size_t max) const throw(std::bad_alloc);
	void readOnlyOncesClose(MatchMatrix &match, size_t max) const throw();
	size_t readOnlyOncesMark(MatchMatrix &match) const throw();
	size_t readOnlyOnces(MatchMatrix &match) const throw(std::bad_alloc);
#endif // READ_ONCES

	size_t processResults(MatchMatrix &match,
		int (&f)(const SizeGroup &, const FileGroup &, const Filename &,
			const FileGroup &, const Filename &, int), 
		int flags) const throw();

public:
	/**
	 * Compares each FileGroup against each other and passes the result to
	 * the given function
	 *
	 * @param match - the match matrix where we keep the temp results.
	 * @param f - the function that will be called for each match. When
	 *            it returns true the internal loop is finished, but the
	 *            external loop is skipped. (i.e. when two FileGroups match
	 *            and the first  call returns true, then the result would
	 *            be 1 : n instead of m : n.)
	 * @param flags	- the match result must have at least on bit in flags
	 *                set that is not the FILE_BY_LOGIC flag.
	 * @param preCheck - Checks whether or not this combination can be
	 *                   skipped based on the information that is know at
	 *                   this time. The combination is checks when this
	 *                   function is not set or it returns true.
	 * @returns the amount of identical files
	 */
	size_t compareFiles(MatchMatrix &match,
		int (&f)(const SizeGroup &, const FileGroup &, const Filename &,
			const FileGroup &, const Filename &, int), 
		int flags,
		int (*preCheck)(const SizeGroup &, 
			const FileGroup &, const FileGroup &) = NULL
		) throw (std::bad_alloc);
};

#endif // AK_SIZEGROUP_H

