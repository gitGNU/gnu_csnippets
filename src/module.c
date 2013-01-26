/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>.
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#include <csnippets/module.h>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))

#include <csnippets/asprintf.h>

#include <elf.h>
#include <dirent.h>
#include <dlfcn.h>
#include <ctype.h>
#include <errno.h>

struct marked {
	void *ptr;
	struct list_node n;
};
static LIST_HEAD(marked);

static __exit void free_mod_memory(void)
{
	struct marked *m, *next;
	list_for_each_safe(&marked, m, next, n) {
		dlclose(m->ptr);
		list_del_from(&marked, &m->n);
		free(m);
	}
}
#define MARK_FOR_DELETION(h)			\
	struct marked *m = malloc(sizeof(*m));	\
	if (m) {				\
		m->ptr = h;			\
		list_add_tail(&marked, &m->n);	\
	}

static __inline int filter(const struct dirent *dir)
{
	const char *name = dir->d_name;
	return strlen(name) > 3 ? !strcmp(name + strlen(name) - 3, ".so") : 0;
}

int readfn_module(const char *filename, struct modsym *s,
		const char *func)
{
	void *handle;
	char *tmpf;

	if (!asprintf(&tmpf, "./%s", filename))
		return -1;
	handle = dlopen(tmpf, RTLD_NOW);
	free(tmpf);
	if (!handle)
		return -1;
	s->ptr = dlsym(handle, func);
	s->name = (char *)func;

	MARK_FOR_DELETION(handle);
	return 0;
}

int read_module(const char *file, struct mod **mod,
                bool (*filter) (const char *))
{
	FILE *fp;
	struct mod *ret = NULL;
	struct modsym *msym = NULL;

	off_t size;
	int err = 0, i, j;
	unsigned int sym_loc = 0, symcount = 0;
	char **syms = NULL, *buffer,
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

	hdr  = (Elf32_Ehdr *)buffer;
	shdr = (Elf32_Shdr *)(buffer + hdr->e_shoff);

	for (i = 0; i < hdr->e_shnum; ++i) {
		if (shdr[i].sh_type == SHT_STRTAB
		    && strcmp(buffer + shdr[hdr->e_shstrndx].sh_offset + shdr[i].sh_name,
		              ".dynstr") == 0) {
			sym_loc = i;
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

			char *symbn = buffer + shdr[sym_loc].sh_offset + sym[j].st_name;
			if (filter && !filter(symbn))
				continue;

			xrealloc(syms, syms, (symcount + 1) * sizeof(char *),
			         err = -ENOMEM; goto cleanup);
			xmalloc(syms[symcount], strlen(symbn) + 1,
			        err = -ENOMEM; goto cleanup);

			strcpy(syms[symcount], symbn);
			++symcount;
		}
	}

	if (symcount <= 0) {
		*mod = NULL;
		return 0;
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

	xmalloc(ret, sizeof(struct mod), err = -ENOMEM; goto cleanup);
	list_head_init(&ret->children);

	for (i = 0; i < symcount; ++i) {
		xmalloc(sym, sizeof(*sym), err = -ENOMEM; goto cleanup);

		msym->ptr = dlsym(handle, syms[i]);
		if ((errorstr = dlerror())) {
#ifdef _DEBUG_MODULES
			dbg("error resolving symbol %s: %s\n", syms[i], errorstr);
#endif
			continue;
		}

		msym->name = syms[i];
		list_add_tail(&ret->children, &msym->node);
	}

cleanup:
	if (err && msym && ret) {
		struct modsym *tmp;
		list_for_each_safe(&ret->children, msym, tmp, node) {
			list_del_from(&ret->children, &msym->node);
			free(msym);
		}
	}

	if (syms) {
		for (i = 0; i < symcount; ++i)
			free(syms[i]);
		free(syms);
	}

	free(buffer);
	free(tmpf);
	if (mod)
		*mod = ret;
	else
		cleanup_module(ret);

	MARK_FOR_DELETION(handle);
	return err;
}

int read_moddir(const char *dir, mod_list *mods,
                bool (*filterp) (const char *))
{
	int so_count = 0;
	int ret = 0;
	struct dirent **so_list;
	struct mod *mod = NULL;

	so_count = scandir(dir, &so_list, filter, NULL);
	if (so_count <= 0)
		return 0;

	list_head_init(mods);
	while (so_count--) {
		if (read_module(so_list[so_count]->d_name, &mod, filterp) == 0
		     && mod) {
			++ret;
			list_add_tail(mods, &mod->node);
		}
		free(so_list[so_count]);
	}

	free(so_list);
	return ret;
}

#endif    /* defined(__unix__) || (defined(__APPLE__) && defined(__MACH__) */

void cleanup_module(struct mod *mod)
{
	struct modsym *sym, *next;
	if (unlikely(!mod))
		return;

	list_for_each_safe(&mod->children, sym, next, node) {
		free(sym->name);
		list_del_from(&mod->children, &sym->node);
		free(sym);
	}

	free(mod);
}

void cleanup_mods(mod_list *mods)
{
	struct mod *mod, *next;
	if (unlikely(!mods))
		return;

	list_for_each_safe(mods, mod, next, node) {
		list_del_from(mods, &mod->node);
		cleanup_module(mod);
	}
}

