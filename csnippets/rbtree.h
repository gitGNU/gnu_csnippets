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

#ifndef _RBTREE_H
#define _RBTREE_H

typedef struct rb_node {
	void *key;
	void *info;
	bool red;

	struct rb_node *left;
	struct rb_node *right;
	struct rb_node *parent;
} rb_node;

typedef struct rb_tree {
	rb_node *root;
	rb_node *null;

	int (*compare) (const void *, const void *);
	void (*destroy_key) (void *);
	void (*destroy_info) (void *);

	void (*print_key) (const void *);
	void (*print_info) (const void *);
} rb_tree;

extern rb_tree *rbtree_create(int (*compare) (const void *, const void *),
			      void (*destroy_key) (void *),
			      void (*destroy_info) (void *),
			      void (*print_key) (const void *),
			      void (*print_info) (const void *));
extern void rbtree_destroy(rb_tree *);

extern rb_node *rbtree_insert(rb_tree *, void *key, void *info);
extern void rbtree_remove(rb_tree *, rb_node *);
extern rb_node *rbtree_query(rb_tree *tree, void *query);

extern rb_node *rbtree_successor(rb_tree *, rb_node *);
extern rb_node *rbtree_predecessor(rb_tree *, rb_node *);

extern void rbtree_print(rb_tree *);

#endif

