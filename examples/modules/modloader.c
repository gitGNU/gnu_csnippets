#include <csnippets/module.h>

static bool filter(const char *s)
{
	return strcmp(s, "module_") != 0;
}

int main(int argc, char **argv)
{
	struct module_list *mlist;
	struct module *module;
	struct module_symbol *symbol;
	int ret;

	ret = modules_load(".", &mlist, filter);
	if (ret)
		fatal("failed to load modules %s.\n", strerror(-ret));

	list_for_each(&mlist->children, module, node) {
		list_for_each(&module->children, symbol, node) {
			void (*mod_fn) (void) = symbol->func_ptr;
			printf("Calling %s at location %p...\n", symbol->symbol_name,
					symbol->func_ptr);
			if (__builtin_expect(!!mod_fn, 1))
				(*mod_fn) ();
		}
	}

	modules_cleanup(mlist);
	return ret;
}

