
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "matchmatrix.h"

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

void MatchMatrix::reset(size_t n) throw()
{
#ifdef DEBUG
	if (n > this->n)
	{
		fprintf(stderr, "%s:%u n (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, n, this->n);
		exit(EXIT_FAILURE);
	}
#endif // DEBUG

#ifdef HAVE_MEMSET
#ifdef READ_ONCES
	memset(equal, 0, n * sizeof(size_t)),
	memset(fd, 0, n * sizeof(int)),
#endif // READ_ONCES
	memset(check, 0, n * sizeof(size_t));
#else // HAVE_MEMSET
	for (size_t i = 1; i < n; ++i)
	{
#if defined(READ_ONCES)
		equal[i] = 0,
		fd[i] = 0;
#endif // READ_ONCES
		check[i] = 0;
	}
#endif // HAVE_MEMSET

	/*  n = 4
	 * x 0 1 2 3
	 * 0 x 1 2 3
	 * 1 x x 4 5
	 * 2 x x x 6
	 * 3 x x x x 7
	 */
	arr[--n][0] = 0;
	for (size_t i = 0; n > 0; ++i, --n)
#ifdef HAVE_MEMSET
		memset(arr[i], 0, n * sizeof(size_t));
#else // HAVE_MEMSET
		for (size_t j = 0; j < n; ++j)
			arr[i][j] = 0;
#endif // HAVE_MEMSET
}

void MatchMatrix::createCapacity(size_t capacity) throw (std::bad_alloc)
{
#ifdef DEBUG
	if (capacity < 2)
	{
		fprintf(stderr, "%s:%zu Number (%zu) to low\n",
			__FILE__, __LINE__, capacity);
		exit(EXIT_FAILURE);
	}
#endif
	signed char *tmp = new signed char[capacity * sizeof(char **) + capacity * (capacity - 1) / 2 + 1];
	arr = (signed char **)tmp;
	tmp += capacity * sizeof(char **);
	for (size_t i = 0; i < capacity; ++i)
	{
		arr[i] = tmp;
		if (i == capacity - 1)
			++tmp;
		else
			tmp += capacity - i - 1;
	}

#ifdef READ_ONCES
	equal = new size_t[capacity],
	fd = new int[capacity],
#endif // READ_ONCES
	check = new size_t[capacity];

#ifdef DEBUG
	if (tmp + capacity * sizeof(char **) + (capacity + 1) * capacity / 2 <= arr[capacity - 1])
	{
		fprintf(stderr, "%s:%u Array out of bounds\n",
			__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
#endif
	reset(n = capacity);
}

MatchMatrix::~MatchMatrix() throw()
{
	signed char *tmp = (signed char *)arr;
	delete tmp;
	delete check;
#ifdef READ_ONCES
	delete equal;
	delete fd;
#endif // READ_ONCES
}

void MatchMatrix::enlargeCapacity(size_t capacity) throw(std::bad_alloc)
{
	if (capacity <= n)
		return;
	signed char *tmp = (signed char *)arr;
	delete tmp;
	delete check;
#ifdef READ_ONCES
	delete equal;
	delete fd;
	equal = NULL, fd = NULL;
#endif // READ_ONCES
	arr = NULL, check = NULL;

	try
	{
		createCapacity(capacity);
	}
	catch(const std::bad_alloc &e)
	{
		n = 0;
		tmp = (signed char *)arr;
		delete tmp;
		delete check;
#ifdef READ_ONCES
		delete equal;
		delete fd;
#endif // READ_ONCES
		throw e;
	}
}

#ifdef DEBUG
int MatchMatrix::get(size_t i, size_t j) const throw()
{
	if (i >= n)
	{
		fprintf(stderr, "%s:%u i (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	if (j < i)
	{
		fprintf(stderr, "%s:%u j (%zu) must be larger or equal to i (%zu)\n",
		__FILE__, __LINE__, j, i);
		exit(EXIT_FAILURE);
	}
	if (j >= n)
	{
		fprintf(stderr, "%s:%u j (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, j, n);
		exit(EXIT_FAILURE);
	}
	return arr[i][i == n - 1 ? 0 : j - i - 1];
}

void MatchMatrix::set(size_t i, size_t j, int result) throw()
{
	if (i >= n)
	{
		fprintf(stderr, "%s:%u i (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	if (j < i)
	{
		fprintf(stderr, "%s:%u j (%zu) must be larger or equal to i (%zu)\n",
		__FILE__, __LINE__, j, i);
		exit(EXIT_FAILURE);
	}
	if (j >= n)
	{
		fprintf(stderr, "%s:%u j (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, j, n);
		exit(EXIT_FAILURE);
	}
	arr[i][i == n - 1 ? 0 : j - i - 1] = result;
}

#ifdef READ_ONCES
size_t MatchMatrix::getEqual(size_t i) const throw() 
{
	if (i > n)
	{
		fprintf(stderr, "%s:%u i (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	return equal[i];
}

int MatchMatrix::getFd(size_t i) const throw()
{
	if (i > n)
	{
		fprintf(stderr, "%s:%u i (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	return fd[i];
}

void MatchMatrix::increaseEqual(size_t i) const throw()  
{
	if (i > n)
	{
		fprintf(stderr, "%s:%u i (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	++equal[i];
}

void MatchMatrix::setEqual(size_t i, size_t x) throw()
{
	if (i > n)
	{
		fprintf(stderr, "%s:%u i (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	equal[i] = x;
}

int MatchMatrix::setFd(size_t i, int x) throw()
{
	if (i > n)
	{
		fprintf(stderr, "%s:%u i (%zu) must be smaller then %zu\n",
			__FILE__, __LINE__, i, n);
		exit(EXIT_FAILURE);
	}
	return fd[i] = x;
}
#endif // READ_ONCES
#endif // DEBUG

