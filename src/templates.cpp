
#include "configure.h"
#include "container.cpp"
#include "list.cpp"
#include "filename.h"
#include "filegroup.h"
#include "sizegroup.h"

template class Iterator<Filename>;
template class ListIterator<Filename>;
template class List<Filename>;
template class Container<FileGroup>; 
template class Container<SizeGroup>; 

