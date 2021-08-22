#include <netdb.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
    struct hostent *ret = gethostent();

    printf("h_name = %s\n", ret->h_name);
    printf("h_aliases = %s\n", *ret->h_aliases);
    printf("h_addrtype = %d\n", ret->h_addrtype);
    printf("h_length = %d\n", ret->h_length);
    printf("h_addr = %s\n", ret->h_addr_list[0]);
    return 0;
}
