#include "apue.h"
#include <stdio.h>
#include <sys/wait.h>

#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif

static void sig_cld(int);

int main(int argc, char const* argv[])
{
  pid_t pid;
  
  if(signal(SIGCLD, sig_cld) == SIG_ERR)
    perror("signal error");
  if((pid = fork()) < 0) {
    perror("fork error");
  }else if(pid == 0) {
    sleep(2);
    _exit(0);
  }

  pause();
  exit(0);
}


static void sig_cld(int signo) {
  pid_t pid;
  int status;

  printf("SIGCLD received\n");

  if(signal(SIGCLD, sig_cld) == SIG_ERR)
    perror("signal error");

  if((pid = wait(&status)) < 0)
    perror("wait error");

  printf("pid = %d\n",pid);
}
