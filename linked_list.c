#include <stdlib.h>
#include <stdio.h>

// Stolen from https://www.learn-c.org/en/Linked_lists

typedef struct node {
    int x;
    int y;
    struct node * next;
} node_t;

void print_list(node_t * head) {
    node_t * current = head;

    while (current != NULL) {
        printf("%d\n", current->x, "%d\n", current->y);
        current = current->next;
    }
}

void push(node_t ** head, int x, int y) {
    node_t * new_node;
    new_node = (node_t *) malloc(sizeof(node_t));

    new_node->x = x;
    new_node->y = y;

    new_node->next = *head;
    *head = new_node;
}
