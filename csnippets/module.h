/*
 * Copyright (c) 2012 Ahmed Samy  <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#ifndef _MODULE_H
#define _MODULE_H

#include <csnippets/list.h>

struct modsym {
	char *name;		/* symbol name (function name mostly)  */
	void *ptr;              /* function pointer.  */

	struct list_node node;
};

struct mod {
	struct list_head children;      /* Symbols list  */
	struct list_node node;          /* Next and previous module, see list.h  */
};

typedef struct list_head mod_list;

/**
 * read_module() - Load a single mod
 *
 * @file - (.dll,.so,...) to load from.
 * @mod - the mod pointer where the function pointer goes.
 * @filter the filter function pointer, this function is used to filter out
 *         un-needed mods, that has function name of first parameter,
 *         if this function returns false, the symbol is ignored, otherwise
 *         we store it.
 * This function returns 0 on success and -errno in the event of failure.
 *
 * Example:
 *	see examples/mods/modloader.c
 */
extern int read_module(const char *filename, struct mod **mod,
                       bool (*filter) (const char *));
/* Like read_module() but only find function name.  and store
 * information in modsym.  */
extern int readfn_module(const char *filename, struct modsym *,
			 const char *func);
extern void cleanup_module(struct mod *mod);

/**
 * read_moddir() - Load a list of mods from @dir
 *
 * @dir - the directory containing the mods to load.
 * @list - a null list that will be set after a successfull load.
 * @filter - See read_module().
 *
 * This function returns the number of mods loaded or -1 on failure.
 *
 * Example:
 *	See examples/mods/modloader.c
 */
extern int read_moddir(const char *dir, mod_list *list,
                        bool (*filter) (const char *));
extern void cleanup_mods(mod_list *mods);

#endif  /* _MODULE_H */

