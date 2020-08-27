#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <limits.h>
#include <sys/mman.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

#define CODESIZE 5U

struct func_stub
{
    void *fn;
    unsigned char code_buf[CODESIZE];
};

int stub_init();
void stub_set(struct func_stub *pstub, void *fn, void *fn_stub);
void stub_reset(struct func_stub *pstub);


static long pagesize = -1;

static inline void *pageof(const void* p)
{
    return (void *)((unsigned long)p & ~(pagesize - 1));
}

void stub_set(struct func_stub *pstub, void *fn, void *fn_stub)
{
    pstub->fn = fn;
    memcpy(pstub->code_buf, fn, CODESIZE);

    if (-1 == mprotect(pageof(fn), pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
    {
        perror("mprotect to w+r+x faild");
        exit(errno);
    }

    *(unsigned char *)fn = (unsigned char)0xE9;
    *(unsigned int *)((unsigned char *)fn + 1) = (unsigned char *)fn_stub - (unsigned char *)fn - CODESIZE;

    if (-1 == mprotect(pageof(fn), pagesize * 2, PROT_READ | PROT_EXEC))
    {
        perror("mprotect to r+x failed");
        exit(errno);
    }

    return;
}

void stub_reset(struct func_stub *pstub)
{
    if (NULL == pstub->fn)
    {
        return;
    }

    if (-1 == mprotect(pageof(pstub->fn), pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
    {
        perror("mprotect to w+r+x faild");
        exit(errno);
    }

    memcpy(pstub->fn, pstub->code_buf, CODESIZE);

    if (-1 == mprotect(pageof(pstub->fn), pagesize * 2, PROT_READ | PROT_EXEC))
    {
        perror("mprotect to r+x failed");
        exit(errno);
    }

    memset(pstub, 0, sizeof(struct func_stub));

    return;
}

int stub_init(void)
{
    int ret;

    pagesize = sysconf(_SC_PAGE_SIZE);

    ret = 0;
    if (pagesize < 0)
    {
        perror("get system _SC_PAGE_SIZE configure failed");
        ret = -1;
    }

    return ret;
}

void f1()
{
    printf("f1\n");

    return;
}

void f2()
{
    printf("f2\n");

    return;
}

void *_memset(void *s, int ch, size_t n)
{
    printf("-memset\n");

    return s;
}

int test_stub()
{
    char ac[10] = {1};
    struct func_stub stub;

    if (-1 == stub_init())
    {
        printf("faild to init stub\n");
        return 0;
    }

    stub_set(&stub, (void *)memset, (void *)_memset);

    memset(ac, 0, 10);

    stub_reset(&stub);

    memset(ac, 0, 10);
    printf("ac[0] = %hhu\n", ac[0]);

    stub_set(&stub, (void *)f1, (void *)f2);

    f1();

    stub_reset(&stub);

    f1();

    return 0;
}
REGISTER_TEST_CMD(test_stub);
