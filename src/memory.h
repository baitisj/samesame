#ifndef AK_MEMORY_H
#define AK_MEMORY_H
#define AK_MEMORY_H

#include "configure.h"

#include <cstddef>
#include <new>

void setMemoryMode(int mode);
void setFileSize(int fileSize);

void *operator new(std::size_t sz) throw (std::bad_alloc);

inline void *operator new[](std::size_t sz) throw (std::bad_alloc)
{ return operator new(sz); }

void operator delete(void *ptr);

inline void operator delete[](void *ptr)
{ operator delete(ptr); }

#ifdef DEBUG
void checkDynamic();
#endif // DEBUG
#endif // AK_MEMORY_H

