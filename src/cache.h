
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#include "configure.h"
#include "hash.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif // HAVE_SYS_STAT_H

typedef struct match_t {
	struct stat s1;
	struct stat s2;
	int result;

	// make sure match_t don't get out of scope!
	match_t &operator=(const match_t &t)
	{
		s1.st_ino = t.s1.st_ino, s1.st_dev = t.s1.st_dev;
		s2.st_ino = t.s2.st_ino, s2.st_dev = t.s2.st_dev;
		result = t.result;
		return *this;
	}
} match_t;

class Cache
{
	size_t capacity;
	match_t *arr;

public:
	Cache(unsigned capacity = STATIC_CACHE_CAPACITY);
	~Cache();

private:
	/**
	 * Get the index where m is or should be.
	 */
	hash_t key(const match_t &m);

public:
	/**
	 * Do we have m in the cache?
	 */
	int operator==(const match_t &m);
	int operator!=(const match_t &m)
	{ return !operator==(m); }

	/**
	 * Put m into the cache
	 */
	void operator+=(const match_t &m);

	/**
	 * Return the entry at the index.
	 */
	const match_t &operator[](size_t index);

	/**
	 * Return the entry at the index where m is or should be.
	 */
	const match_t &operator[](const match_t &m)
	{ return operator[](key(m)); }
};

