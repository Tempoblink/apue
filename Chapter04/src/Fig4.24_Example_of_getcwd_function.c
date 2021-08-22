#include "apue.h"

int main(int argc, char const *argv[]) {
    char *ptr;
    size_t size;
    if (chdir("/usr/spool/uucppubl") < 0)
        err_sys("chdir failed");
    ptr = path_alloc((int *) &size);
    if (getcwd(ptr, size) == NULL)
        err_sys("getcwd failed");
    printf("cwd = %s\n", ptr);
    exit(0);
}
