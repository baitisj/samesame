
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "debug.h"

// using std::new_handler;
using std::bad_alloc;

long varDynamic = 0;
int checkLeak = 0;

void *operator new(size_t sz) throw (std::bad_alloc)
{
	if (sz == 0)
		sz = 1;
#if HAVE_MALLOC
	void *ptr = (void *) malloc (sz);
#else // HAVE_MALLOC
#error malloc function required, see file config.h.in
#endif // HAVE_MALLOC
	++varDynamic;
  
	while (ptr == 0)
	{
//		new_handler handler = __new_handler;
#ifdef __EXCEPTIONS
		throw bad_alloc();
#else   
		std::abort();
#endif
#if HAVE_MALLOC
		ptr = (void *) malloc (sz);
#else // HAVE_MALLOC
#error malloc function required, see file config.h.in
#endif // HAVE_MALLOC
	}
#ifdef DEBUG_PRINT_ADDR
	fprintf(stderr, "%s:%d new %p\n", __FILE__, __LINE__, ptr);
#endif // DEBUG_PRINT_ADDR
	return ptr;
}

void operator delete(void *ptr)
{
	if (ptr)
	{
#ifdef DEBUG_PRINT_ADDR
		fprintf(stderr, "%s:%d delete %p\n", __FILE__, __LINE__, ptr);
#endif // DEBUG_PRINT_ADDR
		if (--varDynamic == 0)
			fprintf(stderr, "debug: no leaks!\n");
		else if (checkLeak)
			fprintf(stderr, "debug: leaking %lu items\n", varDynamic);
		free(ptr);
	}
}

void checkDynamic()
{
	fprintf(stderr, "debug: leaking %lu items\n", varDynamic);
	checkLeak = 1;
}

