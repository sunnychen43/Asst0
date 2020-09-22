#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*-----------------------------OPERATORS-------------------------------------*/
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
        curr->name = malloc(strlen(name) + 1);
        strcpy(curr->name, name);
        curr->valid = true;
    }
    else {
        add_op(&(curr->children), op_chr+1, name);
    }

    return;
}

OP *search_op(OP *head, char c) {
    OP *curr = head;
    while (curr != NULL) {
        if (curr->c == c) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
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
/*----------------------------------------------------------------------------*/

/*------------------------------HASHTABLE-------------------------------------*/

#define HASHSIZE 37
typedef struct T_Entry {
	char *val;
    struct T_Entry *next;
} T_Entry;
static T_Entry *hashtab[HASHSIZE];


int hash(char *s) {
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++) {
    	hashval = *s + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

T_Entry *T_lookup(char *s) {
	T_Entry *p = hashtab[hash(s)];
	for (; p != NULL; p = p->next) {
		if (strcmp(s, p->val) == 0) {
			return p;
		}
	}
	return NULL;
}

T_Entry *T_add(char *name) {
	int hashval = hash(name);

	T_Entry *p = malloc(sizeof(T_Entry));
	p->next = hashtab[hashval];
	p->val = malloc(strlen(name)+1);
	strcpy(p->val, name);

	hashtab[hashval] = p;

	return p;
}

void T_free() {
	for (int i=0; i < HASHSIZE; i++) {
		T_Entry *p = hashtab[i];
		T_Entry *tmp;
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
    load();
    // printLL(op_LL, 0);

    char s[] = "+-";

    int i=0;
    while (i < strlen(s)) {
        char c = s[i];

        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            i++; continue;
        }


        OP *prev = NULL;
        OP *curr = op_LL;
        int j = i;
        for (; j < strlen(s); j++) {
            if (curr == NULL) {break;}

            char peek = s[j];
            OP *res = search_op(curr, peek);

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

    printf("%d\n", T_lookup("asdf") == NULL);
    T_add("asdf");
    printf("%d\n", T_lookup("asdf") == NULL);

    return 0;
}