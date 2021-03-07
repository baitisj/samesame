
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_FILENAME_H
#define AK_FILENAME_H

#include "configure.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif // HAVE_STRING_H

#include <new>

class Filename
{
public:
	/**
	 * Assumes a and b are of type Filename ** and compares the strings.
	 */
	static int compareFirst(const void *a, const void *b) throw()
	{ return (*(Filename **)a)->compare(**(Filename **)b); }

	static int compareLast(const void *a, const void *b) throw()
	{ return (*(Filename **)b)->compare(**(Filename **)a); }

	/**
	 * Returns the filename in a strin
	 */
	const char *data() const throw() { return (const char*)this; }

	/**
	 * Compares this filename with the other filename.
	 *
	 * @returns 0 on succes
	 */
	int compare(const Filename &obj) const throw()
	{ return strcmp((const char*)this, (const char*)&obj); }

	/**
	 * Compares this filename with the other filename.
	 *
	 * @returns true - if both filenames are equal.
	 */
	int operator==(const Filename &obj) const throw()
	{ return !compare(obj); }

	/**
	 * Compares this filename with the other filename.
	 *
	 * @returns true - if both filenames are not equal.
	 */
	int operator!=(const Filename &obj) const throw()
	{ return compare(obj); }

};

class WrapperFilename
{
	char *str;
public:
	/**
	 * Creates a capacity for a sting of the given size.
	 */
	WrapperFilename(size_t capacity = 0) throw (std::bad_alloc);

	/**
	 * Copies the string str in to the object.
	 */
	WrapperFilename(const char *str) throw (std::bad_alloc);

	~WrapperFilename() throw() {
		delete[] str;
	}

	Filename &getFilename() { return *(Filename *)str; }

	/**
	 * Resize the capacity to hold a string of a length of capacity.
	 * @see renew
	 */
	void resize(size_t capacity) throw (std::bad_alloc);

	/**
	 * Renew the capacity to hold a string of a length of capacity.
	 * @see resize
	 */
	void renew(size_t capacity) throw (std::bad_alloc);

	/**
	 * Repleaces the current filename with a copy of the given string.
	 * Note: You need to know for sure there is enove space for str.
	 */
	void operator=(const char *str) throw() { strcpy(this->str, str); }
};

#endif // AK_FILENAME_H

