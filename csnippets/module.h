/*
 * Copyright (c) 2012 Ahmed Samy  <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#ifndef _MODULE_H
#define _MODULE_H

typedef struct mod mod_t;

mod_t *module_open(const char *filename);
void module_close(mod_t *mod);

const char *module_error(void);
const char *module_name(mod_t *);
bool module_symbol(mod_t *, const char *name, void **symbol);

#endif  /* _MODULE_H */

