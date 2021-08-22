#include <stdio.h>

int main(int argc, char const *argv[]) {
    const char *text1 = "[1]:hello, world.|";
    fputs("fputs(): ", stdout);
    int status = fputs(text1, stdout);
    if (status == EOF) {
        perror("fputs error");
    }
    status = fputs(text1, stdout);
    if (status == EOF) {
        perror("fputs error");
    }

    const char *text2 = "[2]:hello, world.\n|";
    status = fputs(text2, stdout);
    if (status == EOF) {
        perror("fputs error");
    }
    status = fputs(text2, stdout);
    if (status == EOF) {
        perror("fputs error");
    }
    
    fputs("puts(): ", stdout);
    puts(text1);

    puts(text2);
    return 0;
}
