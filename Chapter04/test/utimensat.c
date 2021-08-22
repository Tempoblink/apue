#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const char *filename = "test.log";
const char *symbol = "test.log.s";

int main(int argc, char const *argv[]) {
    struct stat file1, file2;

    int fd1 = open(filename, O_CREAT | O_RDWR, 0777);
    int status = stat(filename, &file1);
    if (status < 0) {
        perror("stat error");
        return -1;
    }
    printf("filename = %s\natime = %ld\nmtime = %ld\nctime = %ld\n", filename, file1.st_atime, file1.st_mtime, file1.st_ctime);

    sleep(1);
    do {
        status = symlink(filename, symbol);
        if (status < 0) {
            if (errno == EEXIST) {
                unlink("symbol");
                continue;
            } else {
                perror("symlink error");
                return -1;
            }
        }
    } while (0);

    status = lstat(symbol, &file1);
    if (status < 0) {
        perror("lstat error");
        return -1;
    }
    printf("filename = %s\natime = %ld\nmtime = %ld\nctime = %ld\n", symbol, file1.st_atime, file1.st_mtime, file1.st_ctime);

    status = lstat(__FILE_NAME__, &file1);
    if (status < 0) {
        perror("lstat error");
        return -1;
    }
    printf("filename = %s\natime = %ld\nmtime = %ld\nctime = %ld\n", __FILE_NAME__, file1.st_atime, file1.st_mtime, file1.st_ctime);

    struct timespec times[2];
    memset(times, 0x00, sizeof(times));
    times[0] = file1.st_atimespec;
    times[1] = file1.st_atimespec;

    status = utimensat(AT_FDCWD, "test.log.s", times, 0);
    if (status < 0) {
        perror("utimensat error");
        return -1;
    }

    status = stat(filename, &file1);
    if (status < 0) {
        perror("stat error");
        return -1;
    }
    status = lstat(symbol, &file2);
    if (status < 0) {
        perror("lstat error");
        return -1;
    }
    printf("filename = %s\natime = %ld\nmtime = %ld\nctime = %ld\n", filename, file1.st_atime, file1.st_mtime, file1.st_ctime);
    printf("filename = %s\natime = %ld\nmtime = %ld\nctime = %ld\n", symbol, file1.st_atime, file1.st_mtime, file1.st_ctime);

    unlink(filename);
    unlink(symbol);
    return 0;
}
