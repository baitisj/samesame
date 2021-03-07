
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ *
 * This source was written with a tabstop every four characters             *
 * In vi type :set ts=4                                                     *
 * ************************************************************************ */

#ifndef AK_CONTAINER_CPP
#define AK_CONTAINER_CPP

#include "configure.h"
#include "container.h"
#include "toolkit.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif // HAVE_STDLIB_H

#include <new>

template <class T>
Container<T>::Container(unsigned capacity, float factor) throw (std::bad_alloc)
{
#ifdef DEBUG
	if (capacity < 1)
	{
		fprintf(stderr, "%s:%d Capacity(%i) must be at least 1\n",
			__FILE__, __LINE__, capacity);
		exit(EXIT_FAILURE);
	}
	if (factor <= 0 || factor >= 1)
	{
		fprintf(stderr, "%s:%d factor(%f) must be between 0 and 1\n",
			__FILE__, __LINE__, factor);
		exit(EXIT_FAILURE);
	}
	magic1 = MAGIC_CODE;
	magic2 = MAGIC_CODE;
#endif
	mode = CONTAINER_HASH;
	size = 0;
	hashFunction = NULL;
	this->factor = (size_t) (1024 * factor);
	arr = new T *[this->capacity = capacity]; // throws bad_alloc
	for (size_t i = 0; i < capacity; ++i)
		arr[i] = NULL;
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
Container<T>::~Container() throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	delete[] arr;
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
size_t Container<T>::key(const T &t) const throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	size_t key = (hashFunction ? hashFunction(t) : 0) % capacity;
	while(arr[key] != NULL && t != *arr[key])
		if (++key == capacity)
			key = 0;
#ifdef DEBUG
	if (key >= capacity)
	{
		fprintf(stderr, "%s:%d key(%zu) must be between 0 <= key < %zu\n",
			__FILE__, __LINE__, key, capacity);
		exit(EXIT_FAILURE);
	}
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	return key;
}

template <class T>
void Container<T>::setCapacity(size_t n) throw(std::bad_alloc)
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	size_t c = 1;
	while(c < n)	c <<= 1;
	while(c < size)	c <<= 1;
	T **swap = new T *[c];
	size_t m = capacity;
	capacity = c;
	T **tmp = arr;
	arr = swap;
	for (size_t i = 0; i < capacity; ++i)
		arr[i] = 0;
	for (size_t i = 0; i < m; ++i)
		arr[key(*tmp[i])] = tmp[i];
	delete[] tmp;
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
T *Container<T>::first() const throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	for (size_t i = 0; i < capacity; ++i)
		if (arr[i] != NULL)
			return arr[i];
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	return NULL;
}

template <class T>
T *Container<T>::last() const throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	size_t i;
	switch(mode)
	{
		default:
			i = capacity - 1;
			break;
		case CONTAINER_VECTOR:
		case CONTAINER_VECTOR | CONTAINER_SORTED:
			i = size - 1;
	}
	for (; i < capacity; --i)
		if (arr[i] != NULL)
			return arr[i];
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	return NULL;
}

