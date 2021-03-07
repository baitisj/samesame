
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_WRITE2DISK_H
#define AK_WRITE2DISK_H

#include "configure.h"

#ifdef DISK_STORAGE

#include "storage.h"

class Holder;
class SizeGroup;
class FileGroup;
class Filename;

/**
 * Writes paths to disk using a storage object.
 */
class Write2Disk : public SamefileVisitor
{
	Storage &storage;
	unsigned long counter;
	size_t max, fileSize;

public:
	Write2Disk(Storage &storage) : storage(storage) {};

	/**
	 * Return the number of lines written or read.
	 */
	size_t done() { return counter; }

	/**
	 * Reset the internal variables and set the maximum file size to
	 * write to disk.
	 */
	Write2Disk &reset(size_t max)
	{ counter = 0, this->max = max; return *this; }

	int visit(Holder &r);
	int visit(SizeGroup &);
	int visit(FileGroup &);
	void visit(Filename &);
};

#endif // DISK_STORAGE
#endif // AK_WRITE2DISK_H

