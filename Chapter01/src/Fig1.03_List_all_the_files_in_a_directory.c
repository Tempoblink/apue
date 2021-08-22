#include "apue.h"
#include <dirent.h>//目录相关函数库

int main(int argc, char const *argv[]) {
    DIR *dp;//目录指针
    struct dirent *dirp;//文件的目录项

    if (argc != 2)
        err_quit("usage: ls directory_name");

    if ((dp = opendir(argv[1])) == NULL)// 打开文件夹，返回指向DIR结构的指针
        err_sys("can't open %s", argv[1]);
    while ((dirp = readdir(dp)) != NULL)//读取文件夹里的内容
        printf("%s\n", dirp->d_name);
    closedir(dp);//记得关闭文件指针
    exit(0);
}
