
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_PRINTHARDLINKED_H
#define AK_PRINTHARDLINKED_H

#include "configure.h"
#include "visitor.h"

class Holder;
class SizeGroup;
class FileGroup;
class Filename;

/**
 * Prints all the filenames that are hard linked.
 */
class PrintHardLinked : public SamefileVisitor
{
	int (&func)(const char *a, const char *b, nlink_t nlink,
		off_t fileSize, const char *sep);
	const char *sep;
	off_t fileSize;

public:
	PrintHardLinked(int (&func)(const char *a, const char *b,
		nlink_t nlink, off_t fileSize, const char *sep),
		const char *sep)
		: func(func), sep(sep), fileSize(0)
	{}

	int visit(Holder &);
	int visit(SizeGroup &);
	int visit(FileGroup &);
	void visit(Filename &);
};

#endif // AK_PRINTHARDLINKED_H

