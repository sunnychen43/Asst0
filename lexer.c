#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*-----------------------------OPERATORS-------------------------------------*/
typedef struct OP {
    char c, *name;
    struct OP *children;
    struct OP *next;
} OP;

static OP *op_main;


OP *op_init() {
    OP *op = malloc(sizeof(*op));
    op->name = NULL;
    op->children = NULL;
    op->next = NULL;
    return op;
}

void op_add(OP **head, char *op_chr, char *name) {

    OP *prev = NULL;
    OP *curr = *head;
    bool found = false;

    /* Traverse through linked list until match or end */
    while (curr != NULL) {
        if (curr->c == op_chr[0]) {break;}
        prev = curr;
        curr = curr->next;
    }

    /* curr == NULL means no match, we need to init and insert a new op */
    if (curr == NULL) {
        /* If head is empty, insert there */
        if (*head == NULL) {
            *head = op_init();
            curr = *head;
        }
        /* Otherwise link with end of LL */
        else {
            prev->next = op_init();
            curr = prev->next; 
        }
        /* Set op char */
        curr->c = op_chr[0];
    }
    
    /* If op_chr terminates, set current op name (curr op may not have a name) */
    if (strlen(op_chr) == 1) {
        curr->name = malloc(strlen(name) + 1);
        strcpy(curr->name, name);
    }
    else {
        op_add(&(curr->children), op_chr+1, name);
    }

    return;
}

OP *op_search(OP *head, char c) {
    OP *curr = head;
    while (curr != NULL) {
        if (curr->c == c) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void op_load_file() {
    FILE *fp = fopen("newops.txt", "r");

    char op_chr[5], name[30];
    while (fscanf(fp, "%s\t%[^\n]s\n", op_chr, name) != EOF) {
        op_add(&op_main, op_chr, name);
    }
}

void op_print(OP *head, int level) { 
    OP *curr = head;
    while (curr != NULL) {
        for (int i=0; i < level; i++) {
            printf("\t");
        }
        printf("%c ", curr->c);
        if (curr->name != NULL) {
            printf("%s", curr->name);
        }
        printf("\n");
        op_print(curr->children, level+1);
        curr = curr->next;
    }
}
/*----------------------------------------------------------------------------*/

/*------------------------------HASHTABLE-------------------------------------*/

#define HASHSIZE 37
typedef struct HashItem {
	char *val;
    struct HashItem *next;
} HashItem;

static HashItem *hashtab[HASHSIZE];


int _hash(char *s) {
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++) {
    	hashval = *s + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

HashItem *ht_lookup(char *s) {
	HashItem *p = hashtab[_hash(s)];
	for (; p != NULL; p = p->next) {
		if (strcmp(s, p->val) == 0) {
			return p;
		}
	}
	return NULL;
}

HashItem *ht_add(char *name) {
	int hashval = _hash(name);

	HashItem *p = malloc(sizeof(HashItem));
	p->next = hashtab[hashval];
	p->val = malloc(strlen(name)+1);
	strcpy(p->val, name);

	hashtab[hashval] = p;

	return p;
}

void ht_free() {
	for (int i=0; i < HASHSIZE; i++) {
		HashItem *p = hashtab[i];
		HashItem *tmp;
		while (p != NULL) {
			tmp = p->next;
			free(p->val);
			free(p);
			p = tmp;
		}
	}
}
/*----------------------------------------------------------------------------*/

int main() {
    op_load_file();
    op_print(op_main, 0);

    char s[] = "+-";

    int i=0;
    while (i < strlen(s)) {
        char c = s[i];

        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            i++; continue;
        }


        OP *prev = NULL;
        OP *curr = op_main;
        int j = i;
        for (; j < strlen(s); j++) {
            if (curr == NULL) {break;}

            char peek = s[j];
            OP *res = op_search(curr, peek);

            if (res == NULL) {break;}

            prev = res;
            curr = res->children;
        }

        if (prev != NULL) {
            printf("%s\n", prev->name);
            i = j;
        }
        else {
            i++;
        }
    }

    printf("%d\n", ht_lookup("asdf") == NULL);
    ht_add("asdf");
    printf("%d\n", ht_lookup("asdf") == NULL);

    return 0;
}