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
#if defined(__linux) || defined(linux)

#include <csnippets/module.h>
#include <csnippets/asprintf.h>

#include <elf.h>
#include <dirent.h>
#include <dlfcn.h>
#include <ctype.h>

static inline int filter(const struct dirent *dir)
{
    const char *name = dir->d_name;
    return strlen(name) > 3 ? !strcmp(name + strlen(name) - 3, ".so") : 0;
}

int module_load(const char *file, struct module **mod,
        const char *start_name)
{
    FILE *fp;
    struct module *module = NULL;
    struct module_symbol *symbol = NULL;

    off_t size;
    int err = 0, i, j;
    unsigned int symbol_loc = 0, symbol_count = 0;
    char **symbols = NULL, *buffer,
         *errorstr;

    Elf32_Ehdr *hdr;
    Elf32_Shdr *shdr;
    Elf32_Sym *sym;
    void *handle = NULL;

    if (!(fp = fopen(file, "r")))
        return -errno;

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    if (!(buffer = malloc(size))) {
        fclose(fp);
        return -ENOMEM;
    }

    if (fread(buffer, 1, size, fp) != size) {
        fclose(fp);
        free(buffer);
        return -ENOBUFS;
    }
    fclose(fp);

    hdr = (Elf32_Ehdr *)buffer;
    shdr = (Elf32_Shdr *)(buffer + hdr->e_shoff);

    for (i = 0; i < hdr->e_shnum; ++i) {
        if (shdr[i].sh_type == SHT_STRTAB &&
                strcmp(buffer + shdr[hdr->e_shstrndx].sh_offset + shdr[i].sh_name,
                    ".dynstr") == 0) {
#ifdef __debug_modules
            printf("found symbol %d\n", i);
#endif
            symbol_loc = i;
            break;
        }
    }

    for (i = 0; i < hdr->e_shnum; ++i) {
        if (shdr[i].sh_type != SHT_DYNSYM)
            continue;

        sym = (Elf32_Sym *)(buffer + shdr[i].sh_offset);
        for (j = 0; j < shdr[i].sh_size / shdr[i].sh_entsize; ++j) {
            if (!sym[j].st_name || !sym[j].st_size)
                continue;

            char *func_name = buffer + shdr[symbol_loc].sh_offset + sym[j].st_name;
            if (!strstr(func_name, start_name))
                continue;
#ifdef __debug_modules
            printf("found %s.\n", func_name);
#endif
            xrealloc(symbols, symbols, (symbol_count + 1) * sizeof(char *),
                    err = -ENOMEM; goto cleanup);
            xmalloc(symbols[symbol_count], strlen(func_name) + 1,
                    err = -ENOMEM; goto cleanup);

            strcpy(symbols[symbol_count], func_name);
            ++symbol_count;
        }
    }

    char *tmpf;
    if (asprintf(&tmpf, "./%s", file) < 0) {
        err = -ENOMEM;
        goto cleanup;
    }

    if (!(handle = dlopen(tmpf, RTLD_LAZY))) {
        err = -errno;
        goto cleanup;
    }

    xmalloc(module, sizeof(struct module), err = -ENOMEM; goto cleanup);
    list_head_init(&module->children);
    for (i = 0; i < symbol_count; ++i) {
        xmalloc(symbol, sizeof(*symbol), err = -ENOMEM; goto cleanup);

        symbol->func_ptr = dlsym(handle, symbols[i]);
        if ((errorstr = dlerror())) {
            warning("error resolving symbol %s: %s\n", 
                    symbols[i], errorstr);
            goto cleanup;
        }

        symbol->symbol_name = strdup(symbols[i]);
        list_add(&module->children, &symbol->node);
    }

cleanup:
    if (err && symbol && module) {
#ifdef __debug_module
        printf("Cleaning up symbols...\n");
#endif
        for (;;) {
            symbol = list_top(&module->children, struct module_symbol, node);
            if (!symbol)
                break;

            free(symbol->symbol_name);
            list_del_from(&module->children, &symbol->node);
            free(symbol);
        }
    }

    if (symbols) {
        for (i = 0; i < symbol_count; ++i)
            free(symbols[i]);
        free(symbols);
    }

    free(buffer);
    free(tmpf);
    if (mod)
        *mod = module;
    else
        module_cleanup(module);

    return err;
}

int modules_load(const char *dir, struct module_list **modules,
        const char *start_name)
{
    int num = 0;
    int so_count = 0;
    int ret = 0;
    struct dirent **so_list;
    struct module_list *mods;
    struct module *mod = NULL;

    so_count = scandir(dir, &so_list, filter, NULL);
    if (so_count < 0)
        return -1;

    xmalloc(mods, sizeof(*mods), return 0);
    while (so_count--) {
#ifdef __debug_modules
        printf("Attempting to load module %s...\n", so_list[so_count]->d_name);
#endif
        if ((ret = module_load(so_list[so_count]->d_name, &mod,
                        start_name) != 0)) {
            warning("failed to load module: %s (%d)\n",
                    so_list[so_count]->d_name, ret);
        }
        else
            ++mods->num_modules;

        if (mod)
            list_add(&mods->children, &mod->node);

        free(so_list[so_count]);
    }

    *modules = mods;
    free(so_list);
    return 0;
}

#endif    /* defined __linux || linux */

