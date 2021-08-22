#include "apue.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    if (chdir("/tmp") < 0)
        err_sys("chdir failed");
    printf("chdir to /tmp succeeded\n");
    exit(0);
    return 0;
}
