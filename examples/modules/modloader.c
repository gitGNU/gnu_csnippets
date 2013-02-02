#include <csnippets/module.h>

int main(int argc, char *argv[])
{
	struct mod *m;
	void (*init) (int, char **);
	int (*deinit) (void);

	m = module_open(argv[1]);
	if (!m) {
		fprintf(stderr, "unable to open module %s: %s\n", argv[1], module_error());
		return 1;
	}

	if (!module_symbol(m, "init", (void **)&init)) {
		fprintf(stderr, "unable to find init function: %s\n", module_error());
		return 1;
	}

	init(argc - 2, &argv[2]);
	if (!module_symbol(m, "deinit", (void **)&deinit)) {
		fprintf(stderr, "unable to find deinit function: %s\n", module_error());
		return 1;
	}

	return deinit();
}

