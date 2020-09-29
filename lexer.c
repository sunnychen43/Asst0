#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

/*-----------------------------OPERATORS-------------------------------------*/
typedef struct OP {
    char c, *name;
    struct OP *children;
    struct OP *next;
} OP;

static OP *op_main;


OP *op_init() {
    OP *op = malloc(sizeof(*op));
    if (op == NULL) {
        printf("malloc op failed. Aborting...\n");
        exit(EXIT_FAILURE);
    }

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
        if (curr->name == NULL) {
            printf("malloc op->name failed. Aborting...\n");
            exit(EXIT_FAILURE);
        }
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

    fclose(fp);
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

#define HASHSIZE 43  /* 32 expected entries, hashsize = prime close to size*1.3 */
typedef struct HashItem {
	char *val;
    struct HashItem *next;
} HashItem;

static HashItem *ht_table[HASHSIZE];


int _hash(char *s) {
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++) {
    	hashval = *s + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

HashItem *ht_lookup(char *s) {
	HashItem *p = ht_table[_hash(s)];
	for (; p != NULL; p = p->next) {
		if (strcmp(s, p->val) == 0) {
			return p;
		}
	}
	return NULL;
}

HashItem *ht_add(char *name) {
    HashItem *p = malloc(sizeof(HashItem));
    if (p == NULL) {
        printf("malloc ht failed. Aborting...\n");
        exit(EXIT_FAILURE);
    }

	p->val = malloc(strlen(name)+1);
    if (p->val == NULL) {
        printf("malloc ht->val failed. Aborting...\n");
        exit(EXIT_FAILURE);
    }
	strcpy(p->val, name);

    int hashval = _hash(name);
    p->next = ht_table[hashval];
	ht_table[hashval] = p;

	return p;
}

void ht_free() {
	for (int i=0; i < HASHSIZE; i++) {
		HashItem *tmp, *p = ht_table[i];
		while (p != NULL) {
			tmp = p->next;
			free(p->val);
			free(p);
			p = tmp;
		}
	}
}
/*----------------------------------------------------------------------------*/

/*--------------------------------NUMBER--------------------------------------*/

int oneOf(char a, char* b) { //checks if a is contained in b, returns 1 if so, 0 otherwise
    int i = 0;
    int length = strlen(b); //be sure that b includes a nul terminator
    for (i = 0; i < length; i++) {
        if (a == b[i]) {
            return 1;
        }
    }
    return 0;
}

bool is_dec(char c) {
    return c <= '9' && c >= '0';
}

int is_hex(char a) { //checks if hex
    if (('0' <= a && a <='9') || ('a' <= a && a <='f') || ('A' <= a && a <='F')) {
        return 1;
    }
    return 0;
}

int hex(char* arg, int index) { //returns -1 if no hex found, returns new starting index if hex found
    int idx = index;
    char* firstTwo = malloc(3*sizeof(char)); //firstTwo holds the first two char from arg, starting at index
    firstTwo[0] = arg[idx++];
    firstTwo[1] = arg[idx++]; //i need to check for null pointer here
    firstTwo[2] = '\0';
    if (strcmp(firstTwo, "0x") && strcmp(firstTwo, "0X")) {
        return index; //not a hex
    }
    //congrats, we have a hex, how long is it?
    printf("hex integer ");
    printf("%s", firstTwo);
    while (is_hex(arg[idx])) {
        printf("%c", arg[idx]);
        idx++;
    }
    printf("\n");
    free(firstTwo);
    return idx; //this was all coded assuming 0x is a hex, change if otherwise
}

int oct(char* arg, int index) { //returns -1 if no octal, new index if octal is found
    int idx = index;
    char* acceptable = "12345670";
    char* decimal = "89";
    if (!(arg[idx] == '0')) {
        return index; //not a hex
    }
    //congrats, we have a oct, how long is it?
    while (oneOf(arg[idx], acceptable)) { //just realized, why doesn't this go over the size of arg?
        idx++;
    }
    if (oneOf(arg[idx], decimal)) {
        return index;
    }
    if (arg[idx] == '.' && oneOf(arg[++idx], "1234567890")) { //need to include e-24 is 5e-4 is a decimal
        return index;
    }
    else {
        printf("octal integer ");
        for (int i = index; i < idx; i++) {
            printf("%c", arg[i]);
        }
        printf("\n");
        return idx; //this was all coded assuming 0x is a hex, change if otherwise
    }
    return -1;
}

int scan_dec(char* arg, int index) {
    int idx = index;
    if (!is_dec(arg[idx])) {
        return index; //not a decimal or float
    }
    //ok we start with a number
    while (is_dec(arg[idx])) {
        idx++;
    }
    if (arg[idx] == '.' && is_dec(arg[idx+1])) { //need to include e-24 is 5e-4 is a decimal
        return index;
    }

    printf("decimal ");
    for (int i = index; i < idx; i++) {
        printf("%c", arg[i]);
    }
    printf("\n");
    return idx;
}

int scan_float(char* arg, int index) { //bug rn with consecutive decimals and no space, ie 324.324.234
    int idx = index;
    bool has_decpoint = false; //boolean, 0 if decimal point not found yet
    if (!is_dec(arg[idx])) {
        return index;
    }
    //ok we have a float
    printf("float ");
    while (is_dec(arg[idx])) {
        printf("%c", arg[idx]);
        idx++;
    }

    if (arg[idx] == '.') {
        printf(".");
        idx++; //increment so we're past the decimal point
        while (is_dec(arg[idx])) {
            printf("%c", arg[idx]);
            idx++;
        }
        printf("\n");
        return idx; 
    }
    
    return -1;
}
/*----------------------------------------------------------------------------*/

/*---------------------------------WORD---------------------------------------*/
void word_load_file() {
    FILE *fp = fopen("reserved.txt", "r");

    char buf[10];
    while (fscanf(fp, "%s\n", buf) != EOF) {
        ht_add(buf);
    }
}
/*----------------------------------------------------------------------------*/

void scan(char *s) {
    int i=0;
    while (i < strlen(s)) {
        char c = s[i];

        /* skip comments */
        if (c == '/' && s[i+1] == '*') {
            bool found = false;
            int j=i+2;
            for (; j < strlen(s)-1; j++) {
                if (s[j] == '*' && s[j+1] == '/') {
                    found = true;
                    break;
                }
            }
            if (found) {
                i = j+2;
                continue;
            }
        }

        /* skip quotes */
        if (c == '\"' || c == '\'') {
            bool found = false;
            int j = i+1;
            for (; j < strlen(s); j++) {
                if (s[j] == c) {
                    found = true;
                    break;
                }
            }

            if (found) {
                printf("string \"%.*s\"\n", j-i-1, s+i+1);
                i = j+1;
                continue;
            }
        }


        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            i++; continue;
        }

        /* Detect operator */
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
        /* OP found, update index and print op */
        if (prev != NULL) {
            printf("%s %.*s\n", prev->name, j-i, s+i); /* '.*' specifies length of string to print */
            i = j;
            continue;
        }

        /* Number */
        /* Array of function pointers*/
        int (*f_ptr[])(char *, int) = {&hex, &oct, &scan_dec, &scan_float}; 
        int peek_ind;
        for (int j=0; j < 4; j++) {
            peek_ind = (f_ptr[j])(s, i);
            if (peek_ind > i) {
                i = peek_ind-1;
                break;
            }
        }

        /* Word */
        if (isalpha(c)) {
            int j=i+1;
            while (j < strlen(s)) {
                if (!isalnum(s[j])) {break;}
                j++;
            }

            /* check for reserved word */
            char *word = malloc(j-i+1);
            if (word == NULL) {
                printf("malloc word failed. Aborting...\n");
                exit(EXIT_FAILURE);
            }
            
            strncpy(word, s+i, j-i);
            word[j-i] = '\0';

            if (ht_lookup(word) != NULL) {  /* keyword found */
                printf("keyword \"%s\"\n", word, word);
            }
            else {
                printf("word \"%s\"\n", word);
            }

            i = j;
            continue;
        }

        i++;
    }
}

int main() {
    op_load_file();
    word_load_file();
    // op_print(op_main, 0);

    char s[] = "0X111";
    scan(s);

    return 0;
}
