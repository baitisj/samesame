
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_FILEGROUP_H
#define AK_FILEGROUP_H

#include "configure.h"
#include "hash.h"
#include "container.h"
#include "list.h"
// #include "storage.h"
// #include "visitor.h"
#include "filename.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif // HAVE_SYS_STAT_H
#ifdef DEBUG
#include <stdio.h>
#endif

#include <new>

class SizeGroup;
class SamefileVisitor;

void deleteFilename(void *ptr);

/**
 * A FileGroup is a group of Filename(s) that share the same device and inode
 * numers.
 * @see Filename, Container
 */
class FileGroup
{
	static WrapperFilename tmp;

public:
	__uint32_t	device;
	ino_t		inode;
	nlink_t		nlink;
#ifndef LOW_MEMORY_PROFILE
	time_t		mtime;
#endif // LOW_MEMORY_PROFILE

private:
	List<Filename> list;

	friend class SizeGroup;

public:
	/**
	 * Assumes a and b are of type FileGroup ** and compares them on the
	 * number of links. If those are equal then the first Filename is 
	 * used for comparison.
	 */
	static int compareNlink(const void *a, const void *b) throw();

	/**
	 * Assumes a and b are of type FileGroup ** and compares them 
	 * alphabetically using the first Filename.
	 * In order for this to work the function sort must be called.
	 * @see sort
	 */
	static int compareFirst(const void *a, const void *b) throw();

	/**
	 * Assumes a and b are of type FileGroup ** and compares them 
	 * reversed alphabetically using the last Filename. 
	 * The function sort must be called in order for this to work.
	 * @see sort
	 */
	static int compareLast(const void *a, const void *b) throw();

#ifndef LOW_MEMORY_PROFILE
	/**
	 * Assumes a and b are of type FileGroup ** and compares them 
	 * chronologically using the modication time.
	 * The function sort must be called in order for this to work.
	 * @see sort
	 */
	static int compareOldest(const void *a, const void *b) throw();

	/**
	 * Assumes a and b are of type FileGroup ** and compares them 
	 * reversed chronologically using the modication time.
	 * The function sort must be called in order for this to work.
	 * @see sort
	 */
	static int compareYoungest(const void *a, const void *b) throw();
#endif // LOW_MEMORY_PROFILE

	/**
	 * Creates a FileGroup object without a preset shared metadata.
	 */
	FileGroup() throw (std::bad_alloc);

	/**
	 * Creates a FileGroup object with a preset shared meta data.
	 * @param s - shared meta data 
	 * @param capacity - the initial capacity of the group.
	 */
	FileGroup(const struct stat &s, size_t capacity = 1)
	throw (std::bad_alloc);

	~FileGroup() throw() { list.deleteItems(&deleteFilename); }

	/**
	 *
	 */
	void empty() throw() { list.empty(1); }

#ifdef DEBUG
//	size_t getCapacity() const throw() { return 0; }
#endif
	/**
	 * Return the number of Filenames within this group.
	 */
	size_t getSize() const throw() { return list.getSize(); }
	int isEmpty() const throw() { return list.isEmpty(); }

        /**
         * Gets the boundry for the mode of the container.
         */   
        size_t getBoundry() const throw() { return list.getSize(); }

	/**
	 * Creates a new iterator object to the list and transfers
	 * ownership.
	 */
	ListIterator<Filename> *createIterator() const
	{ return new ListIterator<Filename>(list);}

	/**
	 * Implements the visitor pattern.
	 */
	void accept(SamefileVisitor &v);

	/**
	 * Checks if both files are on the same device.
	 */
	int isOnSameDevice(const FileGroup &obj) const throw()
	{ return device == obj.device; }

	/**
	 * Sort all the Filename on alphabethical order.
	 */
	void sort(int (&compare)(const void *a, const void *b)) throw()
	{ list.sort(compare); };

	/**
	 * Open the file using any of Filename(s) within this group.
	 */
	int open(int flags) const throw();

	/**
	 * Compares if this FileGroup has the same contence of the given
	 * FileGroup.
	 * @returns see ::fcmp()
	 */
	int fcmp(const FileGroup &obj, off_t fileSize) const throw();

	/**
	 * Is this the same dev, inode pair?
	 */
	int operator==(const FileGroup &obj) const throw()
	{ return inode == obj.inode && device == obj.device; }

	/**
	 * Is this the same dev, inode pair?
	 */
	int operator!=(const FileGroup &obj) const throw()
	{ return inode != obj.inode || device != obj.device; }

	/**
	 * Doesn't str live in this object?
	 */
	int operator!=(const char *str) const throw (std::bad_alloc);

	/**
	 * Does str live in this object?
	 */
	int operator==(const char *str) const throw (std::bad_alloc)
	{ return !operator!=(str); }

	/**
	 * Sets the meta data of this FileGroup to s.
	 */
	void operator=(const struct stat &s) throw()
	{ inode = s.st_ino, device = s.st_dev; }

	/**
	 * Adds a Filename to the FileGroup.
	 */
	void operator+=(const char *path) throw (std::bad_alloc);
};

#endif // AK_FILEGROUP_H

