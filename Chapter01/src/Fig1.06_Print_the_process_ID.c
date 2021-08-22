#include "apue.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    printf("hello world from process ID %ld\n", (long) getpid());
    exit(0);
}
