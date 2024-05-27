/**
 * AVL tree data structure test using integers.
 * Copyright (C) 2024 Lazar Razvan.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree/avl_tree.h"


/*****************************************************************************/

// custom avl tree function operations
void *avl_tree_alloc(void *data)
{
	void *d;

	d = malloc(1 * sizeof(int));
	if (!d)
		return NULL;

	*(int *)d = *(int *)data;

	return d;
}

void avl_tree_free(void *data)
{
	free(data);
}

int avl_tree_compare(void *data1, void *data2)
{
	int val1 = *(int *)data1;
	int val2 = *(int *)data2;

	return val1 - val2;
}

void avl_tree_print(void *data)
{
	int val = *(int *)data;

	printf("%d\n", val);
}


/*****************************************************************************/


int main()
{
	avl_tree_entry *avl_tree;
	int data[] = {
		10, 20, 30, 40, 50, 60, 70, 80, 90, 100,
		110, 120, 5, 15, 25, 35, 45, 55, 65, 75,
		85, 95, 105, 115, 125, 4, 6, 14, 16, 24,
		26, 34, 36, 44, 46, 54, 56, 64, 66, 74,
		76, 84, 86, 94, 96, 104, 106, 114, 116, 124
    };
    int data_size = sizeof(data) / sizeof(data[0]);

	// create AVL
	avl_tree = avl_tree_create(avl_tree_alloc,
								avl_tree_free,
								avl_tree_compare,
								avl_tree_print);
	if (!avl_tree) {
		fprintf(stderr, "Fail to create AVL tree!\n");
		goto finish;
	}

	// print AVL level order
	avl_tree_level_order_print(avl_tree);

	// populate the AVL
	for (int i = 0; i < data_size; i++) {
		if (avl_tree_insert(avl_tree, &data[i])) {
			fprintf(stderr, "Fail to add node %d [%d]!\n", i, data[i]);
			goto tree_destroy;
		}
	}

	// print AVL level order
	avl_tree_level_order_print(avl_tree);

	// Expected output
	//80 40 100 20 60 90 110 10 30 50 70 85 95 105 120 5 15 25 35 45 55 65 75
	//84 86 94 96 104 106 115 125 4 6 14 16 24 26 34 36 44 46 54 56 64 66 74 76
	//114 116 124

tree_destroy:
	avl_tree_destroy(avl_tree);
finish:
	return 0;
}
