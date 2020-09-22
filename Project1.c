//#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
//#include <sys/type.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

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

int hex(char* arg, int index) { //returns -1 if no hex found, returns new starting index if hex found
    int idx = index;
    char* acceptable = "1234567890abcdefABCDEF";
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
    while (oneOf(arg[idx], acceptable)) {
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

int decimal(char* arg, int index) {
    int idx = index;
    char* acceptable = "1234567890";
    if (!oneOf(arg[idx], acceptable)) {
        return index; //not a decimal or float
    }
    //ok we start with a number
    while (oneOf(arg[idx], acceptable)) {
        idx++;
    }
    if (arg[idx] == '.' && oneOf(arg[++idx], acceptable)) { //need to include e-24 is 5e-4 is a decimal
        return index;
    }
    else {
        printf("decimal ");
        for (int i = index; i < idx; i++) {
            printf("%c", arg[i]);
        }
        printf("\n");
        return idx;
    }
    return -1;
}

int flo(char* arg, int index) { //bug rn with consecutive decimals and no space, ie 324.324.234
    int idx = index;
    int decimalPoint = 0; //boolean, 0 if decimal point not found yet
    char* acceptable = "1234567890";
    if (!oneOf(arg[idx], acceptable)) {
        if (!oneOf(arg[idx], ".")) {
            return index; //must be a word
        }
        decimalPoint = 1;
    }
    //ok we have a float
    printf("float ");
    while (oneOf(arg[idx], acceptable)) {
        printf("%c", arg[idx]);
        idx++;
    }
    if (arg[idx] == '.' && !decimalPoint) { //need to include e-24 is 5e-4 is a decimal
        printf(".");
        idx++; //increment so we're past the decimal point
        decimalPoint = 1; //not really necessary, but for the sake of completeness
        while (oneOf(arg[idx], acceptable)) {
            printf("%c", arg[idx]);
            idx++;
        }
        printf("\n");
        return idx; 
    }
    else { //we're done here, new token has started
        printf("\n");
        return idx; 
    }
    return -1;
}

int word(char* arg, int index) {
    printf("word ");
    while (index < strlen(arg)) {
        printf("%c", arg[index]);
        index++;
    }
    printf("\n");
    return index;
}

int main(int argc, char** argv) {
    int i = 1;
    int index = 0;
    int newIndex = 0;
    for (i = 1; i < argc; i++) {
        index = 0;
        while (index < strlen(argv[i])) {
            newIndex = hex(argv[i], index);
            if (newIndex != index) {
                index = newIndex;
                continue;
            }
            newIndex = oct(argv[i], index);
            if (newIndex != index) {
                index = newIndex;
                continue;
            }
            newIndex = decimal(argv[i], index);
            if (newIndex != index) {
                index = newIndex;
                continue;
            }
            newIndex = flo(argv[i], index);
            if (newIndex != index) {
                index = newIndex;
                continue;
            }
            index = word(argv[i], index);
        }
    }
    return 0;
}