/*
 * This Red-Black Tree implementation is shamelessly copied
 * and modified from http://web.mit.edu/~emin/www.old/source_code/red_black_tree/index.html
 * 
 * All Credits go to Emin for the implementation.
 *
 * License:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that neither the name of Emin
 * Martinian nor the names of any contributors are be used to endorse or
 * promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <csnippets/rbtree.h>

#include <stdlib.h>
#include <stdio.h>

static void rotate_left(rb_tree *tree, rb_node *a)
{
	rb_node *b;
	rb_node *null = tree->null;

	b = a->right;
	a->right = b->left;

	if (b->left != null)
		b->left->parent = a;
	b->parent = a->parent;
	if (a == a->parent->left)
		a->parent->left = b;
	else
		a->parent->right = b;

	b->left = a;
	a->parent = b;
}

static void rotate_right(rb_tree *tree, rb_node *b)
{
	rb_node *a;
	rb_node *null = tree->null;

	a = b->left;
	b->left = a->right;

	if (a->right != null)
		a->right->parent = b;
	a->parent = b->parent;
	if (b == b->parent->left)
		b->parent->left = a;
	else
		b->parent->right = a;

	a->right = b;
	b->parent = a;
}

static void insert_auxiliar(rb_tree *tree, rb_node *c)
{
	rb_node *a, *b;
	rb_node *null = tree->null;

	c->left = c->right = null;
	b = tree->root;
	a = tree->root->left;

	while (a != null) {
		b = a;
		if (tree->compare(a->key, c->key) == 1)
			a = a->left;
		else
			a = a->right;
	}

	c->parent = b;
	if (b == tree->root || tree->compare(b->key, c->key))
		b->left = c;
	else
		b->right = c;
}

static void rbtree_fixup(rb_tree *tree, rb_node *a)
{
	rb_node *root = tree->root;
	rb_node *w;

	while (!a->red && a != root) {
		if (a == a->parent->left) {
			w = a->parent->right;
			if (w->red) {
				w->red = false;
				w->parent->red = true;
				rotate_left(tree, a->parent);
				w = a->parent->right;
			}

			if (!w->right->red && !w->left->red) {
				w->red = true;
				a = a->parent;
			} else {
				if (!w->right->red) {
					w->left->red = false;
					w->red = true;
					rotate_right(tree, w);
					w = a->parent->right;
				}

				w->red = a->parent->red;
				a->parent->red = false;
				w->right->red = false;
				rotate_left(tree, a->parent);
				a = root;
			}
		} else {
			w = a->parent->left;
			if (w->red) {
				w->red = false;
				a->parent->red = true;
				rotate_right(tree, a->parent);
			}

			if (!w->right->red && !w->left->red) {
				w->red = true;
				a = a->parent;
			} else {
				if (!w->left->red) {
					w->right->red = false;
					w->red = true;
					rotate_left(tree, w);
					w = a->parent->left;
				}

				w->red = a->parent->red;
				a->parent->red = false;
				w->left->red = false;
				rotate_right(tree, a->parent);
				a = root;
			}
		}
	}

	a->red = false;
}

static void destroy_auxiliar(rb_tree *tree, rb_node *n)
{
	rb_node *null = tree->null;
	if (n != null) {
		destroy_auxiliar(tree, n->left);
		destroy_auxiliar(tree, n->right);
		tree->destroy_key(n->key);
		tree->destroy_info(n->info);
		free(n);
	}
}

static void _rbtree_print(rb_tree *tree, rb_node *node)
{
	rb_node *null = tree->null;
	rb_node *root = tree->root;

	if (node != null) {
		_rbtree_print(tree, node->left);

		printf("info=");
		if (node->info)
			tree->print_info(node->info);
		else
			printf("null");

		printf("   Key=");
		tree->print_key(node->key);

		printf("   Left-Key=");
		if (node->left != null)
			tree->print_key(node->left->key);
		else
			printf("null");

		printf("   Right-Key=");
		if (node->right != null)
			tree->print_key(node->right->key);
		else
			printf("null");

		printf("   Parent-Key=");
		if (node->parent != root)
			tree->print_key(node->parent->key);
		else
			printf("null");

		printf("   red=%i\n", node->red);
		_rbtree_print(tree, node->right);
	}
}

rb_tree *rbtree_create(int (*compare) (const void *, const void *),
			void (*destroy_key) (void *),
			void (*destroy_info) (void *),
			void (*print_key) (const void *),
			void (*print_info) (const void *))
{
	rb_tree *ret;
	rb_node *tmp;

	ret = malloc(sizeof(*ret));
	if (!ret)
		return NULL;

	ret->compare	  = compare;
	ret->destroy_key  = destroy_key;
	ret->destroy_info = destroy_info;
	ret->print_key    = print_key;
	ret->print_info   = print_info;

	tmp = malloc(sizeof(*tmp));
	if (!tmp) {
		free(ret);
		return NULL;
	}

	tmp->parent = tmp->left = tmp->right;
	tmp->red = false;
	tmp->key = NULL;
	ret->null = tmp;

	tmp = malloc(sizeof(*tmp));
	if (!tmp) {
		free(ret->null);
		free(ret);
		return NULL;
	}

	tmp->parent = tmp->left = tmp->right = ret->null;
	tmp->red = false;
	tmp->key = NULL;
	ret->root = tmp;

	return ret;
}

void rbtree_destroy(rb_tree *tree)
{
	destroy_auxiliar(tree, tree->root->left);
	free(tree->root);
	free(tree->null);
	free(tree);
}

rb_node *rbtree_insert(rb_tree *tree, void *key, void *info)
{
	rb_node *a, *b;
	rb_node *node;

	a = malloc(sizeof(*a));
	if (!a)
		return NULL;
	a->key = key;
	a->info = info;

	insert_auxiliar(tree, a);
	node = a;
	a->red = true;

	while (a->parent->red) {
		if (a->parent == a->parent->parent->left) {
			b = a->parent->parent->right;
			if (b->red) {
				a->parent->red = false;
				b->red = false;
				a->parent->parent->red = true;
			} else {
				if (a == a->parent->right) { 
					a = a->parent;
					rotate_left(tree, a);
				}

				a->parent->red = false;
				a->parent->parent->red = true;
				rotate_right(tree, a->parent->parent);
			}
		} else {
			b = a->parent->parent->left;
			if (b->red) {
				a->parent->red = false;
				b->red = false;
				a->parent->parent->red = true;
				a = a->parent->parent;
			} else {
				if (a == a->parent->left) {
					a = a->parent;
					rotate_right(tree, a);
				}

				a->parent->red = false;
				a->parent->parent->red = true;
				rotate_left(tree, a->parent->parent);
			}
		}
	}

	tree->root->left->red = false;
	return node;
}

void rbtree_remove(rb_tree *tree, rb_node *c)
{
	rb_node *a, *b;
	rb_node *null = tree->null;
	rb_node *root = tree->root;

	b = ((c->left == null) || (c->right == null) ? c : rbtree_successor(tree, c));
	a = b->left == null ? b->right : b->left;

	if (root == (a->parent = b->parent))
		root->left = a;
	else {
		if (b == b->parent->left)
			b->parent->left = a;
		else
			b->parent->right = a;
	}

	if (b != c) {
		if (!b->red)
			rbtree_fixup(tree, a);

		tree->destroy_key(c->key);
		tree->destroy_info(c->info);
		b->left = c->left;
		b->right = c->right;
		b->parent = c->parent;
		b->red = c->red;
		c->left->parent = c->right->parent = b;
		if (c == c->parent->left)
			c->parent->left = b;
		else
			c->parent->right = b;
		free(c);
	} else {
		tree->destroy_key(b->key);
		tree->destroy_info(b->info);
		if (!b->red)
			rbtree_fixup(tree, a);
		free(b);
	}
}

rb_node *rbtree_successor(rb_tree *tree, rb_node *a)
{
	rb_node *b;
	rb_node *null = tree->null;
	rb_node *root = tree->root;

	if (null != (b = a->right)) {
		while (b->left != null)
			b = b->left;
		return b;
	}

	b = a->parent;
	while (a == b->right) {
		a = b;
		b = b->parent;
	}

	if (b == root)
		return null;
	return b;
}

rb_node *rbtree_predecessor(rb_tree *tree, rb_node *a)
{
	rb_node *b;
	rb_node *null = tree->null;
	rb_node *root = tree->root;

	if (null != (b = a->left)) {
		while (b->right != null)
			b = b->right;
		return b;
	}

	b = a->parent;
	while (a == b->left) {
		if (b == root)
			return null;
		a = b;
		b = b->parent;
	}

	return b;
}

rb_node *rbtree_query(rb_tree *tree, void *query)
{
	rb_node *a = tree->root->left;
	rb_node *null = tree->null;
	int comp_val;

	if (a == null)
		return NULL;

	comp_val = tree->compare(a->key, query);
	while (comp_val != 0) {
		if (comp_val == 1) /* a->key > query */
			a = a->left;
		else
			a = a->right;
		if (a == null)
			return NULL;
		comp_val = tree->compare(a->key, query);
	}

	return a;
}

void rbtree_print(rb_tree *tree)
{
	_rbtree_print(tree, tree->root->left);
}

