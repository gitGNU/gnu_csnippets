/* Public Domain */
#ifndef _STR_MISC_H
#define _STR_MISC_H

/* See the comment in csnippets/csnippets.h  */
#include_next <string.h>

/* Shamelessly stolen from ccan/str.  */
#define streq(a, b) (strcmp((a), (b)) == 0)
#define strstarts(str,prefix) (strncmp((str),(prefix),strlen(prefix)) == 0)
static inline bool strends(const char *str, const char *postfix)
{
	if (strlen(str) < strlen(postfix))
		return false;

	return streq(str + strlen(str) - strlen(postfix), postfix);
}

char *strtrim(char *str);
char **strexplode(char *string, char seperator, int *size);
int strwildmatch(const char *pattern, const char *string);
/* Compare each character of a string using the
 * compare function specified.  */
bool strccmp(const char *str, int (*cmp) (int));
/* Convert a string with convfun function and return
 * a malloc'd version of it.  */
char *strconv(const char *str, int (*conv) (int));

#endif /*  _STR_MISC_H */

