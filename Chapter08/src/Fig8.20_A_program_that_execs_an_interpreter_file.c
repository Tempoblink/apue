#include "apue.h"
#include <sys/wait.h>

int main(int argc, char const *argv[]) {
    pid_t pid;

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {           /* child */
        if (execl("./testinterp",  "testinterp", "myarg1", "MY ARG2", (char *) 0) < 0)
            err_sys("execl error");
    }
    if (waitpid(pid, NULL, 0) < 0)    /* parent */
        err_sys("waitpid error");
    exit(0);
}
