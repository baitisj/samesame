bin_PROGRAMS=
man1_MANS= man/samesame.1
TESTS_ENVIRONMENT = $(SHELL)
TESTS= tests/build.sh

if SAMEFILE
bin_PROGRAMS+= samefile
man1_MANS+= man/samefile.1
if LOW_MEMORY_PROFILE
TESTS+= tests/samefile-lmp.sh
else
TESTS+= tests/samefile.sh
endif
samefile_SOURCES = src/samefile.cpp src/main.cpp src/holder.cpp src/sizegroup.cpp src/matchmatrix.cpp src/storage.cpp src/filegroup.cpp src/filename.cpp src/templates.cpp src/list.cpp src/container.cpp src/list.cpp src/hash.cpp src/stats.cpp src/printhardlinked.cpp src/write2disk.cpp src/toolkit.cpp src/buffer.cpp src/memory.cpp
endif

if SAMEARCHIVE
bin_PROGRAMS+= samearchive
man1_MANS+= man/samearchive.1
TESTS+= tests/samearchive.sh
samearchive_SOURCES = src/samearchive.cpp src/main.cpp src/holder.cpp src/sizegroup.cpp src/matchmatrix.cpp src/storage.cpp src/filegroup.cpp src/filename.cpp src/templates.cpp src/container.cpp src/hash.cpp src/stats.cpp src/printhardlinked.cpp src/write2disk.cpp src/toolkit.cpp src/buffer.cpp src/memory.cpp
endif

if SAMEARCHIVE_LITE
bin_PROGRAMS+= samearchive-lite
man1_MANS+= man/samearchive-lite.1
TESTS+= tests/samearchive-lite.sh
samearchive_lite_SOURCES = src/samearchive-lite.cpp src/toolkit.cpp src/buffer.cpp src/cache.cpp src/hash.cpp
endif

if SAMECP
bin_PROGRAMS+= samecp
man1_MANS+= man/samecp.1
TESTS+= tests/samecp.sh
samecp_SOURCES = src/samecp.cpp src/mainAction.cpp src/toolkit.cpp src/buffer.cpp src/cache.cpp src/hash.cpp
endif

if SAMECHFLAGS
bin_PROGRAMS+= samechflags
man1_MANS+= man/samechflags.1
samechflags_SOURCES = src/samechflags.cpp src/mainAction.cpp src/toolkit.cpp src/buffer.cpp src/cache.cpp src/hash.cpp
endif

if SAMECHMOD
bin_PROGRAMS+= samechmod
man1_MANS+= man/samechmod.1
samechmod_SOURCES = src/samechmod.cpp src/mainAction.cpp src/toolkit.cpp src/buffer.cpp src/cache.cpp src/hash.cpp
endif

if SAMECHOWN
bin_PROGRAMS+= samechown
man1_MANS+= man/samechown.1
samechown_SOURCES = src/samechown.cpp src/mainAction.cpp src/toolkit.cpp src/buffer.cpp src/cache.cpp src/hash.cpp
endif 

if SAMEDELAY
bin_PROGRAMS+= samedelay
man1_MANS+= man/samedelay.1
TESTS+= tests/samedelay.sh
samedelay_SOURCES = src/samedelay.cpp src/mainAction.cpp src/toolkit.cpp src/buffer.cpp src/cache.cpp src/hash.cpp
endif

if FSORT
bin_PROGRAMS+= fsort
fsort_SOURCES = src/fsort.cpp src/toolkit.cpp src/buffer.cpp
endif

if SAMELN
bin_PROGRAMS+= sameln
man1_MANS+= man/sameln.1
TESTS+= tests/sameln.sh
sameln_SOURCES = src/sameln.cpp src/mainAction.cpp src/toolkit.cpp src/buffer.cpp src/cache.cpp src/hash.cpp
endif

if SAMEMV
bin_PROGRAMS+= samemv
man1_MANS+= man/samemv.1
TESTS+= tests/samemv.sh
samemv_SOURCES = src/samerm.cpp src/mainAction.cpp src/toolkit.cpp src/buffer.cpp src/cache.cpp src/hash.cpp
endif

if SAMERM
bin_PROGRAMS+= samerm
man1_MANS+= man/samerm.1
TESTS+= tests/samerm.sh
samerm_SOURCES = src/samerm.cpp src/mainAction.cpp src/toolkit.cpp src/buffer.cpp src/cache.cpp src/hash.cpp
endif

if DEBUG
AM_CXXFLAGS= -DDEBUG -g -Wall
else
AM_CXXFLAGS= -O2
endif

if CHECKSUM
AM_CXXFLAGS+= -DCHECKSUM
endif

if DISK_STORAGE
AM_CXXFLAGS+= -DDISK_STORAGE
endif

if EXPERIMENTAL
AM_CXXFLAGS+= -DEXPERIMENTAL
endif

if LOW_MEMORY_PROFILE
AM_CXXFLAGS+= -DLOW_MEMORY_PROFILE
endif

if PREREAD
AM_CXXFLAGS+= -DPREREAD
endif

if READ_ONCES
AM_CXXFLAGS+= -DREAD_ONCES
endif

if PROFILER
AM_CXXFLAGS+= -pg
endif

