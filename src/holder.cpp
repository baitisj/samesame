
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "hash.h"
#include "toolkit.h"
#include "matchmatrix.h"
#include "sizegroup.h"
#include "holder.h"
#include "visitor.h"

#include <new>

/* ************************************************************************ */
SizeGroup Holder::tmp;

Holder::Holder(size_t capacity) throw (std::bad_alloc) : hash(capacity)
{
	hash.setHashFunction(SizeGroup::hashFunction);
}

size_t Holder::remove(off_t min, off_t max) throw()
{
	size_t counter = 0, n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i) if (hash[i] != NULL)
		if (min <= hash[i]->getFileSize() && hash[i]->getFileSize() < max)
		{
			hash.deleteItem(i);
			++counter;
		}
	hash.fix();
	return counter;
}

SizeGroup &Holder::operator[](const struct stat &s) throw (std::bad_alloc)
{
	tmp = s;
	if (hash[tmp] != NULL)
		return *hash[tmp];

	SizeGroup *ptr = new SizeGroup(s); // throws bad_alloc
	try
	{
		hash += *ptr; // throws bad_alloc
		return *ptr;
	}
	catch(std::bad_alloc &e)
	{
		delete ptr;
		throw(e);
	}
}

void Holder::sort(int (&compare)(const void *a, const void *b),
	int (&compareFilename)(const void *a, const void *b)) throw()
{
	hash.sort(SizeGroup::compare); // converts the container to an array
	size_t n = hash.getSize();
	for (size_t i = 0; i < n; ++i)
		hash[i]->sort(compare, compareFilename);
}

void Holder::accept(SamefileVisitor &v)
{
	if (v.visit(*this))
		return;
	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
		if (hash[i] != NULL)
			hash[i]->accept(v);
}

const SizeGroup *Holder::getLargestSizeGroup()
{
	size_t max = 0, size, n = hash.getBoundry();
	const SizeGroup *ret = NULL;
	for (size_t i = 0; i < n; ++i)
	{
		if (hash[i] == NULL)
			continue;
		if ((size = hash[i]->getSize()) > max)
			max = size, ret = hash[i];
	}
	return ret;
}

ulongest_t Holder::compareFiles(MatchMatrix &match, 
	int (&func)(const SizeGroup &, const FileGroup &, 
		const Filename &, const FileGroup &, const Filename &, int),
	int flags,
	int (&addingAllowed)(const char *, const FileGroup &),
	int (*postAction)(SizeGroup &, size_t, size_t),
	int (*preCheck)(const SizeGroup &, const FileGroup &, const FileGroup &)
	) throw(std::bad_alloc)
{
	ulongest_t waisted = 0;
	size_t n = hash.getBoundry();
	for (size_t i = 0; i < n; ++i)
	{
		if (hash[i] == NULL)
			continue;
		SizeGroup &select = *hash[i];
#ifdef DISK_STORAGE
		select.diskRead(addingAllowed);
#endif // DISK_STORAGE
		waisted += select.getFileSize() *
			select.compareFiles(match, func, flags, preCheck);
		if (postAction != NULL && postAction(select, i, n))
			hash.deleteItem(i);
//		if (flags & SIGNAL_TERM)
//			return waisted;
	}
	return waisted;
}

