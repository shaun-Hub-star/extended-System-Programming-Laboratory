#include <stdio.h>

int count_digits(char* str) {
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            count++;
        }
    }
    return count;
}

int main(int argc, char **argv) {
    char* str = argv[1];
    int digit_count = count_digits(str);
    printf("The number of digits in the string['%s'] is: : %d\n", str, digit_count);
    return 0;
}