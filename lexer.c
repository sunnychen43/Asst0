#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct OP {
    char c, *name;
    bool valid;
    struct OP *children;
    struct OP *next;
} OP;

static OP *op_LL;

OP *init_op() {
    OP *op = malloc(sizeof(*op));
    op->name = NULL;
    op->valid = false;
    op->children = NULL;
    op->next = NULL;
    return op;
}

void set_name(OP *op, char *name) {
    op->name = malloc(strlen(name) + 1);
    strcpy(op->name, name);
    op->valid = true;
}

void add_op(OP **head, char *op_chr, char *name) {

    OP *prev = NULL;
    OP *curr = *head;
    bool found = false;

    /* Traverse through linked list until match or end */
    while (curr != NULL) {
        if (curr->c == op_chr[0]) {
            found = true;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    if (!found) {
        if (*head == NULL) {
            *head = init_op();
            curr = *head;
        }
        else {
            prev->next = init_op();
            curr = prev->next; 
        }
        curr->c = op_chr[0];
    }
    
    if (strlen(op_chr) == 1) {
        set_name(curr, name);
    }
    else {
        add_op(&(curr->children), op_chr+1, name);
    }

    return;
}

void load() {
    FILE *fp = fopen("newops.txt", "r");

    char op_chr[5], name[30];
    while (fscanf(fp, "%s\t%[^\n]s\n", op_chr, name) != EOF) {
        add_op(&op_LL, op_chr, name);
    }
}

void printLL(OP *head, int level) { 
    OP *curr = head;
    while (curr != NULL) {
        for (int i=0; i < level; i++) {
            printf("\t");
        }
        printf("%c ", curr->c);
        if (curr->valid) {
            printf("%s", curr->name);
        }
        printf("\n");
        printLL(curr->children, level+1);
        curr = curr->next;
    }
}

int main() {
    load();
    printLL(op_LL, 0);

    return 0;
}