
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_SAMEFILE_VISITOR_H
#define AK_SAMEFILE_VISITOR_H

#include "configure.h"

class Holder;
class SizeGroup;
class FileGroup;
class Filename;

struct SamefileVisitor
{
	virtual int visit(Holder &holder) = 0;
	virtual int visit(SizeGroup &sizegroup) = 0;
	virtual int visit(FileGroup &filegroup) = 0;
	virtual void visit(Filename &filename) = 0;
};

#include "holder.h"
#include "sizegroup.h"
#include "filegroup.h"
#include "filename.h"

#endif // AK_SAMEFILE_VISITOR_H

