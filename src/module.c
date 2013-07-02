/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>.
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#include <csnippets/module.h>

typedef struct mod {
	void *handle;
	const char *name;
} mod_t;

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))

#include <csnippets/asprintf.h>

#include <dirent.h>
#include <dlfcn.h>

mod_t *module_open(const char *filename)
{
	char *tmpf;
	mod_t *ret;
	void *d;

	if (!asprintf(&tmpf, "./%s", filename))
		return NULL;

	d = dlopen(tmpf, RTLD_LAZY);
	if (!d) {
		free(tmpf);
		return NULL;
	}

	xmalloc(ret, sizeof(*ret), free(tmpf); dlclose(d); return NULL);
	ret->name = tmpf;
	ret->handle = d;
	return ret;
}

void module_close(mod_t *mod)
{
	if (!mod)
		return;

	free((void *)mod->name);
	dlclose(mod->handle);
	free(mod);
	return;
}

const char *module_error(void)
{
	return dlerror();
}

bool module_symbol(mod_t *m, const char *name, void **symbol)
{
	void *ptr;
	if (!m)
		return false;
	ptr = dlsym(m->handle, name);
	if (!ptr)
		return false;

	if (symbol)
		*symbol = ptr;
	return true;
}

#elif defined(_WIN32)

/*
 * The following code is mostly written by reading through
 * my /usr/i486-mingw/include/winbase.h and some documentation
 * that I read and remembered.
 *
 * NB, This code is also untested, I have not compiled it.  */
#include <windows.h>

mod_t *module_open(const char *filename)
{
	HINSTANCE h;
	mod_t *ret;

	h = LoadLibrary(TEXT(filename));
	if (!h)
		return NULL;

	xmalloc(ret, sizeof(*ret), FreeLibrary(h));
	ret->handle = h;
	ret->name = filename;
	return ret;
}

void module_close(mod_t *m)
{
	if (!m)
		return;

	free((void *)m->name);
	FreeLibrary((HINSTANCE)m->handle);
	free(m);
}

const char *module_error(void)
{
	int error = GetLastError();
	// what next?
}

bool module_symbol(mod_t *m, const char *name, void **symbol)
{
	void *ptr;
	if (!m)
		return false;

	ptr = GetProcAddress((HINSTANCE)m->handle, TEXT(name));
	if (!ptr)
		return false;
	if (symbol)
		*symbol = ptr;
	return true;
}

#endif    /* defined(__unix__) || (defined(__APPLE__) && defined(__MACH__) */

const char *module_name(mod_t *mod)
{
	return mod->name;
}

