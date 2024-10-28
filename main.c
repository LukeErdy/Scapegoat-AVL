#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX 64
double ALPHA = 0.667; // 2/3
int capacity = 200000;

typedef struct Node {
	char* USERNAME;
	char** SERVERID;
	char* UNIX_TIME_OF_BAN;
	struct Node* parent;
	struct Node* left;
	struct Node* right;
	int count;
	int height;
} node;

node* root = NULL;
int tree_size = 0;




void inorder(node* point, int* size, node* ordered_list) {
	if (point == NULL) return;
	if (point->left) inorder(point->left, size, ordered_list);

	if (ordered_list) ordered_list[*size] = *point;
	(*size)++;

	if (point->right) inorder(point->right, size, ordered_list);
	return;
}

void preorder(node* point) {
	if (point == NULL) {
		printf("_ ");
		return;
	};
	printf("%s\n", point->USERNAME);
	printf("(");
	preorder(point->left);
	preorder(point->right);
	printf(")\n");

	return;
}

int imax(int x, int y) {
	return (x > y) ? x : y;
}

int get_height(node* point) {
	if (!point) return 0;
	return point->height;
}

int insert_BST(node* new_node, node* subtree) {
	int depth = 0;
	if (root == NULL) {
		root = new_node;
		return depth;
	}
	node* point = subtree;
	while (1) {
		point->height = 1 + imax(get_height(point->left), get_height(point->right));
		if (!strcmp(point->USERNAME, new_node->USERNAME)) {
			for (int i = 0; i < MAX; i++) {
				if (point->SERVERID[i] && !strcmp(point->SERVERID[i], new_node->SERVERID[0])) break;
				if (point->SERVERID[i] == NULL) {
					point->SERVERID[i] = malloc(MAX * sizeof(char));
					strcpy(point->SERVERID[i], new_node->SERVERID[0]);
					point->count += 1;
					break;
				}
			}

			if (atoi(new_node->UNIX_TIME_OF_BAN) > atoi(point->UNIX_TIME_OF_BAN)) strcpy(point->UNIX_TIME_OF_BAN, new_node->UNIX_TIME_OF_BAN);
			break;
		} else if (strcmp(point->USERNAME, new_node->USERNAME) > 0) {
			depth++;
			if (point->left) point = point->left;
			else {
				point->left = new_node;
				new_node->parent = point;
				tree_size++;
				break;
			}
		} else {
			depth++;
			if (point->right) point = point->right;
			else {
				point->right = new_node;
				new_node->parent = point;
				tree_size++;
				break;
			}
		}
	}
	//preorder(root);
	return depth;
}


node* insert_BST_sorted(node* ordered_list, int start, int end) {
	if (start > end) return NULL;
	int median = (start + end) / 2;

	ordered_list[median].left = insert_BST_sorted(ordered_list, start, median - 1);
	if (ordered_list[median].left) ordered_list[median].left->parent = &ordered_list[median];

	ordered_list[median].right = insert_BST_sorted(ordered_list, median + 1, end);
	if (ordered_list[median].right) ordered_list[median].right->parent = &ordered_list[median];

	return &ordered_list[median];
}


void left_rotate(node* point) {
	node* temp = point->right;
	node* tree2 = temp->left;
	temp->left = point;
	point->right = tree2;
	temp->parent = point->parent;

	if (point->parent) {
		if (point->parent->left == point) point->parent->left = temp;
		else point->parent->right = temp;
	} else root = temp;

	point->parent = temp;
	if (tree2) tree2->parent = point;
	// heights
	point->height = 1 + imax(get_height(point->left), get_height(point->right));
	temp->height = 1 + imax(get_height(temp->left), get_height(temp->right));
}

void right_rotate(node* point) {
	node* temp = point->left;
	node* tree2 = temp->right;
	temp->right = point;
	point->left = tree2;
	temp->parent = point->parent;

	if (point->parent) {
		if (point->parent->left == point) point->parent->left = temp;
		else point->parent->right = temp;
	} else root = temp;

	point->parent = temp;
	if (tree2) tree2->parent = point;
	// heights
	point->height = 1 + imax(get_height(point->left), get_height(point->right));
	temp->height = 1 + imax(get_height(temp->left), get_height(temp->right));
}


int main(int argc, char** argv) {
	if (argc == 3 && strcmp(argv[1], "scapegoat") && strcmp(argv[1], "avl")) exit(1);
	FILE* griefers;
	if (argc == 2) griefers = fopen(argv[1], "r");
	else if (argc == 3) griefers = fopen(argv[2], "r");

	int struct_index = 0;
	char* buffer = malloc(MAX * sizeof(char)); // buffer for char* fields
	node* new_node = NULL; // initialize

	while (fscanf(griefers, "%s", buffer) == 1) {
		if (struct_index == 0) {
			new_node = calloc(1, sizeof(node)); // first field, new node is necessary, calloc because relatives should be NULL by default
			new_node->USERNAME = malloc((strlen(buffer) + 1) * sizeof(char));
			strcpy(new_node->USERNAME, buffer);
		} else if (struct_index == 1) {
			new_node->SERVERID = calloc(MAX, sizeof(char*));
			new_node->SERVERID[0] = malloc((strlen(buffer) + 1) * sizeof(char));
			strcpy(new_node->SERVERID[0], buffer);
		} else if (struct_index == 2) {
			new_node->UNIX_TIME_OF_BAN = malloc((strlen(buffer) + 1) * sizeof(char));
			strcpy(new_node->UNIX_TIME_OF_BAN, buffer);
			struct_index = -1;

			new_node->height = 1;
			int depth = insert_BST(new_node, root); //printf("depth: %d\n", depth);

			if ((!strcmp(argv[1], "scapegoat") || argc == 2) && depth >= floor(log(tree_size) / log(1 / ALPHA)) + 1) {
				node* ordered_list = malloc(capacity * sizeof(node));
				int size = 0;
				inorder(root, &size, ordered_list);
				root = insert_BST_sorted(ordered_list, 0, size - 1);
			} else if (!strcmp(argv[1], "avl")) {
				node* point = new_node;
				while (point) {
					if (abs(get_height(point->left) - get_height(point->right)) > 1) {
						// point is unbalanced node
						// rotate
						if (get_height(point->left) > get_height(point->right)) { // right rotate, update heights
							right_rotate(point);
						} else if (get_height(point->left) < get_height(point->right)) { // left rotate, update heights
							left_rotate(point);
						}
						//printf("point is unbalanced. left has height %d and right has height %d\n", get_height(point->left), get_height(point->right));
						break;
					}
					point = point->parent;
				}
			}
		}
		struct_index++;
	}
	
	while (fscanf(stdin, "%s", buffer) == 1) {
		node* point = root;
		while (point) {
			if (!strcmp(point->USERNAME, buffer)) {
				printf("%s was banned from %d servers. most recently on: %s\n", buffer, point->count + 1, point->UNIX_TIME_OF_BAN);
				break;
			} else if (strcmp(point->USERNAME, buffer) > 0) point = point->left;
			else point = point->right;
		}
		if (!point) printf("%s is not currently banned from any servers.\n", buffer);
	}
	return 0;
}