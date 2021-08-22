#include "apue.h"
#include <setjmp.h>

#define TOK_ADD 5

jmp_buf jmpbuffer;

void do_line(char *);
void cmd_add(void);
int get_token(void);

int main(int argc, char const *argv[]) {
    char line[MAXLINE];

    if (setjmp(jmpbuffer) != 0)
        printf("error");
    while (fgets(line, MAXLINE, stdin) != NULL)
        do_line(line);
    exit(0);
}

void  cmd_add(void) {
  int token;

  token = get_token();
  if(token < 0)         /* an error has occurred */
    longjmp(jmpbuffer, 1);
  /* rest of processing for this command */
}
