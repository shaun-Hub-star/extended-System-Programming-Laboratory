# include <stdio.h>
# include <stdlib.h>
#include <string.h>

char toUpper(char letter);
int isLowerCase(char *ascii);

int main(int argc, char **argv) {

    FILE*in = stdin;
    FILE* out = stdout;
    char c;
    while ((c = fgetc(in)) != EOF) {
        if (isLowerCase(&c))
            fputc(toUpper(c), out);
        else
            fputc(c, out);
      }
    fputc('\n', out);
    

    return 0;
}

char toUpper(char letter) {
    return letter - 32;
}

int isLowerCase(char *ascii) {
    return *ascii >= 'a' && *ascii <= 'z';
}


