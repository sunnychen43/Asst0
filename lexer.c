#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

/*-----------------------------OPERATORS-------------------------------------*/

/* 
 * operators are stored as a LL, each node also contains children that
 * share the same prefix. (ex: '-' will contain '->' and '--' as children) 
 */
typedef struct OP {
    char c, *name;
    struct OP *children;
    struct OP *next;
} OP;

/* global head of LL */
static OP *op_main;

/* 
 * Initializes OP struct nodes. Memory will be allocated only to the struct itself,
 * the name, children, and next fields are set to NULL.
 * 
 * Parameters
 *     None
 * Precondition
 *     None
 * Returns
 *     OP* pointer to allocated struct
 */
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

/* 
 * Creates a op node and inserts into head. Each node can only contain 1 char,
 * so will call itself recursivly untill all characters in op_chr have been 
 * inserted.
 * 
 * Parameters
 *     OP **head - double pointer to head of LL
 *     const char *op_chr - operator chars
 *     const char *name - operator name
 * Precondition
 *     op_chr and name are valid char arrays, not NULL
 * Returns
 *     None
 */
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

/* 
 * Creates a op node and inserts into head. Each node can only contain 1 char,
 * so will call itself recursivly untill all characters in op_chr have been 
 * inserted.
 * 
 * Parameters
 *     OP **head - double pointer to head of LL
 *     const char *op_chr - operator chars
 *     const char *name - operator name
 * Precondition
 *     op_chr and name are valid char arrays, not NULL
 * Returns
 *     None
 */
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

void op_load_data() {

    /* char[] array of all operators and their names */
    /* the operator and its name are seperated by a space */
    const char *OP_DATA[43] = 
        {
            "( left parenthesis", ") right parenthesis", "[ left bracket", "] right bracket", 
            ". structure member", "-> structure pointer", ", comma", "! negate", "~ 1s complement", 
            ">> shift right", "<< shift left", "^ bitwise XOR", "| bitwise OR", "++ increment", 
            "-- decrement", "+ addition", "/ division", "|| logical OR", "&& logical AND", 
            "? conditional true", ": conditional false", "== equality test", "!=  inequality test", 
            "< less than test", "> greater than test", "<= less than or equal test", ">= greater than or equal test", 
            "= assignment", "+= plus equals", "-= minus equals", "*= times equals", "/= divide equals",  
            "\%= mod equals", ">>= shift right equals", "<<= shift left equals", "&= bitwise AND equals",  
            "^= bitwise XOR equals", "|= bitwise OR equals", "& AND/address operator", "- minus/subtract operator", 
            "* multiply/dereference operator", "\" double quote", "\' single quote"
        };
    
    /* load each operator into op_main */
    char op_chr[5], name[30];
    for (int i=0; i < 43; i++) {
        sscanf(OP_DATA[i], "%s %[^\n]s", op_chr, name); /* parse the char[] in OP_DATA */
        op_add(&op_main, op_chr, name);
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

/* 
 * Checks if the next available token is a hexadecimal, and returns the index the next token starts at.
 * 
 * Parameters
 *     contant char* c - input string
 *     int index - index of current character in the original string
 * Precondition
 *     index is less than the length of the arg char array
 * Returns
 *     integer index of the first character next token (returns parameter index if no hexadecimal is found)
 */
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

/* 
 * Checks if the next available token is a octal, and returns the index the next token starts at.
 * 
 * Parameters
 *     contant char* c - input string
 *     int index - index of current character in the original string
 * Precondition
 *     index is less than the length of the arg char array
 * Returns
 *     integer index of the first character next token (returns parameter index if no octal is found)
 */
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
        printf("\"\n");
        return idx; //this was all coded assuming 0x is a hex, change if otherwise
    }
}

/* 
 * Checks if the next available token is a decimal, and returns the index the next token starts at.
 * 
 * Parameters
 *     contant char* c - input string
 *     int index - index of current character in the original string
 * Precondition
 *     index is less than the length of the arg char array
 * Returns
 *     integer index of the first character next token (returns parameter index if no decimal is found)
 */
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

/* 
 * Checks if the next available token is a float, and returns the index the next token starts at.
 * 
 * Parameters
 *     contant char* c - input string
 *     int index - index of current character in the original string
 * Precondition
 *     index is less than the length of the arg char array
 * Returns
 *     integer index of the first character next token (returns parameter index if no float is found)
 */
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
    /* scan num functions */
    int (*f_ptr[])(const char *, int) = {&scan_hex, &scan_oct, &scan_dec, &scan_float};

    int i=0;
    while (i < strlen(s)) {
        char c = s[i];

        /* skip comments */
        if (c == '/' && s[i+1] == '*') {
            bool found = false;

            int j=i+2; /* first two char are comment, skip them */
            for (; j < strlen(s)-1; j++) {
                if (s[j] == '*' && s[j+1] == '/') {
                    found = true;
                    break;
                }
            }

            /* skip ahead if found */
            if (found) {
                i = j+2; /* j is at '*' of comment */
                continue;
            }
        }

        /* catch quotes */
        if (c == '\"' || c == '\'') {
            bool found = false;

            int j = i+1; /* skip first quote */
            for (; j < strlen(s); j++) {
                if (s[j] == c) { /* make sure quotes match */
                    found = true;
                    break;
                }
            }

            if (found) {
                printf("string \"%.*s\"\n", j-i-1, s+i+1); /* j-i-1 is string len, s+i+1 is starting pos */
                i = j+1;
                continue;
            }
        }

        /* skip terminators */
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            i++; continue;
        }

        /* catch operators */
        OP *prev = NULL;
        OP *curr = op_main;
        int j = i;
        for (; j < strlen(s); j++) {
            if (curr == NULL) {break;} /* op has been found and cant be longer */

            OP *res = op_search(curr, s[j]);

            if (res == NULL) {break;} /* match not found */

            prev = res;
            curr = res->children;
        }

        /* OP found, increase index and print op */
        /* prev will be set if match is found */
        if (prev != NULL) {
            printf("%s \"%.*s\"\n", prev->name, j-i, s+i); /* '.*' specifies length of string to print */
            i = j;
            continue;
        }

        /* catch number */
        if (isdigit(c)) {
            for (int j=0; j < 4; j++) {
                int peek_ind = (f_ptr[j])(s, i);
                if (peek_ind > i) {
                    i = peek_ind;
                    break;
                }
            }
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
                printf("keyword \"%s\"\n", word);
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
    op_load_data();
    word_load_file();
    // op_print(op_main, 0);

    char s[] = "123.12.2.2.31";
    scan(s);

    return 0;
}
