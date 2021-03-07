
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
#include "sizegroup.h"
#include "holder.h"
#include "printhardlinked.h"

int PrintHardLinked::visit(Holder &holder) {return 0; }
int PrintHardLinked::visit(SizeGroup &sizegroup)
{
	fileSize = sizegroup.getFileSize();
	return 0;
}

int PrintHardLinked::visit(FileGroup &filegroup)
{
	Iterator<Filename> &a = *filegroup.createIterator();
	Iterator<Filename> &b = *filegroup.createIterator();
	int skip = 0;
	for(; !a.end(); ++a) if (a.getItem() != NULL)
	{
		b = a;
		++b;
		for(; !b.end(); ++b) if (b.getItem() != NULL)
			if (func(a.getItem()->data(), b.getItem()->data(),
				filegroup.nlink, fileSize, sep))
				skip = 1;
		if (skip)
			break;
	}
	delete &a;
	delete &b;
	return 1;
}

void PrintHardLinked::visit(Filename &filename) {}

