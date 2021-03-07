
#ifndef AK_CONFIGURE_H
#define AK_CONFIGURE_H

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

// #define CHECKSUM
// #define DISK_STORAGE
// #define DEBUG
// #define EXPERIMENTAL
// #define EXPERIMENTAL_LIST_SPEEDUP
// #define PREREAD
// #define READ_ONCE
// #define LOW_MEMORY_PROFILE

#define USE_MMAP(sz, n)		((sz) < 8 * 1048576)
#define USE_MMAP2(sz, n)	((sz) < 4 * 1048576)
#define BUFFER_SIZE(sz)		(sz < MAXPHYS ? MAXPHYS : (sz > 16 * MAXPHYS ? 16 * MAXPHYS : sz))

#ifndef TEMP_STORAGE_DIR
#define TEMP_STORAGE_DIR	"/tmp/"
#endif
#ifndef PATH_INIT
#define PATH_INIT		256
#endif
#ifndef STATIC_CACHE_CAPACITY
#define STATIC_CACHE_CAPACITY	8192
#endif
#ifndef EXPECTED_MAX_GROUP
#define EXPECTED_MAX_GROUP 	1024
#endif // EXPECTED_MAX_GROUP

#ifdef CHECKSUM
#define CHECKSUM_LEN 48
#endif // CHECKSUM

#ifndef PHYSPAGES_THRESHOLD
#define PHYSPAGES_THRESHOLD (32*1024)
#endif // PHYSPAGES_THRESHOLD

#ifdef __LONG_LONG_SUPPORTED
typedef long long longest_t;
typedef unsigned long long ulongest_t;
#else // __LONG_LONG_SUPPORTED
typedef long longest_t;
typedef unsigned long ulongest_t;
#endif // __LONG_LONG_SUPPORTED

#ifndef OFF_MAX
#include <limits.h>
#ifdef __OFF_MAX
#define OFF_MAX __OFF_MAX
#else
#define OFF_MAX INT_MAX
#endif
#endif // OFF_MAX

#ifdef HAVE_CONFIG_H
#include "../config.h"
#else // HAVE_CONFIG_H

#define PACKAGE "samesame"
#define PACKAGE_BUGREPORT "samesame@akruijff.dds.nl"
#define PACKAGE_NAME "SameSame"
#define PACKAGE_STRING "SameSame 1.10"
#define PACKAGE_TARNAME "samesame"
#define PACKAGE_VERSION "1.10"
#define VERSION "1.10"

#endif // HAVE_CONFIG_H

#define COPYRIGHT		"%s, %s, copyright (c) 2010 Alex de Kruijff\n"

#endif // AK_CONFIGURE_H

