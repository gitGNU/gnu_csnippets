/*
 * Copyright (c) 2012 Allan Ference  <f.fallen45@gmail.com>
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
#include <csnippets/module.h>

int main(int argc, char **argv)
{
    struct module_list *mlist;
    struct module *module;
    struct module_symbol *symbol;
    int ret;

    ret = modules_load(".", &mlist, "module_");
    if (ret)
        fatal("failed to load modules %s.\n", strerror(errno));

    list_for_each(&mlist->children, module, node) {
        list_for_each(&module->children, symbol, node) {
            void (*mod_fn) (void) = symbol->func_ptr;
            printf("Calling %s at location %p...\n", symbol->symbol_name,
                    symbol->func_ptr);
            if (__builtin_expect(!!mod_fn, 0))
                (*mod_fn) ();
        }
    }

    modules_cleanup(mlist);
    return ret;
}

