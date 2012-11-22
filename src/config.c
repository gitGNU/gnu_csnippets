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
#include <csnippets/config.h>
#include <csnippets/strmisc.h>

static void free_tokens(char **t, int n)
{
    int i;

    for (i = 0; i < n; ++i)
        free(t[i]);
    free(t);
}

struct config *config_parse(const char *filename)
{
    FILE *fp;
    int current_line = 0,
        open_brace   = 0;
    int len, i;
    struct config *config = NULL;
    char line[2947], **tokens;

    fp = fopen(filename, "r");
    if (!fp) {
        elog("failed to open configuration file %s\n", filename);
        return NULL;
    }

    while (fgets(line, 2048, fp)) {
        current_line++;

        char *ptr = strchr(line, '#');
        if (unlikely(ptr))
            *ptr = '\0';

        if (*line == '\r' || *line == '\0' || *line == '\n')
            continue;

        len = strlen(line);
        switch (open_brace) {
        case 0:
            for (i = 0; i < len; i++) {
                if (line[i] == '{') {
                    open_brace = 1;
                    line[i] = '\0';

                    struct config *conf;
                    xmalloc(conf, sizeof(*conf), goto cleanup);
                    strncpy(conf->section, strtrim(line), 32);

                    conf->def = NULL;
                    conf->next = config;
                    config = conf;
                    break;
                }
            }

            if (!open_brace)
                elog("parser error: out of brace at line %d\n", current_line);
            break;
        case 1:
            if (!config) {
                elog("adding a key with no section is forbidden.\n");
                break;
            }

            for (i = 0; i < len; i++)
                if (line[i] == '}') {
                    open_brace = 0;
                    break;
                }

            if (open_brace) {
                int n_tokens = 0;
                struct def *def;

                tokens = strexplode(line, '=', &n_tokens);
                if (!tokens || n_tokens > 2) {
                    elog("parser error: illegal equality at line %d\n",
                            current_line);
                    break;
                }
 
                xmalloc(def, sizeof(*def), goto cleanup);
                if (n_tokens == 2) {
                    strncpy(def->key, strtrim(tokens[0]), 33);
                    def->value = strdup(strtrim(tokens[1]));
                } else {
                    elog("parser error: too many (or few) parameters at line %d\n", current_line);
                    free(def);
                    free_tokens(tokens, n_tokens);
                    break;
                }
                free_tokens(tokens, n_tokens);

                def->next = config->def;
                config->def = def;
            }
            break;
        }
    }

    fclose(fp);
    return config;
cleanup:
    config_free(config);
    fclose(fp);
    return NULL;
}

void config_free(struct config *entry)
{
    struct config *p, *next;
    struct def *def, *next_def;
    if (unlikely(!entry))
        return;

    p = entry;
    while (p) {
        next = p->next;
        def = p->def;
        while (def) {
            next_def = def->next;
            free(def->value);
            free(def);
            def = next_def;
        }
        free(p);
        p = next;
    }
}