template <class T>
void Container<T>::deleteItem(size_t i) throw()
{
#ifdef DEBUG
	if (i >= capacity)
	{
		fprintf(stderr, "%s:%d %zu should be less then %zu\n",
			__FILE__, __LINE__, i, capacity);
	}
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	delete arr[i];
	arr[i] = NULL;
	--size;
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
void Container<T>::deleteItems() throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	for (size_t i = 0; i < capacity; ++i)
		if (arr[i] != NULL)
		{
			delete arr[i];
			arr[i] = NULL;
		}
	size = 0;
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
void Container<T>::fix() throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif // DEBUG
	switch(mode)
	{
		case CONTAINER_HASH:
			{
				for (size_t i = 0; i < capacity; ++i) if (arr[i] != NULL)
					if (key(*arr[i]) != i)
					{
						arr[key(*arr[i])] = arr[i];
						arr[i] = NULL;
					}
			}
			break;
		case CONTAINER_VECTOR:
			{
				size_t i = 0, j = capacity - 1;
				while(i < size && j >= size)
				{
					// find the first NULL pointer
					while(i < j && arr[i] != NULL)
						++i;

					// find the first non-NULL pointer
					while(i < j && arr[j] == NULL)
						--j;

					arr[i] = arr[j];
					arr[j] = NULL;
					--j;
				}
			}
			break;
		case CONTAINER_VECTOR | CONTAINER_SORTED:
			{
				size_t i = 0, j = 0, k = 0;
				while(i < capacity)
				{
					// find the first NULL pointer
					for (; i < capacity && arr[i] != NULL; ++i);

					// find the first non-NULL pointer;
					if (j < i)
						j = i + 1;
					else if (j == i)
						++j;
					for (; j < capacity && arr[j] == NULL; ++j); //Intended
					if (j >= capacity)
						break;

					// find the second NULL pointer
#ifdef DEBUG
					fprintf(stderr, "%s:%d needs to be tested\n",
							__FILE__, __LINE__);
#endif // DEBUG
					if (k < j)
						k = j + 1;
					else if (k == j)
						++k;
					for (; k < capacity && arr[k] != NULL; ++k); // Intended
					if (k >= capacity)
						k = capacity;

					// Copy pointers
#ifdef DEBUG
					fprintf(stderr, "%s:%d needs to be tested\n",
							__FILE__, __LINE__);
#endif // DEBUG
					for (; j < k; ++i, ++j)
						arr[i] = arr[j];
					for (--j; j >= i; --j)
							arr[j] = NULL;
				}
			}
			break;
	}
#ifdef DEBUG
	switch(mode)
	{
		case CONTAINER_HASH:
			for (size_t i = 0; i < capacity; ++i) if (arr[i] != NULL)
				if (key(*arr[i]) != i)
				{
					fprintf(stderr, "%s:%d didn't fix the hash.\n",
						__FILE__, __LINE__);
					exit(EXIT_FAILURE);
				}
			break;
		case CONTAINER_VECTOR:
		case CONTAINER_VECTOR | CONTAINER_SORTED:
			for (size_t i = 0; i < size; ++i)
				if (arr[i] == NULL)
				{
					fprintf(stderr,
						"%s:%d This line should have some value.\n",
						__FILE__, __LINE__);
					printf("i=%zu size=%zu cap=%zu\n", i, size, capacity);
					exit(EXIT_FAILURE);
				}
			for (size_t i = size; i < capacity; ++i)
				if (arr[i] != NULL)
				{
					fprintf(stderr,
						"%s:%d A This line should not have some value.\n",
						__FILE__, __LINE__);
					printf("i=%zu size=%zu cap=%zu\n", i, size, capacity);
					exit(EXIT_FAILURE);
				}
	}
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
void Container<T>::convert(int mode) throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	if ((this->mode & ~CONTAINER_SORTED) == (mode & ~CONTAINER_SORTED))
		return;
	switch(mode)
	{
		case CONTAINER_HASH:
			fprintf(stderr, "%s:%d not implemented\n", __FILE__, __LINE__);
			exit(EXIT_FAILURE);
			break;
		case CONTAINER_VECTOR:
			this->mode = CONTAINER_VECTOR;
			fix();
			break;
#ifdef DEBUG
		default:
			fprintf(stderr,
				"%s:%d wrong mode value: %i\n", 
				__FILE__, __LINE__, mode);
			break;
#endif // DEBUG
	}
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
void Container<T>::sort(int (&cmp)(const void *a, const void *b)) throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
	size_t counter = 0;
	for (size_t i = 0; i < capacity; ++i)
	{
		if (arr[i] != NULL)
			++counter;
		for (size_t j = i + 1; j < capacity; ++j)
			if (arr[i] != NULL && arr[i] == arr[j])
			{
				fprintf(stderr,
					"%s:%d Object double in container.\n",
					__FILE__, __LINE__);
				exit(EXIT_FAILURE);
			}
	}
	if (counter != size)
	{
		fprintf(stderr,
			"%s:%d Container wrong size.\n",
			__FILE__, __LINE__);
		printf("counter=%zu size=%zu cap=%zu\n",
			counter, size, capacity);
		exit(EXIT_FAILURE);
	}
#endif
	if (!(mode & CONTAINER_VECTOR))
		convert(CONTAINER_VECTOR);
#ifdef DEBUG
	counter = 0;
	for (size_t i = 0; i < capacity; ++i)
	{
		if (arr[i] != NULL)
			++counter;
		for (size_t j = i + 1; j < capacity; ++j)
			if (arr[i] != NULL && arr[i] == arr[j])
			{
				fprintf(stderr,
					"%s:%d Object double in container.\n",
					__FILE__, __LINE__);
				exit(EXIT_FAILURE);
			}
	}
	if (counter != size)
	{
		fprintf(stderr,
			"%s:%d Container wrong size.\n",
			__FILE__, __LINE__);
		printf("counter=%zi size=%zi cap=%zi\n",
			counter, size, capacity);
		exit(EXIT_FAILURE);
	}
	for (size_t i = 0; i < size; ++i)
		if (arr[i] == NULL)
		{
			fprintf(stderr,
				"%s:%d B This line should have some value.\n",
				__FILE__, __LINE__);
			printf("i=%zu size=%zu cap=%zu\n", i, size, capacity);
			exit(EXIT_FAILURE);
		}
#endif // DEBUG
	qsort(arr, size, sizeof(T *), cmp);
	mode = CONTAINER_VECTOR | CONTAINER_SORTED;
#ifdef DEBUG
	counter = 0;
	for (size_t i = 0; i < capacity; ++i)
	{
		if (arr[i] != NULL)
			++counter;
		for (size_t j = i + 1; j < capacity; ++j)
			if (arr[i] != NULL && arr[i] == arr[j])
			{
				fprintf(stderr,
					"%s:%d Object double in container.\n",
					__FILE__, __LINE__);
				exit(EXIT_FAILURE);
			}
	}
	if (counter != size)
	{
		fprintf(stderr,
			"%s:%d Container wrong size.\n",
			__FILE__, __LINE__);
		printf("counter=%zu size=%zu cap=%zu\n",
			counter, size, capacity);
		exit(EXIT_FAILURE);
	}
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
T *Container<T>::operator[](const T &t) const throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	switch(mode)
	{
		case CONTAINER_HASH:
			return arr[key(t)];

		case CONTAINER_VECTOR:
		case CONTAINER_VECTOR | CONTAINER_SORTED:
		default:
			for (size_t i = 0; i < size; ++i)
				if (arr[i] != NULL && t == *arr[i])
					return arr[i];
	}
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	return NULL;
}

