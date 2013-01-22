/* Public Domain */
#include <csnippets/string.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *strtrim(char *str)
{
	char *end;

	/* trim leading spaces */
	while (isspace(*str)) str++;

	if (*str == 0)   /* all spaces? */
		return str;

	end = str + strlen(str) -1;
	while (end > str && isspace(*end)) end--;

	*(end+1) = '\0';
	return str;
}

char **strexplode(char *string, char separator, int *size)
{
	int start = 0, i, k = 0, count = 2;
	char **strarr;
	for (i = 0; string[i] != '\0'; i++)
		if (string[i] == separator)
			count++;

	*size = count-1;
	strarr = calloc(count, sizeof(char*));
	i = 0;
	while (*string != '\0') {
		if (*string == separator) {
			strarr[i] = calloc(k - start + 2,sizeof(char));
			strncpy(strarr[i], string - k + start, k - start);
			strarr[i][k - start + 1] = '\0'; /* ensure null termination */
			start = k;
			start++;
			i++;
		}
		string++;
		k++;
	}

	strarr[i] = calloc(k - start + 1,sizeof(char));
	strncpy(strarr[i], string - k + start, k - start);
	strarr[++i] = NULL;

	return strarr;
}

int strwildmatch(const char *pattern, const char *string)
{
	switch (*pattern) {
	case '\0':
		return *string;
	case '*':
		return !(!strwildmatch(pattern+1, string) || (*string && !strwildmatch(pattern, string+1)));
	case '?':
		return !(*string && !strwildmatch(pattern+1, string+1));
	default:
		return !((toupper(*pattern) == toupper(*string)) && !strwildmatch(pattern+1, string+1));
	}
}

bool strccmp(const char *str, int (*cmp) (int))
{
	register const char *p = str;
	if (!str || !cmp)
		return false;
	while (*p)
		if (!cmp((int)*p++))
			return false;
	return true;
}

char *strconv(const char *str, int (*conv) (int))
{
	int len;
	register int i;
	char *ret;

	if (!conv || !str || *str == '\0')
		return NULL;
	len = strlen(str);
	xmalloc(ret, len + 1, return NULL);
	for (i = 0; i < len; ++i)
		ret[i] = conv(str[i]);
	ret[i + 1] = '\0';
	return ret;
}

bool strstartswith(const char *str, const char *start)
{
	while (*start)
		if (*start++ != *str++)
			return false;
	return true;
}

bool strendswith(const char *str, const char *end)
{
	register const char *estr = str + (strlen(str) - 1);
	register const char *eend = end + (strlen(end) - 1);
	while (*eend)
		if (*eend-- != *estr--)
			return false;
	return true;
}

