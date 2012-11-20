#include <csnippets/error.h>
#include <csnippets/asprintf.h>

#include <stdarg.h>

static const char *type_strings[] = { "fatal error", "warning", "notice", NULL };
extern char *program_invocation_short_name;

#define __log(s, args...) do { \
    fprintf(stderr, s, ##args); /* freopen specific (stderr) */ \
    fflush(stderr);             /* write the changes right away */ \
    fprintf(stdout, s, ##args); /* on console */ \
} while (0)

void __noreturn error_nret(const char *str, ...)
{
    va_list ap;
    char *buff;

    va_start(ap, str);
    vasprintf(&buff, str, ap); 
    va_end(ap);

    __log("%s: %s\n%s: error is not recoverable, terminating now...\n",
            program_invocation_short_name, buff, program_invocation_short_name);
    free(buff);
    exit(EXIT_FAILURE);
}

void __noreturn log_errno(const char *str, ...)
{
    va_list ap;
    char *buff;

    va_start(ap, str);
    (void) vasprintf(&buff, str, ap);
    va_end(ap);

    __log("%s: %s (%d): %s\n", program_invocation_short_name, buff, errno, strerror(errno));
    __log("%s: error is not recoverable, terminating now...\n", program_invocation_short_name);
    free(buff);
    exit(EXIT_FAILURE);
}

void error(int error_type, const char *str, ...)
{
    va_list va;
    char *buff;

    va_start(va, str);
    (void) vasprintf(&buff, str, va);
    va_end(va);

    assert((error_type == -1 || error_type < countof(type_strings)));
    if (error_type == LOG_NULL) /* special type for logging */
        __log("%s: %s", program_invocation_short_name, buff);
    else
        __log("%s: %s: %s", program_invocation_short_name, type_strings[error_type], buff);
    free(buff);
}

