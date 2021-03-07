
/* ************************************************************************ *
 * ************************************************************************ * 
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "mainAction.h"

static char **arr = NULL;
static size_t arrSize = 0, arrCapacity = 1024;

void buffer_add(const char *line)
{
	if (line == NULL)
		return;
	if (arrSize == arrCapacity)
	{
		char **tmp = new char *[2 * arrCapacity];
		memcpy(tmp, arr, arrCapacity * sizeof(char *));
		delete arr;
		arr = tmp;
#ifdef DEBUG
		for (size_t i = arrCapacity; i < 2 * arrCapacity; ++i)
			arr[i] = NULL;
#endif // DEBUG
		arrCapacity *= 2;
	}
	size_t len = strlen(line) + 1;
	arr[arrSize] = new char[len];
	memcpy(arr[arrSize], line, len);
	++arrSize;
}

void buffer_flush(const char *line = NULL)
{
	if (arr == NULL)
		return;
	for (size_t i = 0; i < arrSize; ++i)
	{
#ifdef DEBUG
		if (arr[i] == NULL)
		{
			fprintf(stderr, "debug: %s:%d arr[%u] mustn't be NULL\n",
				__FILE__, __LINE__, i);
			exit(EXIT_FAILURE);
		}
#endif
		printf("%s\n", arr[i]);
		delete arr[i];
#ifdef DEBUG
		arr[i] = NULL;
#endif
	}
	arrSize = 0;
	if (line != NULL)
		buffer_add(line);
}

int first = 1;
match_t old;

int buffer_action(Cache &cache, match_t &m,
	const char *line, const char *f1, const char *f2)
{
	if (first)
	{
		first = 0;
		old = m;
	}

	// add the line if any the files matches any of the previous files
	// flush otherwise
	if (m.s1.st_size == old.s1.st_size ||
		m.s1.st_size == old.s2.st_size ||
		m.s2.st_size == old.s2.st_size ||
		m.s2.st_size == old.s1.st_size ||
		arrSize == 0)
		buffer_add(line);
	else
		buffer_flush(line);
	old = m;
	return 0;
}

int main(int argc, char **argv)
{
	flags |= SYMLINK | WITHOUT_FILE_CHECK;
	arr = new char *[arrCapacity];
#ifdef DEBUG
	for (size_t i = 0; i < arrCapacity; ++i)
		arr[i] = NULL;
#endif // DEBUG
	arrSize = 0;
	actionProcessOptions(argc, argv, "buffer");
	actionProcessInput(buffer_add, buffer_action);
	buffer_flush();
	actionProcessStats();
	delete arr;
}

