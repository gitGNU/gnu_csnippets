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

struct module_list {
	struct list_head children;  /* List of modules  */
	unsigned int num_modules;   /* how many modules were loaded?  */
};

/**
 * module_load() - Load a single module
 *
 * @file - (.dll,.so,...) to load from.
 * @module - the module pointer where the function pointer goes.
 * @filter the filter function pointer, this function is used to filter out
 *         un-needed modules, that has function name of first parameter,
 *         if this function returns false, the function name is ignored, otherwise
 *         we store it.
 * This function returns 0 on success and -errno in the event of failure.
 *
 * Example:
 *	#include <csnippets/module.h>
 *
 *	static bool filter(const char *s)
 *	{
 *		if (strcmp(s, "module_") == 0)
 *			return true;
 *		return false;
 *	}
 *
 *	int main() {
 *		struct module *module;
 *		struct module_symbol *symbol;
 *		char *myfile = "helloworld.so";
 *		int ret;
 *
 *		ret = module_load(myfile, &module, filter);
 *		if (ret != 0)
 *			fatal("failed to load module %s %s\n",
 *				myfile, strerror(-ret));
 *
 *		list_for_each(&module->children, symbol, node)
 *			void (*myfuncptr) (void *) = 
 *				void (*myfunc) (void *)module->func_ptr;
 *		if (__builtin_expect(myfuncptr, 1))
 *			(*myfuncptr) (NULL);
 *		module_cleanup(module);
 *		return 0;
 *	}
 */
extern int module_load(const char *filename, struct module **module,
                       bool (*filter) (const char *));;

/**
 * modules_load() - Load a list of modules from dir `dir'.
 *
 * @dir - the directory containing the modules to load.
 * @list - a null list that will be set after a successfull load.
 * @filter - See module_load().
 *
 * This function returns 0 on success or -errno on failure.
 *
 * Example:
 *	#include <csnippets/module.h>
 *
 *	static bool filter(const char *s)
 *	{
 *		if (strcmp(s, "module_") == 0)
 *			return true;
 *		return false;
 *	}
 *
 *	int main() {
 *		struct module_list *modules;
 *		struct module *module;
 *		int ret;
 *		const char *dir = "mymoduledir";
 *
 *		ret = modules_load(dir, &modules);
 *		if (ret)
 *			fatal("failed to load modules from %s.\n", dir);
 *
 *		log("found %d modules in %s.\n", modules->num_modules, dir);
 *		list_for_each(&modules->children, module, node)
 *			list_for_each(&module->children, symbol, node)
 *				void (*myfuncptr) (void *) = void (*myfunc) (void *)module->func_ptr;
 *			if (__builtin_expect(myfuncptr, 0))
 *				(*myfuncptr) (NULL);
 *		modules_cleanup(modules);
 *		return 0;
 *	}
 */
extern int modules_load(const char *dir, struct module_list **list,
                        bool (*filter) (const char *));

/**
 * modules_cleanup() - Free modules
 *
 * @param modules the modules list allocated by modules_load().
 *
 * @return nothing.
 */
extern void modules_cleanup(struct module_list *modules);

/**
 * module_cleanup() - Free a module
 *
 * @param module the module to free.
 *
 * @return nothing.
 */
extern void module_cleanup(struct module *module);

#endif  /* _MODULE_H */

