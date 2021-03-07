
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_STATS_H
#define AK_STATS_H

#include "configure.h"
#include "visitor.h"

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif // HAVE_LIMITS_H

// class Holder;
// class SizeGroup;
// class FileGroup;
// class Filename;

/**
 * Collects stats from the holder graph
 */
class Stats : public SamefileVisitor
{
	class SizeGroup *largestGroupFiles;
	unsigned long sizeLargestGroupFiles;
	off_t minFileSize, maxFileSize, currentFileSize;
	unsigned long files, filenames; // file : filename = 1 : n
	ulongest_t totalSize;

public:
	Stats() :
		minFileSize(UINT_MAX), maxFileSize(0),
		largestGroupFiles(NULL), sizeLargestGroupFiles(0),
		files(0), filenames(0), totalSize(0)
	{}

	/**
	 * Reset the internal values.
	 */
	Stats &reset()
	{
		totalSize = files = filenames = 0;
		maxFileSize = 0;
		minFileSize = OFF_MAX;
		return *this;
	}

	off_t getMinFileSize() { return minFileSize; }
	off_t getMaxFileSize() { return maxFileSize; }

	unsigned long getFiles() { return files; }
	unsigned long getFilenames() { return filenames; }

	unsigned long getSizeLargestGroupFiles() { return sizeLargestGroupFiles; }
	class SizeGroup *getLargestGroupFiles() { return largestGroupFiles; }

	ulongest_t getTotalSize() { return totalSize; }

	int visit(class Holder &);
	int visit(class SizeGroup &);
	int visit(class FileGroup &);
	void visit(class Filename &);
};

#endif // AK_STATS_H

