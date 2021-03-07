
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"

#include <new>

class Buffer
{
	friend class PreventBufferLeak;
	static Buffer *singleton;
	size_t n, capacity, pagesize;
	unsigned char *buf;

public:
	static Buffer &getDefault(size_t capacity = 2 * MAXBSIZE);
	Buffer(size_t capacity = 2 * MAXBSIZE);
	~Buffer() throw();

	off_t getPageSize() const throw() { return pagesize; }
	off_t getCapacity() const throw() { return capacity; }
	void setAmountPages(size_t) throw();

#ifdef DEBUG
	unsigned char *operator[](size_t index) const throw();
#else
	unsigned char *operator[](size_t index) const throw()
	{ return buf + index * pagesize; }
#endif
};

