#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {

    // openat中fd为打开的普通文件
    printf("openat中fd为打开的普通文件: ");
    int fd1 = open("../src/Fig3.02_Create_a_file_with_a_hole_in_it.c", O_RDONLY);
    int fd2 = openat(fd1, "Fig3.02_Create_a_file_with_a_hole_in_it.c", O_RDONLY);
    char src[2048], file[2048];
    memset(file, 0x00, sizeof(file));
    memset(src, 0x00, sizeof(src));
    int r1 = read(fd1, src, sizeof(src));
    if (fd1 < 0) {
        perror("open file error");
    }
    int r2 = read(fd2, file, sizeof(file));
    if (fd2 < 0) {
        perror("open file error");
    }
    printf("%s\n", (r1 == r2) ? "true" : "false");
    printf("%s", file);

    //openat中fd为打开的目录
    printf("openat中fd为打开的目录: ");
    int fd3 = open("../src", O_DIRECTORY);
    if (fd3 < 0) {
        perror("open dir error");
    }
    int fd4 = openat(fd3, "Fig3.02_Create_a_file_with_a_hole_in_it.c", O_RDONLY);
    if (fd3 < 0) {
        perror("open file error");
    }
    memset(file, 0x00, sizeof(file));
    int r3 = read(fd4, file, sizeof(file));
    printf("%s\n", (r1 == r3) ? "true" : "false");

    //使用AT_FDCWD
    char cwd[64];
    printf("cwd: %s\n", getcwd(cwd, sizeof(cwd)));
    printf("使用AT_FDCWD： ");
    int fd5 = openat(AT_FDCWD, "./src/Fig3.02_Create_a_file_with_a_hole_in_it.c", O_RDONLY);
    if (fd5 < 0) {
        perror("open file error");
    }
    memset(file, 0x00, sizeof(file));
    int r4 = read(fd5, file, sizeof(file));
    printf("%s\n", (r1 == r4) ? "true" : "false");

    int fd6 = openat(AT_FDCWD, "openat.c", O_RDONLY);
    if (fd6 < 0) {
        perror("open file error");
    }
    memset(file, 0x00, sizeof(file));
    int r5 = read(fd6, file, sizeof(file));
    printf("%s\n", file);

    return 0;
}
