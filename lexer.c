#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

/*-----------------------------OPERATORS-------------------------------------*/
/* Defines the operator structure "OP", that contains a char, and pointers to two OP structures, children and next. */
typedef struct OP {
    char c, *name;
    struct OP *children;
    struct OP *next;
} OP;

static OP *op_main;

/* Initializes op structure using malloc and sets all members to null before returning a pointer to the structure */
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

/* Initializes op structure using malloc and sets all members to null before returning a pointer to the structure */
void op_add(OP **head, const char *op_chr, const char *name) {
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
    // FILE *fp = fopen("newops.txt", "r");

    // char op_chr[5], name[30];
    // while (fscanf(fp, "%s\t%[^\n]s\n", op_chr, name) != EOF) {
    //     op_add(&op_main, op_chr, name);
    // }

    const char *OP_DATA[43] = {
                            "( left parenthesis", ") right parenthesis", "[ left bracket", "] right bracket", 
                            ". structure member", "-> structure pointer", ", comma", "! negate", "~ 1s complement", ">> shift right", 
                            "<< shift left", "^ bitwise XOR", "| bitwise OR", "++ increment", "-- decrement", 
                            "+ addition", "/ division", "|| logical OR", "&& logical AND", "? conditional true", 
                            ": conditional false", "== equality test", "!=  inequality test", "< less than test", "> greater than test", 
                            "<= less than or equal test", ">= greater than or equal test", "= assignment", "+= plus equals", "-= minus equals", 
                            "*= times equals", "/= divide equals", "\%= mod equals", ">>= shift right equals", "<<= shift left equals", 
                            "&= bitwise AND equals", "^= bitwise XOR equals", "|= bitwise OR equals", "& AND/address operator",  
                            "- minus/subtract operator", "* multiply/dereference operator", "\" double quote", "\' single quote"
                        };
    
    char op_chr[5], name[30];

    for (int i=0; i < 43; i++) {
        sscanf(OP_DATA[i], "%s %[^\n]s", op_chr, name);
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

#define HASHSIZE 43  /* 32 expected entries, hashsize = prime close to size*1.3 */
typedef struct HashItem {
	char *val;
    struct HashItem *next;
} HashItem;

static HashItem *ht_table[HASHSIZE];


int _hash(const char *s) {
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++) {
    	hashval = *s + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

HashItem *ht_lookup(const char *s) {
	HashItem *p = ht_table[_hash(s)];
	for (; p != NULL; p = p->next) {
		if (strcmp(s, p->val) == 0) {
			return p;
		}
	}
	return NULL;
}

HashItem *ht_add(const char *name) {
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
/* Returns true if char c is an acceptable character for decimals */
bool is_dec(char c) {
    return (c <= '9') && (c >= '0');
}

/* Returns true if char c is an acceptable character for hexadecimals */
bool is_hex(char c) { //checks if hex
    return ('0' <= c && c <='9') || ('a' <= c && c <='f') || ('A' <= c && c <='F');
}

/* Returns true if char c is an acceptable character for octals */
bool is_oct(char c) {
    return c <= '7' && c >= '0';
}

/* checks if the next available token is a hexadecimal, and returns the index the next token starts at.
Will return the same index inputed if no hexadecimal token is found */
int scan_hex(const char* arg, int index) {
    int idx = index;

    if (arg[idx] != '0' || (arg[idx+1] != 'x' && arg[idx+1] != 'X')) {
        /* not a hexadecimal */
        return index;
    }

    /* hexadecimal token has been found, print out the hexadecimal identified 0x or 0X */
    printf("hex \"");
    printf("%c%c", arg[idx], arg[idx+1]);
    idx += 2;
    /* print all acceptable hexadecimal values, note there could be no values */
    while (is_hex(arg[idx])) {
        printf("%c", arg[idx]);
        idx++;
    }
    printf("\"\n");
    return idx;
}

/* checks if the next available token is a octal, and returns the index the next token starts at.
Will return the same index inputed if no octal token is found */
int scan_oct(const char* arg, int index) {
    int idx = index;
    if (arg[idx] != '0') {
        /* not an octal */
        return index;
    }
    /* octal token found */
    while (is_oct(arg[idx])) {
        idx++;
    }
    /* checks for edge case where token is decimal, not octal */
    if (arg[idx] == '8' || arg[idx] == '9') {
        return index;
    }
    /* checks for edge case where token is float, not octal */
    else if (arg[idx] == '.' && is_dec(arg[idx+1])) {
        return index;
    }
    /* case where token is octal, and is ended upon reaching a nonoctal character */
    else {
        printf("octal \"");
        for (int i = index; i < idx; i++) {
            printf("%c", arg[i]);
        }
        printf("\n\"");
        return idx;
    }
}

/* checks if the next available token is a decimal, and returns the index the next token starts at.
Will return the same index inputed if no decimal token is found */
int scan_dec(const char* arg, int index) {
    int idx = index;
    if (!is_dec(arg[idx])) {
        /* not a decimal or float */
        return index; 
    }
    /* continues incrementing idx until a nondecimal character is found */
    while (is_dec(arg[idx])) {
        idx++;
    }
    /* found a float */
    if (arg[idx] == '.' && is_dec(arg[idx+1])) {
        return index;
    }
    /* found a decimal */
    printf("decimal \"");
    for (int i = index; i < idx; i++) {
        printf("%c", arg[i]);
    }
    printf("\"\n");
    return idx;
}

/* checks if the next available token is a float, and returns the index the next token starts at.
Will return the same index inputed if no float token is found */
int scan_float(const char* arg, int index) {
    int idx = index;
    /* boolean denoting if a decimal point has been observed in the token*/
    bool has_decpoint = false;
    if (!is_dec(arg[idx])) {
        return index;
    }
    /* float found */
    printf("float \"");
    while (is_dec(arg[idx])) {
        printf("%c", arg[idx]);
        idx++;
    }
    /* decimal point found */
    if (arg[idx] == '.') {
        printf(".");
        idx++;
        while (is_dec(arg[idx])) {
            printf("%c", arg[idx]);
            idx++;
        }
        printf("\"\n");
        return idx; 
    }
    
    return -1;
}
/*----------------------------------------------------------------------------*/

/*---------------------------------WORD---------------------------------------*/
void word_load_file() {
    // FILE *fp = fopen("reserved.txt", "r");

    // char buf[10];
    // while (fscanf(fp, "%s\n", buf) != EOF) {
    //     ht_add(buf);
    // }

    const char *WORD_DATA[32] = {   
                            "auto", "break", "case", "char", "const", "continue", "default", 
                            "do", "double", "else", "enum", "extern", "float", "for", 
                            "goto", "if", "int", "long", "register", "return", "short", 
                            "signed", "sizeof", "static", "struct", "switch", "typedef", "union", 
                            "unsigned", "void", "volatile", "while" 
                        };

    for (int i=0; i < 32; i++) {
        ht_add(WORD_DATA[i]);
    }
}
/*----------------------------------------------------------------------------*/

void scan(const char *s) {
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
            printf("%s \"%.*s\"\n", prev->name, j-i, s+i); /* '.*' specifies length of string to print */
            i = j;
            continue;
        }

        /* Number */
        /* Array of function pointers*/
        bool found = false;
        int (*f_ptr[])(const char *, int) = {&scan_hex, &scan_oct, &scan_dec, &scan_float}; 
        int peek_ind;
        for (int j=0; j < 4; j++) {
            peek_ind = (f_ptr[j])(s, i);
            if (peek_ind > i) {
                found = true;
                i = peek_ind-1;
                break;
            }
        }

        if (found) {
            continue;
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

        printf("unknown char \"%c\"\n", s[i]);
        i++;
    }
}

int main() {
    op_load_file();
    word_load_file();
    // op_print(op_main, 0);

    char s[] = "0x98"; //test "`"
    scan(s);

    return 0;
}
