
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include <err.h>
#include <errno.h>
#include <grp.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pwd.h>

#include "mainAction.h"

static uid_t owner = (uid_t)-1;
static gid_t group = (gid_t)-1;

/**
 * Removes one of the files and replaces it with a hard- of symlink to the
 * other.
 *
 * @returns PRINT_AGAIN          - always need to continue
 */
int mychown(const char *left, const char *right,
	const struct stat &s1, const struct stat &s2,
	const char *dst, const char *src)
{
	if (S_DRYRUN(flags))
		return PRINT_AGAIN;
	if (owner == (uid_t)-1  || group == (uid_t)-1 ||
	    owner == s1.st_uid || group == s1.st_gid ||
	    owner == s2.st_uid || group == s2.st_gid)
		return PRINT_AGAIN;
	lchown(left, owner, group);
	lchown(right, owner, group);
	return PRINT_AGAIN;
}

uid_t id(const char *name, const char *type)
{
	uid_t value;
	char *end;
	errno = 0;
	value = strtoul(name, &end, 10);
	if (errno)
		err(1, "%s", name);
	if (*end = '\0')
		errx(1, "%s: illegal %s name", name, type);
	return value;
}

int main(int argc, char **argv)
{
	flags |= SYMLINK | WITHOUT_FILE_CHECK;
	actionProcessOptions(argc, argv, "chown");

	int argi = actionProcessOptions(argc, argv, "chown");
	if (argc <= argi)
		actionUsage("chown");

	// get arguments
	{
		char *s = strchr(argv[argi], ':');
		if (s != NULL)
		{
			*s = '\0';
			if (*++s != '\0')	// Argument was not: uid:...
			{
				struct group *gr = getgrnam(argv[argi]);
				group = gr != NULL ? gr->gr_gid : id(s, "group");
			}
		}
	}
	{
		char *s = argv[argi];
		if (argv[argi] != '\0')	// Argument was not: ...:gid
		{
			struct passwd *pw = getpwnam(argv[argi]);
			owner = pw != NULL ? pw->pw_uid : id(s, "user");
		}
	}

	// work
	actionProcessInput(mychown);
	actionProcessStats();
}

