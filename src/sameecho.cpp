
/* ************************************************************************ *
 *                         Written by Alex de Kruijff                       *
 * ************************************************************************ * 
 * This source was written with a tabstop every four characters             * 
 * In vi type :set ts=4                                                     * 
 * ************************************************************************ */

#include "mainAction.h"

/**
 * Prints the line again.
 */
int echo(const char *left, const char *right,
	const struct stat &s1, const struct stat &s2,
	const char *dst, const char *src)
{
	return PRINT_AGAIN;
}

int main(int argc, char **argv)
{
	actionProcessOptions(argc, argv, "echo");
	actionProcessInput(echo);
	actionProcessStats();
}

