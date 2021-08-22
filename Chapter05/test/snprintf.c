#include <stdio.h>

int main(int argc, char const* argv[])
{
  char buf[10];

  int status = snprintf(buf, 10, "%s", "hello,world");
  printf("%d\n", status);
  if(status < 0) {
    perror("snprintf error");
    return -1;
  }
  printf("%s", buf);
  return 0;
}
