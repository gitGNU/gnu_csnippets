/* Public Domain */
#ifndef _STR_MISC_H
#define _STR_MISC_H

/* See the comment in csnippets/csnippets.h  */
#include_next <string.h>

char *strtrim(char *str);
char **strexplode(char *string, char seperator, int *size);
int strwildmatch(const char *pattern, const char *string);
/* Compare each character of a string using the
 * compare function specified.  */
bool strccmp(const char *str, int (*cmp) (int));
/* Convert a string with convfun function and return
 * a malloc'd version of it.  */
char *strconv(const char *str, int (*conv) (int));
/* Check if a string starts with @start  */
bool strstartswith(const char *str, const char *start);
/* Check if a string ends with @end  */
bool strendswith(const char *str, const char *end);

#endif /*  _STR_MISC_H */

