
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "filename.h"
#include "filegroup.h"
#include "sizegroup.h"
#include "holder.h"
#include "stats.h"

int Stats::visit(Holder &holder)
{
	return 0;
}

int Stats::visit(SizeGroup &sizegroup)
{
	if ((currentFileSize = sizegroup.getFileSize()) < 0)
		return 0;
	if (minFileSize > currentFileSize)
		minFileSize = currentFileSize;
	if (maxFileSize < currentFileSize)
		maxFileSize = currentFileSize;
	if (sizeLargestGroupFiles < sizegroup.getSize())
		largestGroupFiles = &sizegroup,
						  sizeLargestGroupFiles = sizegroup.getSize();
	totalSize += sizegroup.getSize() * currentFileSize,
	files += sizegroup.getSize();
	return 0;
}

int Stats::visit(FileGroup &filegroup)
{
	filenames += filegroup.getSize();
	return 1;
}

void Stats::visit(Filename &filename) {}

