#ifndef __TIMEVAL_H__
#define __TIMEVAL_H__

#include <sys/time.h>
#include <libobject/core/utils/dbg/debug.h>

#if (defined(WINDOWS_USER_MODE))
#include <windows.h>
#define sleep(n) Sleep(1000 * (n))
#else
#include <unistd.h>
#endif

int timeval_cmp(struct timeval *k1,struct timeval *k2);
int timeval_add(struct timeval *k1,struct timeval *k2, struct timeval *r);
int timeval_sub(struct timeval *k1,struct timeval *k2, struct timeval *r);
int timeval_now(struct timeval *t, struct timezone *tz);
int timeval_clear(struct timeval *t);
int timeval_print(struct timeval *t);

#endif
