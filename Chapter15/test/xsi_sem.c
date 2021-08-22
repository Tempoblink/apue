#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <time.h>
#include <unistd.h>

static struct semid_ds sem;

int main(int argc, char const *argv[]) {

    // sem init
    int id;
    key_t keyid;
    do {
        srand(time(NULL));
        keyid = ftok("xsi_sem.c", rand() % 256);
        id = semget(keyid, 1, IPC_CREAT | IPC_EXCL | 0600);
#ifdef DEBUG
        printf("id = %d\n", id);
#endif
    } while ((id < 0) && (errno == EEXIST));
    if (id < 0) {
        perror("create XSI sem failed");
        return -1;
    }

    // after init semid_ds
    printf("call semget() after ...\n");
    union semun arg;
    arg.buf = malloc(sizeof(struct semid_ds));
    semctl(id, 2, IPC_STAT, arg);
    printf("sem.sem_perm.mode = %5o\n", arg.buf->sem_perm.mode);
    printf("sem.sem_perm.uid  = %5d\n", arg.buf->sem_perm.uid);
    printf("sem.sem_perm.gid  = %5d\n", arg.buf->sem_perm.gid);
    printf("sem.sem_perm.cuid = %5d\n", arg.buf->sem_perm.cuid);
    printf("sem.sem_perm.cgid = %5d\n", arg.buf->sem_perm.cgid);
    printf("sem.sem_nsems     = %5d\n", arg.buf->sem_nsems);
    // producter & customer thread create
    // pthread_t thread[2];
    return 0;
}
