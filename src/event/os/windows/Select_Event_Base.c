/**
 * @file select_base.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-13
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#if (defined(WINDOWS_USER_MODE))

#include <stdio.h>
#include <errno.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/event/Select_Base.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>

static int __construct(Select_Base *eb, char *init_str)
{
    dbg_str(OBJ_DETAIL, "select constructor");

    eb->maxfdp = 1;
    FD_ZERO(&eb->event_readset_in);
    FD_ZERO(&eb->event_writeset_in);
    FD_ZERO(&eb->event_readset_out);
    FD_ZERO(&eb->event_writeset_out);

    evsig_init((Event_Base *)eb);

    return 0;
}

static int __deconstrcut(Select_Base *eb)
{
    dbg_str(OBJ_DETAIL, "select constructor");

    evsig_release((Event_Base *)eb);

    return 0;
}

static int __trustee_io(Select_Base *b, event_t *e)
{
    int fd = e->ev_fd;
    Event_Base *p = (Event_Base *)b;
    int readset_count;
    int writeset_count;

    if (fd < 0) {
        dbg_str(EV_WARNNING, "not add this fd =%d", fd);
        return 0;
    }

    if (e->ev_events & EV_READ)
        FD_SET(fd, &b->event_readset_in);
    if (e->ev_events & EV_WRITE)
        FD_SET(fd, &b->event_writeset_in);

    readset_count = b->event_readset_in.fd_count;
    writeset_count = b->event_writeset_in.fd_count;
    b->maxfdp = readset_count > writeset_count ? readset_count + 1 : writeset_count + 1;

    dbg_str(EV_DETAIL, "select base add event, fd =%d, maxfdp=%d", fd, b->maxfdp);

    return (0);
}

static int __reclaim_io(Select_Base *b, event_t *e) 
{
    int fd = e->ev_fd;
    unsigned short events = e->ev_events;

    dbg_str(EV_DETAIL, "select base add event");

    if (fd < 0) return 0;

    if (events & EV_READ)
        FD_CLR(fd, &b->event_readset_in);

    if (events & EV_WRITE)
        FD_CLR(fd, &b->event_writeset_in);

    return 0;
}

static int __dispatch(Select_Base *b, struct timeval *tv)
{
    int res=0, i, j, nfds = b->maxfdp;

    b->event_readset_out  = b->event_readset_in;
    b->event_writeset_out = b->event_writeset_in;

    dbg_str(DBG_DETAIL, "dispatch select in,  nfds=%d", nfds);

    res = select(nfds, &b->event_readset_out, 
                 &b->event_writeset_out, NULL, tv);
    if (res == -1) {
        dbg_str(EV_WARNNING, "res=%d", WSAGetLastError());
        if (errno == EINTR) {
        } else {
            ((Event_Base *)b)->break_flag = 1;
            perror("dispatch");
            dbg_str(EV_WARNNING, "dispatch, erro_no:%d, nfds=%d", errno, nfds);
        }
        return (0);
    } else if (res > 0) {
        if (tv != NULL)
            dbg_str(EV_DETAIL, "select base dispatch io events res=%d, tv=%d", res, tv->tv_sec);
    } else {
        dbg_str(EV_DETAIL, "select timeout");
        return 0;
    }

    i = rand() % nfds;
    for (j = 0; j < nfds; ++j) {
        if (++i >= nfds)
            i = 0;
        if (FD_ISSET(b->event_readset_in.fd_array[i], &b->event_readset_out)) {
            dbg_str(EV_DETAIL, "fd %d has read event", b->event_readset_in.fd_array[i]);
            b->activate_io((Event_Base *)b, b->event_readset_in.fd_array[i], EV_READ);
        }
        if (FD_ISSET(b->event_writeset_in.fd_array[i], &b->event_writeset_out)) {
            dbg_str(EV_DETAIL, "fd %d has write event", b->event_writeset_in.fd_array[i]);
            b->activate_io((Event_Base *)b, b->event_writeset_in.fd_array[i], EV_WRITE);
        }
    }

    dbg_str(EV_DETAIL, "dispatch select out");

    return 0;
}

static class_info_entry_t select_base_class_info[] = {
    Init_Obj___Entry(0, Event_Base, base),
    Init_Nfunc_Entry(1, Select_Base, construct, __construct),
    Init_Nfunc_Entry(2, Select_Base, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3, Select_Base, trustee_io, __trustee_io),
    Init_Vfunc_Entry(4, Select_Base, reclaim_io, __reclaim_io),
    Init_Vfunc_Entry(5, Select_Base, activate_io, NULL),
    Init_Vfunc_Entry(6, Select_Base, dispatch, __dispatch),
    Init_End___Entry(7, Select_Base),
};
REGISTER_CLASS("Select_Base", select_base_class_info);

static void test_ev_callback(int fd, short events, void *arg)
{
    dbg_str(EV_DETAIL, "hello world, event");
}

int test_select_base()
{
    Event_Base *eb;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    char buf[2048];
    cjson_t *root, *e, *s;
    event_t event;

    eb = OBJECT_NEW(allocator, Select_Base, NULL);

    dbg_str(EV_DETAIL, "run at here, eb=%p", eb);

    object_dump(eb, "Select_Base", buf, 2048);
    dbg_str(EV_DETAIL, "Select_Base dump: %s", buf);

    event.ev_timeout.tv_sec  = 10;
    event.ev_timeout.tv_usec = 0;
    event.ev_fd = -1;
    event.ev_callback = test_ev_callback;

    eb->add(eb, &event);
    eb->loop(eb);

    object_destroy(eb);

    return 1;
}
REGISTER_TEST_FUNC(test_select_base);
#endif
