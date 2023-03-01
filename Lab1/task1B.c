# include <stdio.h>
# include <stdlib.h>
#include <string.h>
#define DEBUG "-D"


char toUpper(char letter);

int isLowerCase(char *ascii);

int isDebugMode(int argc, char **argv);

void printArg(FILE *err, int argc, char **argv);

int isFlag(char *flag, int argc, char **argv);

int main(int argc, char **argv) {


    FILE *in = stdin;
    FILE* out = stdout;
    FILE *err = stderr;

    char c;
     if (isDebugMode(argc, argv)) {
        printArg(err, argc, argv);
        while ((c = fgetc(in)) != EOF) {
            if (c != '\n') {
                if (isLowerCase(&c)) {
                    fprintf(err, "%X %X\n", c, toUpper(c));
                    fputc(toUpper(c), out);
                } else {
                    fprintf(err, "%X %X\n", c, c);
                    fputc(c, out);
                }
            }
        }
        fputc('\n', out);
    } else {
        while ((c = fgetc(in)) != EOF) {
            if (isLowerCase(&c))
                fputc(toUpper(c), out);
            else
                fputc(c, out);
        }
        fputc('\n', out);
    }

    return 0;
}

char toUpper(char letter) {
    return letter - 32;
}

int isLowerCase(char *ascii) {
    return *ascii >= 'a' && *ascii <= 'z';
}

int isFlag(char *flag, int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], flag, 2) == 0) return 1;
    }
    return 0;
}

int isDebugMode(int argc, char **argv) {
    return isFlag(DEBUG, argc, argv);
}


void printArg(FILE *err, int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (i == argc - 1)
            fprintf(err, "%s\n", argv[i]);
        else
            fprintf(err, "%s ", argv[i]);
    }
}
