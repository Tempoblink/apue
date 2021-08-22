#include "apue.h"

int main(int argc, char const *argv[]) {
    int i;

    for (i = 0; i < argc; i++)
        printf("argv[%d]: %s\n", i, argv[i]);
    exit(0);
}
