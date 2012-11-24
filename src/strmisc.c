/*
 * Copyright (c) 2012 Allan Ference  <f.fallen45@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <csnippets/strmisc.h>

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

int strwildmatch(const char *pattern, const char *string) {
    switch (*pattern) {
        case '\0': return *string;
        case '*': return !(!strwildmatch(pattern+1, string) || (*string && !strwildmatch(pattern, string+1)));
        case '?': return !(*string && !strwildmatch(pattern+1, string+1));
        default: return !((toupper(*pattern) == toupper(*string)) && !strwildmatch(pattern+1, string+1));
    }
}

bool str_cmp(const char *str, int (*cmp_func) (int))
{
    char *p = (char *)str;
    if (!str || *str == '\0')
        return false;
    if (!cmp_func)
        return false;
    while (*p)
        /*
         * compare every single character in this string
         * if one fails, then the whole comparison fails...
         */
        if (!cmp_func((int)*p++))
            return false;
    return true;
}

char *str_convert(const char *str, int (*convert_func) (int))
{
    char *p;
    int len, i;

    if (unlikely(!convert_func))
        return NULL;

    len = strlen(str);
    if (len < 0)
        return NULL;

    xmalloc(p, len + 1, return NULL);
    for (i = len; i > 0; i--)
        p[i] = convert_func((int)str[i]);
    p[len + 1]  = '\0';

    return p;
}

