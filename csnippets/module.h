/*
 * Copyright (c) 2012 Ahmed Samy  <f.fallen45@gmail.com>
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
#ifndef _MODULE_H
#define _MODULE_H

#include <csnippets/list.h>

struct module_symbol {
	char *symbol_name;           /* symbol name (aka function name mostly)  */
	void *func_ptr;              /* the function pointer, this should be casted
                                      to something like void (*my_func) (...);  */
	struct list_node node;
};

struct module {
	void *handle;                   /* internal usage */
	struct list_head children;      /* Symbols list  */
	struct list_node node;          /* Next and previous module, see list.h  */
};

typedef struct list_head module_list;

/**
 * module_load() - Load a single module
 *
 * @file - (.dll,.so,...) to load from.
 * @module - the module pointer where the function pointer goes.
 * @filter the filter function pointer, this function is used to filter out
 *         un-needed modules, that has function name of first parameter,
 *         if this function returns false, the symbol is ignored, otherwise
 *         we store it.
 * This function returns 0 on success and -errno in the event of failure.
 *
 * Example:
 *	see examples/modules/modloader.c
 */
extern int module_load(const char *filename, struct module **module,
                       bool (*filter) (const char *));
extern void module_cleanup(struct module *module);

/**
 * modules_load() - Load a list of modules from @dir
 *
 * @dir - the directory containing the modules to load.
 * @list - a null list that will be set after a successfull load.
 * @filter - See module_load().
 *
 * This function returns the number of modules loaded or -1 on failure.
 *
 * Example:
 *	See examples/modules/modloader.c
 */
extern int modules_load(const char *dir, module_list *list,
                        bool (*filter) (const char *));
extern void modules_cleanup(module_list *modules);

#endif  /* _MODULE_H */

