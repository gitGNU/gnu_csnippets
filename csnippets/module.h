/*
 * Copyright (c) 2012 Ahmed Samy  <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#ifndef _MODULE_H
#define _MODULE_H

struct mod;

struct mod *module_open(const char *filename);
void module_close(struct mod *mod);

const char *module_error(void);
const char *module_name(struct mod *);
bool module_symbol(struct mod *, const char *name, void **symbol);

#endif  /* _MODULE_H */

