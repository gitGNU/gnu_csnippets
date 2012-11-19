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

void modules_cleanup(struct module_list *modules)
{
    struct module *module;
    struct module_symbol *symbol;
    if (unlikely(!modules))
        return;

    for (;;) {
        module = list_top(&modules->children, struct module, node);
        if (!module)
            break;
        module_cleanup(module);
    }
    free(modules);
}

void module_cleanup(struct module *module)
{
    struct module_symbol *symbol;
    struct module *cpy;
    if (unlikely(!module))
        return;

    for (;;) {
        symbol = list_top(&module->children, struct module_symbol, node);
        if (!symbol)
            break;

        free(symbol->symbol_name);
        list_del_from(&module->children, &symbol->node);
        free(symbol);      
    }

    free(module);
}

