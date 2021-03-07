
/* ************************************************************************ *
 * samerm - Read the output from samefile output and removes on of them     *
 *          from the hard disk. The matches that coudn't be processed are   *
 *          printed on the output.                                          *
 *                                                                          *
 * Remove in such a way that the last modified files is kept:               *
 *            find / | samefile -iZt | samerm-Zt                            *
 * ************************************************************************ * 
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include "mainAction.h"
#include "configure.h"

#include <errno.h>

/**
 * Removes one of the files and replaces it with a hard- of symlink to the
 * other.
 *
 * @returns PRINT_AGAIN          - files didn't meet the requirements
 *          FAILED_REMOVE_LEFT   - coudn't remove the the left file 
 *          FAILED_REMOVE_RIGHT  - coudn't remove the the rigth file
 *          SUCCES_LEFT         - on succes and the left file was removed
 *          SUCCES_RIGHT        - on succes and the right file was removed
 */
int remove(const char *left, const char *right,
	const struct stat &s1, const struct stat &s2,
	const char *dst, const char *src)
{
	if (S_VERBOSE_LEVEL2(flags))
		fprintf(stderr, "%s\n", src);

	if (!S_DRYRUN(flags))
	{
		// remove the src
		if (unlink(src) && S_VERBOSE(flags))
		{
			same_errno = errno;
			return src == left ? FAILED_REMOVE_LEFT : FAILED_REMOVE_RIGHT;
		}
	}

	return src == left ? SUCCES_LEFT : SUCCES_RIGHT;
}

int main(int argc, char **argv)
{
	actionProcessOptions(argc, argv, "remove");
	actionProcessInput(remove);
	actionProcessStats();
}

