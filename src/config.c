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

struct centry_t *config_parse(const char *filename)
{
    FILE *fp;
    int current_line = 0,
        open_brace   = 0;
    int len, i;
    struct centry_t *ret = NULL;
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
        if (unlikely(*line == '\0' || *line == '\n'))
            continue;
        len = strlen(line);
        switch (open_brace) {
        case 0:
            for (i = 0; i < len; i++) {
                if (line[i] == '{') {
                    open_brace = 1;
                    line[i] = 0;

                    struct centry_t *entry;
                    xmalloc(entry, sizeof(*entry), goto cleanup);

                    strncpy(entry->section, strtrim(line), 32);
                    entry->def = NULL;

                    if (unlikely(!ret)) {
                        ret = entry;
                        list_head_init(&ret->children);
                    }

                    list_add(&ret->children, &entry->node);
                    break;
                }
            }
            if (unlikely(!open_brace)) {
                elog("parser error: out of brace at line %d\n",
                        current_line);
            }
            break;
        case 1:
            for (i = 0; i < len; i++) {
                if (line[i] == '}') {
                    open_brace = 0;
                    break;
                }
            }
            if (open_brace) {
                int n_tokens = 0;
                struct cdef_t *def;

                tokens = strexplode(line, '=', &n_tokens);
                if (!tokens || n_tokens > 2) {
                    elog("parser error: illegal equality at line %d\n",
                            current_line);
                }
 
                xmalloc(def, sizeof(*def), goto cleanup);
                if (n_tokens == 2) {
                    strncpy(def->key, strtrim(tokens[0]), 33);
                    def->key[32] = '\0';

                    def->value = strdup(strtrim(tokens[1]));
                } else {
                    elog("parser error: value with no key[?] at line %d\n",
                            current_line);
                    break;
                }
                if (unlikely(!ret->def)) {
                    ret->def = def;
                    list_head_init(&ret->def->def_children);
                }

                list_add(&ret->def->def_children, &def->node);
            }
            break;
        }
    }

    fclose(fp);
    return ret;
cleanup:
    config_free(ret);
    fclose(fp);
    return NULL;
}

void config_free(struct centry_t *entry)
{
    struct centry_t *p, *next;
    struct cdef_t *def, *next_def;
    if (unlikely(!entry))
        return;
    list_for_each_safe(&entry->children, p, next, node) {
        list_for_each_safe(&entry->def->def_children, def, next_def,
                node) {
            free(def->value);
            list_del(&def->node);
            free(def);
        }

        list_del(&p->node);
        free(p);
    }
}

