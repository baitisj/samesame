
#ifndef AK_DEBUG_H
#define AK_DEBUG_H
#define AK_DEBUG_H

#include "configure.h"

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else // HAVE_STDLIB_H
// #error include file stdlib.h required
#endif // HAVE_STDLIB_H

#define MAGIC_CODE 0x5AA55AA5

inline void checkMagic(size_t magic, const char *file, size_t line)
{
	if (magic != MAGIC_CODE)
	{
		fprintf(stderr, "%s:%zu magic code fault.\n", file, line);
		exit(EXIT_FAILURE);
	}
}

#endif // AK_DEBUG_H

