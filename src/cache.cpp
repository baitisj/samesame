
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "hash.h"
#include "cache.h"

#if defined(HAVE_STRING_H) && defined(HAVE_MEMSET)
#include <string.h>
#endif // HAVE_STRING_H && HAVE_MEMSET

#ifdef DEBUG
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>	// EXIT_FAILURE
#endif // HAVE_STDLIB_H
#endif // DEBUG

Cache::Cache(unsigned capacity)
{
	this->capacity = capacity;
	arr = new match_t[capacity];
#ifdef HAVE_MEMSET
	memset(arr, 0, sizeof(match_t) * capacity);
#else // HAVE_MEMSET
	for (size_t i = 0; i < capacity; ++i)
	{
		arr[i].s1.st_dev = 0;
		arr[i].s1.st_ino = 0;
		arr[i].s2.st_dev = 0;
		arr[i].s2.st_ino = 0;
		arr[i].result = 0;
	}
#endif // HAVE_MEMSET
}

Cache::~Cache()
{
	delete[] arr;
}

hash_t Cache::key(const match_t &m)
{
	return hashword((hash_t *)&m.s1.st_ino, sizeof(ino_t) / sizeof(hash_t),
		hashword((hash_t *)&m.s2.st_ino, sizeof(ino_t) / sizeof(hash_t),
		hashword((hash_t *)&m.s1.st_dev, sizeof(dev_t) / sizeof(hash_t),
		hashword((hash_t *)&m.s2.st_dev, sizeof(dev_t) / sizeof(hash_t)
		)))) % capacity;
}

int Cache::operator==(const match_t &m)
{
	return arr[key(m)].s1.st_ino == m.s1.st_ino &&
		arr[key(m)].s2.st_ino == m.s2.st_ino &&
		arr[key(m)].s1.st_dev == m.s1.st_dev &&
		arr[key(m)].s2.st_dev == m.s2.st_dev;
}

void Cache::operator+=(const match_t &m)
{
	arr[key(m)] = m;
}

const match_t &Cache::operator[](size_t index)
{
#ifdef DEBUG
	if (index >= capacity)
	{
		fprintf(stderr, "%s:%d index must be smaller then %u",
			__FILE__, __LINE__, capacity);
		exit(EXIT_FAILURE);
	}
#endif
	return arr[index];
}

