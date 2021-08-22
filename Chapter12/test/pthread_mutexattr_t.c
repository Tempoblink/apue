#include <pthread.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
    pthread_mutexattr_t attr;
    int pshared, err;

    err = pthread_mutexattr_init(&attr);
    err = pthread_mutexattr_getpshared(&attr, &pshared);
    printf("default pshared = %d\n", pshared);

    err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    err = pthread_mutexattr_getpshared(&attr, &pshared);
    printf("set PTHREAD_PROCESS_SHARED, pshared = %d\n", pshared);

    err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
    err = pthread_mutexattr_getpshared(&attr, &pshared);
    printf("set PTHREAD_PROCESS_PRIVATE, pshared = %d\n", pshared);


    err = pthread_mutexattr_getpshared(&attr, NULL);
    printf("%d\n", err);
    return 0;
}
