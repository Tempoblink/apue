#include "apue.h"
#include <stdio.h>

int main(int argc, char const* argv[])
{
  int i;
  char **ptr;
  extern char **environ;

  for(i = 0; i < argc; i++)
    printf("argv[%d]: %s\n", i, argv[i]);

  for(ptr = environ; *ptr != 0; ptr++) 
    printf("%s\n", *ptr);

  exit(0);
}
