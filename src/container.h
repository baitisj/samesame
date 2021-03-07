
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_CONTAINER_H
#define AK_CONTAINER_H

#define CONTAINER_HASH		1
#define CONTAINER_VECTOR	2
#define CONTAINER_SORTED	4	

#include "configure.h"
#include "hash.h"

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif // HAVE_STDDEF_H

#ifdef DEBUG
#include "debug.h"
#endif

#include <new>

template <class T>
class Container
{
#ifdef DEBUG
	size_t magic1;
#endif // DEBUG
	unsigned char mode;
	T **arr;
	short factor;
	size_t size, capacity;
	hash_t (*hashFunction)(const T &t);
#ifdef DEBUG
	size_t magic2;
#endif // DEBUG

public:
	Container(unsigned int capacity = 1, float factor = .8)
	throw (std::bad_alloc);

	~Container() throw();

private:
	/**
	 * Find the location of object t if it exists.
	 * Find a empty location for object t to go if it doesn't exists.
	 */
	size_t key(const T &t) const throw();

public:
	/**
	 * Returns the mode the container is in.
	 */
	int getMode() const throw()
#ifndef DEBUG
	{ return mode; }
#else // DEBUG
	{ checkMagic(magic1 | magic2, __FILE__, __LINE__); return mode; }
#endif // DEBUG

	/**
	 * Gets the size
	 */
	size_t getSize() const throw()
#ifndef DEBUG
	{ return size; }
#else // DEBUG
	{ checkMagic(magic1 | magic2, __FILE__, __LINE__); return size; }
#endif // DEBUG

	/**
	 * Gets the size
	 */
	size_t getCapacity() const throw()
#ifndef DEBUG
	{ return capacity; }
#else // DEBUG
	{ checkMagic(magic1 | magic2, __FILE__, __LINE__); return capacity; }
#endif // DEBUG

	/**
	 * Sets the capacity, using the given variable unless the size of this
	 * container is higher. The capacity will be a power of two.
	 */
	void setCapacity(size_t capacity) throw(std::bad_alloc);

	/**
	 * Gets the boundry for the mode of the container.
	 */
	size_t getBoundry() const throw()
#ifndef DEBUG
	{ return (mode == CONTAINER_VECTOR) ? size : capacity; }
#else // DEBUG
	{ checkMagic(magic1 | magic2, __FILE__, __LINE__);
		return (mode & CONTAINER_VECTOR) ? size : capacity; }
#endif // DEBUG

	void setHashFunction(hash_t (*hashFunction)(const T &t)) throw()
	{ this->hashFunction = hashFunction; }

	/**
	 * Converts the internal working of the container to a hash or a vector.
	 */
	void convert(int mode) throw();

	/**
	 * Find the first element and returns is.
	 */
	T *first() const throw();

	/**
	 * Find the last element and returns is.
	 */
	T *last() const throw();

	/**
	 * Deletes a item. Only use this function if you are not interested in
	 * using the container as hash. Use delete (container -= object) if
	 * you do.
	 *
	 * @param index - the object at this index is deleted and replaced
	 *                with a NULL pointer.
	 * @see -=
	 */
	void deleteItem(size_t index) throw();

	/**
	 * Deletes all items
	 */
	void deleteItems() throw();

	/**
	 * Fixes the items within the container after calls to deleteItem.
	 */
	void fix() throw();

	/**
	 * Deletes all items and reset the capacity of the container.
	 */
	void empty(size_t capacity = 1) throw(std::bad_alloc)
#ifndef DEBUG
	{ deleteItems(); setCapacity(capacity); }
#else // DEBUG
	{ checkMagic(magic1 | magic2, __FILE__, __LINE__);
		deleteItems(); setCapacity(capacity);
		checkMagic(magic1 | magic2, __FILE__, __LINE__); }
#endif // DEBUG

	/**
	 * Converts the array to a vector and sort it.
	 */
	void sort(int (&cmp)(const void *a, const void *b)) throw();

	/**
	 * Find object t and return a reference to it.
	 * This will be quite slow if the container is in the mode of a vector.
	 */
	T *operator[](const T &t) const throw();

	/**
	 * Returns a pointer to a object.
	 * @returns NULL if there is no element at the index.
	 */
	T *operator[](const size_t index) const throw()
#ifndef DEBUG
	{ return arr[index]; }
#else // DEBUG
	{
		checkMagic(magic1 | magic2, __FILE__, __LINE__);
		if (index >= capacity)
		{
			fprintf(stderr, "%s:%u %zu >= %zu\n",
				__FILE__, __LINE__, index, capacity);
			exit(EXIT_FAILURE);
		}
		checkMagic(magic1 | magic2, __FILE__, __LINE__);
		return arr[index];
	}
#endif

	/**
	 * Checks if object t is in the container.
	 * This will be quite slow if the container is in the mode of a vector.
	 */
	int operator!=(const T &t) const throw();
	int operator==(const T &t) const throw()
#ifndef DEBUG
	{ return !operator!=(t); }
#else // DEBUG
	{ checkMagic(magic1 | magic2, __FILE__, __LINE__);
		return !operator!=(t); }
#endif // DEBUG

	/**
	 * Adds object t to the container. Operators with objects as paramaters
	 * will not function properly if objects are added that area equal (==)
	 * to another object in the container.
	 */
	void operator+=(T &t) throw (std::bad_alloc);

	/**
	 * Removes object t from the container. Operators with objects as
	 * paramatersw ill not function properly if objects are added that
	 * area equal (==) to another object in the container.
	 *
	 * Returns a pointer to the object that was removed;
	 */
	T *operator-=(T &t) throw();
};

#endif // AK_CONTAINER_H