template <class T> inline
int Container<T>::operator!=(const T &t) const throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	switch(mode)
	{
		case CONTAINER_HASH:
			return arr[key(t)] == NULL;

		case CONTAINER_VECTOR:
		case CONTAINER_VECTOR | CONTAINER_SORTED:
		default:
			for (size_t i = 0; i < size; ++i)
				if (arr[i] != NULL && t == *arr[i])
					return 0;
			return 1;
	}
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
void Container<T>::operator+=(T &t) throw (std::bad_alloc)
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	size_t resize = 0;
	switch(mode)
	{
		case CONTAINER_HASH:
			if (size >= ((size_t)capacity * factor) >> 10)
				resize = 1;
			break;
		case CONTAINER_VECTOR:
		case CONTAINER_VECTOR | CONTAINER_SORTED:
			if (size == capacity)
				resize = 1;
	}
	if (resize)
	{
		T **swap = new T *[capacity << 1]; // throws bad_alloc
		T **tmp = arr;
		arr = swap;
		size_t n = capacity;
		capacity <<= 1;

		switch(mode)
		{
			case CONTAINER_HASH:
				for (size_t i = 0; i < capacity; ++i)
					arr[i] = NULL;
				for (size_t i = 0; i < n; ++i)
					if (tmp[i] != NULL)
						arr[key(*tmp[i])] = tmp[i];
				break;
			case CONTAINER_VECTOR:
			case CONTAINER_VECTOR | CONTAINER_SORTED:
				for (size_t i = size; i < capacity; ++i)
					arr[i] = NULL;
				memcpy(arr, tmp, size * sizeof(T **));
		}
		delete[] tmp;
	}


	// Adding object t to the hash
	switch(mode)
	{
		case CONTAINER_VECTOR | CONTAINER_SORTED:
		case CONTAINER_VECTOR:	arr[size] = &t;			break;
		case CONTAINER_HASH:	arr[key(t)] = &t;		break;
	}
	++size;
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
}

template <class T>
T *Container<T>::operator-=(T &t) throw()
{
#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif

	switch(mode)
	{
		case CONTAINER_HASH:
		{
			size_t k = key(t);
			if (arr[k] == NULL)
				return NULL;
			T *ptr = arr[k];
			arr[k] = NULL;
			--size;
			if (++k == capacity)
				k = 0;
			while(arr[k] != NULL)
			{
				size_t i = key(*arr[k]);
				if (k != i)
				{
					arr[i] = arr[k];
					arr[k] = NULL;
				}
				if (++k == capacity)
					k = 0;
			}
#ifdef DEBUG
			checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
			return ptr;
		}
		case CONTAINER_VECTOR:
		case CONTAINER_VECTOR | CONTAINER_SORTED:
		{
			size_t i = 0;
			T *ptr = NULL;
			for (i = 0; i < size && *arr[i] != t; ++i);
			ptr = arr[i];
			for (++i; i < size; ++i) arr[i - 1] = arr[i];
			arr[--size] = NULL;
#ifdef DEBUG
			checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
			return ptr;
		}
	}

#ifdef DEBUG
	checkMagic(magic1 | magic2, __FILE__, __LINE__);
#endif
	return NULL;
}

#endif // AK_CONTAINER_CPP

