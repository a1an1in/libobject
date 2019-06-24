#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
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

int test_str_cmp(TEST_ENTRY *entry)
{
    char *dest = "--file-path";
    int ret;

    ret = strcmp(dest, "--file");
    dbg_str(DBG_DETAIL, "cmp --file, ret=%d", ret);

    ret = strcmp(dest, "--file-path");
    dbg_str(DBG_DETAIL, "cmp --file-path, ret=%d", ret);

    ret = strcmp(dest, "--file-path=~/x.xml");
    dbg_str(DBG_DETAIL, "cmp --file-path=~/x.xml, ret=%d", ret);
}
REGISTER_TEST_FUNC(test_str_cmp);

