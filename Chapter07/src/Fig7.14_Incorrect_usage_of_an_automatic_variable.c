#include <stdio.h>

FILE *opendata(void) {
    FILE *fp;
    char databuf[BUFSIZ]; /* setvbuf makes this the stdio buffer */

    if ((fp = fopen("datafil", "r")) == NULL)
        return NULL;
    if (setvbuf(fp, databuf, _IOLBF, BUFSIZ) != 0)
        return NULL;

    return fp;
}
