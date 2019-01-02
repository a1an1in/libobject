#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libobject/core/utils/registry/registry.h>

int test_gethostbyname(TEST_ENTRY *entry)
{
    int i;
    struct hostent *he;
    struct in_addr **addr_list;

    if ((he = gethostbyname("www.baidu.com")) == NULL) {
        perror("gethostbyname");
    }

    addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
        printf("%s ", inet_ntoa(*addr_list[i]));
    }
    printf("\n");

    return 1;
}
REGISTER_TEST_FUNC(test_gethostbyname);
