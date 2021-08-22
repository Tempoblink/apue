#include <netdb.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
    struct protoent *ret = getprotobyname("TCP");
    if (ret == NULL) {
        perror("getprotobyname failed");
        return -1;
    }

    printf("protocol name = %s\n", ret->p_name);
    printf("protocol number = %d\n", ret->p_proto);
    return 0;
}
