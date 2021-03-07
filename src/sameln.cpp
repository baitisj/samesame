
/* ************************************************************************ *
 * samelink - Read the output from samefile output and links identical      *
 *            files together. The matches that coudn't be processed are     *
 *            printed on the output.                                        *
 *                                                                          *
 * Hard link identical files, keepin the last modified files:               *
 *            find / | samefile -iZt | samelink -Zt                         *
 *                                                                          *
 * Create symlinks to between files that are not on the same filesystem     *
 *            find / | samefile -i | samelink -s                            *
 * ************************************************************************ * 
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include "mainAction.h"

#include <errno.h>

/**
 * Removes one of the files and replaces it with a hard- of symlink to the
 * other.
 *
 * @returns PRINT_AGAIN          - files didn't meet the requirements
 *          FAILED_BACKUP_CREATE - coudn't create a backup file
 *          FAILED_BACKUP_DELETE - coudn't delete the backup file
 *          FAILED_REMOVE_LEFT   - coudn't remove the the left file 
 *          FAILED_REMOVE_RIGHT  - coudn't remove the the rigth file
 *          FAILED_RESTORE_LEFT  - coudn't restore the left file
 *          FAILED_RESTORE_RIGHT - coudn't restore the right file
 *          FAILED_LINK_LEFT     - coudn't link to the left file!
 *          FAILED_LINK_RIGHT    - coudn't link to the right file!
 *          SUCCES_LEFT         - on succes and the left file was removed
 *          SUCCES_RIGHT        - on succes and the right file was removed
 */
int relink(const char *left, const char *right,
	const struct stat &s1, const struct stat &s2,
	const char *dst, const char *src)
{
	// check if files are already hard linked
	if (s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino)
		return DISMISS_SILENTLY;

	// check if files are on different filesystems
	if (!S_SYMLINK(flags) && s1.st_dev != s2.st_dev)
		return PRINT_AGAIN;

	// can wel go a head and link?
	int (*linkf)(const char *dst, const char *src) =
		s1.st_dev == s2.st_dev ? link :
			S_SYMLINK(flags) ? symlink : NULL;
	if (linkf == NULL)
		return PRINT_AGAIN;

	if (S_VERBOSE_LEVEL2(flags))
	{
		fprintLink(stderr, dst, src, s1.st_dev == s2.st_dev);
		fprintf(stderr, "\n");
	}

	if (!S_DRYRUN(flags))
	{
		// create a backup
		int backup_error;
		const char postfix[] = ".sameln.";
		const size_t postfixLen = strlen(postfix);
		char *backup = new char[strlen(src) + 10 + postfixLen];
		char *tmp = backup;
		size_t i = 0;

		memcpy(backup, src, strlen(src)), tmp += strlen(src);
		memcpy(tmp, postfix, postfixLen), tmp += postfixLen;
		do
		{
			sprintf(tmp, "%u", i);
		} while((backup_error = link(src, backup) < 0) && ++i == 0);
		if (backup_error < 0)
		{
			delete backup;
			return FAILED_BACKUP_CREATE;
		}

		// relink the src
		if (unlink(src) < 0)
		{
			delete backup;
			return src == left ? FAILED_REMOVE_LEFT : FAILED_REMOVE_RIGHT;
		}
		if (linkf(dst, src) < 0)
		{
			same_errno = errno;
			if (link(backup, src) < 0)
			{
				delete backup;
				return src == left ? FAILED_RESTORE_LEFT : FAILED_RESTORE_RIGHT;
			}
			if (unlink(backup) < 0)
			{
				delete backup;
				return FAILED_BACKUP_DELETE;
			}
			delete backup;
			return src == left ? FAILED_LINK_LEFT : FAILED_LINK_RIGHT;
		}

		// delete the backup
		if (unlink(backup) < 0)
		{
			delete backup;
			return FAILED_BACKUP_DELETE;
		}
		delete backup;
	}

	return src == left ? SUCCES_LEFT : SUCCES_RIGHT;
}

int main(int argc, char **argv)
{
	actionProcessOptions(argc, argv, "relink");
	actionProcessInput(relink);
	actionProcessStats();
}

