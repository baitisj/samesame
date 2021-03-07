
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_MATCHMATRIX_H
#define AK_MATCHMATRIX_H

#include "configure.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif // HAVE_STRING_H

#ifdef DEBUG
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif // HAVE_STDLIB_H
#endif // DEBUG

#include <new>

class MatchMatrix
{
	size_t n;
	signed char **arr;
	size_t *check;
#ifdef READ_ONCES
	size_t *equal;
	int *fd;
#endif // READ_ONCES

public:  
	/**
	 * Creates a MatchMatrix that is able to store the match results of
	 * n elements.
	 */
	MatchMatrix(size_t n) throw (std::bad_alloc)
	{ createCapacity(n); }
	 
	~MatchMatrix() throw();

	/**
	 * Reset a number of elements to 0.
	 * @param n - the rows to be reset
	 */
	void reset(size_t n) throw();

private:
	void createCapacity(size_t n) throw (std::bad_alloc);

public:

	void enlargeCapacity(size_t n) throw (std::bad_alloc);

#ifndef DEBUG
	int get(size_t i, size_t j) const throw()
	{ return arr[i][j - i]; }

	void set(size_t i, size_t j, int result) throw()
	{ arr[i][j - i] = result; }

#ifdef READ_ONCES
	size_t getEqual(size_t i) const throw()
	{ return equal[i]; }

	int getFd(size_t i) const throw()
	{ return fd[i]; }

	void increaseEqual(size_t i) const throw()
	{ ++equal[i]; }

	void setEqual(size_t i, size_t x) throw()
	{ equal[i] = x; }

	int setFd(size_t i, int x) throw()
	{ return fd[i] = x; }
#endif // READ_ONCES
#else // DEBUG
	int get(size_t i, size_t j) const throw();
	void set(size_t i, size_t j, int result) throw();

#ifdef READ_ONCES
	size_t getEqual(size_t i) const throw();
	int getFd(size_t i) const throw();
	void increaseEqual(size_t i) const throw();

	void setEqual(size_t i, size_t x) throw();
	int setFd(size_t i, int x) throw();
#endif // READ_ONCES
#endif // DEBUG
};
#endif // AK_MATCHMATRIX_H

