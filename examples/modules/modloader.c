#include <csnippets/module.h>
#include <errno.h>

static bool filter(const char *s)
{
	return strcmp(s, "mod_") != 0;
}

int main(int argc, char *argv[])
{
	mod_list mlist;
	struct mod *module;
	struct modsym *symbol;
	int ret;

	ret = read_moddir(argv[1] ?: ".", &mlist, filter);
	if (ret < 0)
		fatal("failed to load mods %s.\n", strerror(errno));
	if (ret == 0) {
		printf("No modules found in %s\n", argv[1] ?: ".");
		return 0;
	}

	list_for_each(&mlist, module, node) {
		list_for_each(&module->children, symbol, node) {
			void (*mod_fn) (void) = symbol->ptr;
			printf("Calling %s at location %p...\n", symbol->name,
					symbol->ptr);
			if (__builtin_expect(!!mod_fn, 1))
				(*mod_fn) ();
		}
	}

	cleanup_mods(&mlist);
	return ret;
}

