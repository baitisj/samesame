
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "filename.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif // HAVE_STRING_H

#ifdef DEBUG
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif // HAVE_STDLIB_H
#endif

WrapperFilename::WrapperFilename(size_t size) throw (std::bad_alloc)
{
	this->str = new char[++size]; // throws bad_alloc
#ifdef DEBUG
	if (size == 0)
	{
		fprintf(stderr, "%s:%d size shouldn't be 0\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	this->str[0] = 0;
}

WrapperFilename::WrapperFilename(const char *str) throw (std::bad_alloc)
{
#ifdef DEBUG
	if (str == NULL)
	{
		fprintf(stderr, "%s:%d str shouldn't be NULL\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	size_t len = strlen(str);
	this->str = new char[++len]; // throws bad_alloc
#ifdef DEBUG
	if (len == 0)
	{
		fprintf(stderr, "%s:%d len shouldn't be 0\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	memcpy(this->str, str, len);
}

void WrapperFilename::resize(size_t size) throw (std::bad_alloc)
{
	size_t len = strlen(str);
	if (size < len)
		size = len;
	char *tmp = new char[++size]; // throws bad_alloc
#ifdef DEBUG
	if (size == 0)
	{
		fprintf(stderr, "%s:%d size shouldn't be 0\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	memcpy(tmp, str, ++len);
	delete[] str;
	str = tmp;
}

void WrapperFilename::renew(size_t size) throw (std::bad_alloc)
{
	char *tmp = new char[++size]; // throws bad_alloc
#ifdef DEBUG
	if (size == 0)
	{
		fprintf(stderr, "%s:%d size shouldn't be 0\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	delete[] str;
	str = tmp;
	str[0] = 0;
}

