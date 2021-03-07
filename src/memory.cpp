
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#if HAVE_MALLOC
#include "configure.h"
#include "memory.h"

// using std::new_handler;
using std::bad_alloc;

void setMemoryMode(int mode)
{
	::mode = mode;
}

void setFileSize(off_t fileSize)
{
	::fileSize = fileSize
}

#ifdef DEBUG
unsigned long varDynamic = 0;
int checkLeak = 0;
#endif // DEBUG

int mode = 0;
off_t fileSize = 0;

void *operator new(size_t sz) throw (std::bad_alloc)
{
	fprintf(stderr, "REMOVE THIS: %s:%d memory size %u\n", __FILE__, __LINE__, sz);

	if (sz == 0)
		sz = 1;
	void *ptr;
	switch(mode)
	{
		default:
			ptr = malloc(sz);
	}
#ifdef DEBUG
	++varDynamic
#endif // DEBUG

	if (ptr == NULL)
	{
#ifdef __EXCEPTIONS
		throw bad_alloc();
#else   
		std::abort();
#endif
	}
	return ptr;
}

void operator delete(void *ptr)
{
	if (ptr)
	{
#ifdef DEBUG
		if (--varDynamic == 0)
			fprintf(stderr, "debug: no leaks!\n");
		else if (checkLeak)
			fprintf(stderr, "debug: leaking %u items\n", varDynamic);
#endif // DEBUG
		free(ptr);
	}
}

#ifdef DEBUG
void checkDynamic()
{
	fprintf(stderr, "debug: leaking %u items\n", varDynamic);
	checkLeak = 1;
}
#endif // DEBUG
#endif // HAVE_MALLOC

