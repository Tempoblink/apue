#include <limits.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static sem_t *sem;

int main(int argc, char const *argv[]) {
    char name[_POSIX_NAME_MAX];
    snprintf(name, sizeof(name), "%ld.%d", (long) getpid(), 234);
    printf("call sem_open\n");
    sem = sem_open(name, O_CREAT | O_EXCL, 0600, 1);// sem = 1
    printf("call sem_post\n");
    sem_post(sem);             //sem = 2
    int err = sem_trywait(sem);//sem = 1
    printf("call sem_trywait return: %2d\n", err);
    // err = sem_trywait(sem);
    // printf("%d", err);
    // err = sem_trywait(sem);
    // printf("%d", err);
    printf("call sem_post\n");
    sem_post(sem);         //sem = 2, 测试平台macos，当多次解锁后，可以相应的多次加锁,这与初值矛盾,所以初始值只是个建议值
    err = sem_trywait(sem);//sem = 1
    printf("call sem_trywait return: %2d\n", err);
    err = sem_trywait(sem);// sem = 0
    printf("call sem_trywait return: %2d\n", err);
    err = sem_trywait(sem);// sem = 0
    printf("call sem_trywait return: %2d\n", err);

    sem_unlink(name);
    return 0;
}
