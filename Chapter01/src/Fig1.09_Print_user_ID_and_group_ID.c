#include "apue.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    printf("uid = %d, gid = %d\n", getuid(), getgid());
    exit(0);
}
