
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "buffer.h"

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif // DEBUG

#include <new>

extern Buffer *Buffer::singleton = NULL;

struct PreventBufferLeak
{
	~PreventBufferLeak()
	{
		delete Buffer::singleton;
	}
} preventBufferLeak;

Buffer &Buffer::getDefault(size_t capacity)
{
	if (singleton != NULL && singleton->capacity >= capacity)
		return *singleton;
	else if (singleton == NULL)
		return *(singleton = new Buffer(capacity));
	else while(capacity > singleton->getCapacity())
	{
		try
		{
			Buffer *tmp = new Buffer(capacity);
			delete singleton;
			return *(singleton = tmp);
		}
		catch (const std::bad_alloc &e)
		{
			capacity /= 2;
		}
	}
	return *singleton;
}

Buffer::Buffer(size_t capacity)
{
	this->pagesize = capacity / (n = 1);
	buf = new unsigned char[this->capacity = capacity];
}

Buffer::~Buffer() throw()
{
	delete buf;
}

void Buffer::setAmountPages(size_t n) throw()
{
	if (n < 1)
		n = 1;
	pagesize = capacity / (this->n = n);
#ifdef DEBUG
	if (pagesize == 0)
	{
		fprintf(stderr, "%s:%d stall caused because capacity / n = %zu / %zu = 0\n",
			__FILE__, __LINE__, capacity, n);
		exit(EXIT_FAILURE);
	}
#endif // DEBUG
#ifdef _SC_PAGESIZE
	if (pagesize > (size_t)sysconf(_SC_PAGESIZE))
		pagesize = (size_t)sysconf(_SC_PAGESIZE);
#else // _SC_PAGESIZE
	if (pagesize > 4 * 1024)
		pagesize = 4 * 1024;
#endif // _SC_PAGESIZE
}

#ifdef DEBUG
unsigned char *Buffer::operator[](size_t index) const throw()
{
	if (index >= n)
	{
		fprintf(stderr, "%s:%d requested index %zu exceeds max %zu\n",
			__FILE__, __LINE__, index, n);
		exit(EXIT_FAILURE);
	}
	return buf + index * pagesize;
}
#endif // DEBUG

