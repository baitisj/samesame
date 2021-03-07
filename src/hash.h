
/* ************************************************************************ *
 *                  Bob Jenkins, May 2006, Public Domain                    *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_HASH_H
#define AK_HASH_H

#include "configure.h"

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif // HAVE_STDDEF_H

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
typedef uint32_t hash_t;
#else
typedef unsigned hash_t;
#endif

hash_t one_at_a_time_hash(unsigned char *key, size_t key_len) throw();

/**
 * This works on all machines.  To be useful, it requires
 * -- that the key be an array of uint32_t's, and
 * -- that the length be the number of uint32_t's in the key
 *
 * The function hashword() is identical to hashlittle() on little-endian
 * machines, and identical to hashbig() on big-endian machines,
 * except that the length has to be measured in uint32_ts rather than in
 * bytes.  hashlittle() is more complicated than hashword() only because
 * hashlittle() has to dance around fitting the key bytes into registers.
 *
 * @param k - the key, an array of uint32_t values
 * @param length - the length of the key, in uint32_ts
 * @param initval - the previous hash, or an arbitrary value
 */
hash_t hashword(const hash_t *k, size_t length, hash_t interval = 0) throw();

#endif // AK_HASH_H

