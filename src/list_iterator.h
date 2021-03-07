
#ifndef AK_LIST_ITERATOR_H
#define AK_LIST_ITERATOR.H

template <class T>
class ListIterator
{
	List *list;
	List *current;
	unsigned index;

public:
	ListIterator(List *);

	T *operator[](const size_t index) const throw();
}

#endif // AK_LIST_ITERATOR
