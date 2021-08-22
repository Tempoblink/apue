#include <errno.h>
#include <limits.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/semaphore.h>
#include <unistd.h>

struct slock {
    sem_t *semp;
    char name[_POSIX_NAME_MAX];
};

struct slock *s_alloc() {
    struct slock *sp;
    static int cnt;

    if ((sp = malloc(sizeof(struct slock))) == NULL)
        return (NULL);
    do {
        snprintf(sp->name, sizeof(sp->name), "/%ld.%d", (long) getpid(), cnt++);
        sp->semp = sem_open(sp->name, O_CREAT | O_EXCL, S_IRWXU, 1);
    } while ((sp->semp == SEM_FAILED) && (errno == EEXIST));//try to cnt++.
    if (sp->semp == SEM_FAILED) {                           //sem_open failed!
        free(sp);
        return (NULL);
    }
    sem_unlink(sp->name);//because of *sp open the sem, so sp become anonymous.
    return (sp);
}

void s_free(struct slock *sp) {
    sem_close(sp->semp);
    free(sp);
}

int s_lock(struct slock *sp) {
    return (sem_wait(sp->semp));
}

int s_trylock(struct slock *sp) {
    return (sem_trywait(sp->semp));
}

int s_unlock(struct slock *sp) {
    return (sem_post(sp->semp));
}
