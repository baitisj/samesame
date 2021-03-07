#include "list.h"

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif // HAVE_STDLIB_H

template <class T>
List<T>::~List() throw()
{
	delete nextNode;
}

template <class T>
void List<T>::operator+=(T *obj) throw (std::bad_alloc)
{
	if (item == NULL)
		item = obj;
	else if (nextNode == NULL)
		nextNode = new List(obj);
	else
		nextNode = new List(obj, nextNode);
}

template <class T>
size_t List<T>::getSize() const throw()
{
	size_t counter = 0;
	const List *ptr = this;
	for (; ptr != NULL; ptr = ptr->nextNode)
		if (ptr->item != NULL)
			++counter;
	return counter;
}

template <class T>
T *List<T>::last() const throw()
{
	const List<T> *list = this;
	while(list->nextNode != NULL)
		list = list->nextNode;
	return list->item;
}

template <class T>
void List<T>::deleteItems(void (*f)(void *)) throw()
{
	const List<T> *list = this;
	for(; list != NULL; list = list->next())
	{
		if (f == NULL)
			delete list->item;
		else
			f((void *)list->item);
//		list->item = NULL;
	}
	delete nextNode;
	item = NULL, nextNode = NULL;
}

template <class T>
int List<T>::operator!=(const T &obj) const throw()
{
	const List<T> *list = this;
	while(list != NULL)
	{
		if (list->item != NULL && *list->item == obj)
			return 0;
		list = list->nextNode;
	}
	return 1;
}

template <class T>
void List<T>::sort(int (&cmp)(const void *a, const void *b))
throw(std::bad_alloc)
{
	if (nextNode == NULL)
		return;

	// Start - preventing makeing nested objects
	List<T> *sorted = new List<T>(item, nextNode);
	List<T> *unsorted = nextNode;
	sorted->nextNode = NULL; 

	while(unsorted != NULL)
	{
		// Less than the first?
		if (cmp((void *)&unsorted->item, (void *)&sorted->item) < 0)
		{
			List<T> *tmp = unsorted->nextNode;
			unsorted->nextNode = sorted;
			sorted = unsorted;
			unsorted = tmp;
			continue;
		}

		// Find insert position
		List<T> *ptr = sorted;
		while(ptr != NULL && ptr->nextNode != NULL &&
			cmp((void *)&ptr->nextNode->item, (void *)&unsorted->item) < 0)
			ptr = ptr->nextNode;

		// Insert
		List<T> *tmp = ptr->nextNode;
		ptr->nextNode = unsorted;
#ifdef EXPERIMENTAL_LIST_SPEEDUP
//		while(ptr >= unsorted->item && unsorted->item <= tmp->item)
		if (tmp != NULL)
				while(ptr != NULL && unsorted->nextNode != NULL &&
					cmp((void *)&ptr->item, (void *)&unsorted->item) >= 0) &&
					cmp((void *)&unsorted->item, (void *)&tmp->item) <= 0))
					unsorted = ptr = ptr->nextNode;
#endif // EXPERIMENTAL_LIST_SPEEDUP
		unsorted = unsorted->nextNode;
		ptr->nextNode->nextNode = tmp;
	}

	// Finish
	item = sorted->item;
	nextNode = sorted->nextNode;
	sorted->nextNode = NULL;
	delete sorted;
}

template <class T>
void ListIterator<T>::operator=(const Iterator<T> &a)
{
	 try
	 {
		 const Iterator<T> *b = &a;
		 const ListIterator<T> *ptr = dynamic_cast<const ListIterator<T> *>(b);
		 list = ptr->list;
		 current = ptr->current;
		 index = ptr->index;
	 }
	 catch (const std::bad_cast &e)
	 {
		 fprintf(stderr, "%s:%d ptr not of type ListIterator\n",
			__FILE__, __LINE__);
		 exit(EXIT_FAILURE);
	 }
}

