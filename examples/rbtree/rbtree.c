/*
 * Red-Black Tree Test Example.
 *
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
#include <ctype.h>

static int compare(const void *a, const void *b)
{
	if (*(int *)a > *(int *)b) return 1;
	if (*(int *)a < *(int *)b) return -1;
	return 0;
}

static void destroy_key(void *a)
{

}

static void destroy_info(void *a)
{

}

static void print(const void *a)
{
	printf("%i", *(int *)a);
}

int main(void)
{
	rb_tree *tree;
	rb_node *node;
	int option = 0;
	int key;
	int *mKey;

	tree = rbtree_create(compare, destroy_key, destroy_info, print, print);
	if (!tree)
		return 1;

	while (option != 8) {
		printf("Choose from the following:\n");
		printf("(1) add to tree\n(2) delete from tree\n(3) query key\n");
		printf("(4) find predecessor\n(5) find sucessor\n");
		printf("(6) print tree\n(7) destroy tree and exit\n");

		do
			option = fgetc(stdin);
		while (option != -1 && isspace(option));
		option -= '0'; /* number conversion */

		switch (option) {
		case 1:
			printf("New Key: ");
			scanf("%i", &key);
			mKey = malloc(sizeof(int));
			*mKey = key;
			rbtree_insert(tree, mKey, 0);
			break;
		case 2:
			printf("Key to remove: ");
			scanf("%i", &key);
			node = rbtree_query(tree, &key);
			if (node)
				rbtree_remove(tree, node);
			else
				printf("Key %d not found.\n", key);
			break;
		case 3:
			printf("Key to query: ");
			scanf("%i", &key);
			node = rbtree_query(tree, &key);
			if (node)
				printf("Data found in tree at location %i\n", (int)node);
			else
				printf("Not found\n");
			break;
		case 4:
			printf("Key to predecessor: ");
			scanf("%i", &key);
			node = rbtree_query(tree, &key);
			if (node) {
				node = rbtree_predecessor(tree, node);
				if (node != tree->null)
					printf("Predecessor has key %i\n", *(int *)node->key);
				else
					printf("There is no predecessor for that node\n");
			} else
				printf("Data not in tree\n");
			break;
		case 5:
			printf("Key to successor: ");
			scanf("%i", &key);
			node = rbtree_query(tree, &key);
			if (node) {
				node = rbtree_successor(tree, node);
				if (node != tree->null)
					printf("Successor has key %i\n", *(int *)node->key);
				else
					printf("There is no successor for that node\n");
			} else
				printf("Data not in tree\n");
			break;
		case 6:
			rbtree_print(tree);
			break;
		case 7:
			rbtree_destroy(tree);
			return 0;
		default:
			printf("Invalid option, try again.\n");
			break;
		}
	}

	return 0;
}

