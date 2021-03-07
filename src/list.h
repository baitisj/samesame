
#ifndef AK_LIST_H
#define AK_LIST_H

#include "configure.h"

#include <new>
#include <typeinfo>	// std::bad_cast

template <class T>
class List
{
	T *item;
	List *nextNode;

public:
	List() throw() { item = NULL, nextNode = NULL; }
	List(T *obj) throw() { item = obj, nextNode = NULL; }
	List(T *obj, List<T> *list) throw() { item = obj, nextNode = list; }
	~List() throw();

	int isEmpty() const throw()
	{ return item == NULL; }

	size_t getSize() const throw();

	List *next() const throw() { return nextNode; }
	T *first() const throw() { return item; }
	T *last() const throw();
	void operator+=(T *item) throw (std::bad_alloc);

	void deleteItems(void (*f)(void *) = NULL) throw();
	void empty(size_t n) throw() { deleteItems(); }

	void sort(int (&cmp)(const void *a, const void *b))
		throw(std::bad_alloc);

	int operator!=(const T &obj) const throw();
};

template <class T>
class Iterator
{
public:
	virtual void operator=(const Iterator &a) = 0;
	virtual void operator++() = 0;
	virtual void reset() = 0;
	virtual int end() const = 0;
	virtual T *getItem() const = 0;

};

template <class T>
class ListIterator : public Iterator<T>
{
	const List<T> *list, *current;
	size_t index;

public:
	ListIterator(const List<T> &list)
	: list(&list), current(&list), index(0) {}

	void operator=(const Iterator<T> &a);
	void operator++() { current = current->next(); ++index; }
	void reset() { current = list, index = 0; }
	int end() const { return current == NULL ? 1 : 0; }
	T *getItem() const { return current->first(); }
};

#endif // AK_LIST_H

