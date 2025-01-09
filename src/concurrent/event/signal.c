#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <libobject/core/Rbtree_Map.h>
#include <libobject/core/Linked_List.h>
#include <libobject/core/utils/config.h>
#include <libobject/concurrent/event/Select_Base.h>
#include <libobject/config.h>

static void
evsig_cb(int fd, short what, void *arg)
{
    char signals[1024];
    event_t *event = (event_t *)arg;
    Event_Base *eb = (Event_Base *)event->ev_base;
    Socket *receiver = event->ev_socket;
    int ncaught[NSIG] = {0};
    int n;
    int i;

    memset(&ncaught, 0, sizeof(ncaught));

    n = receiver->recv(receiver, signals, sizeof(signals), 0);
    if (n == -1) {
        dbg_str(EV_ERROR,"evsig_cb recv");
        return ;
    } else if (n == 0) {
    }
    dbg_str(EV_VIP,"evsig_cb in, eb:%p, recve signal count:%d, signal:%d", eb, n, signals[0]);

    for (i = 0; i < n; ++i) {
        uint8_t sig = signals[i];
        dbg_str(EV_DETAIL,"sig=%d",sig);
        if (sig < NSIG)
            ncaught[sig]++;
    }

    for (i = 0; i < NSIG; ++i) {
        if (ncaught[i]) {
            eb->activate_signal(eb, i, ncaught[i]);
        }
    }
    dbg_str(EV_DETAIL,"evsig_cb out");
}

static void __evsig_send(void *element, void *c)
{
    Event_Base *eb = (Event_Base *)element;
    Socket *sender = eb->evsig.sender;

    dbg_str(EV_VIP,"evsig_send signal=%d, event base:%p", *(char *)c, eb);
    sender->send(sender, (char *)c, 1, 0);

    return ;
}

static void signal_handler(int sig)
{
    char msg = (char) sig;
    List *list = global_event_base_list;;
    pthread_t tid = pthread_self();

    dbg_str(EV_VIP, "signal_handler receive signal=%d, event base count=%d, thread id:%d", 
            sig, list->count(list), tid);
    
    list->for_each_arg(list, __evsig_send, (void *)&msg);

    if (sig == SIGSEGV) {
#if (!defined(WINDOWS_USER_MODE))
        print_backtrace();
        exit(0);
#endif 
    }

    return ;
}

int evsig_init(Event_Base *eb)
{
    event_t *event = &eb->evsig.signal_event;
    struct evsig_s *evsig = &eb->evsig;
    allocator_t *allocator = eb->obj.allocator;
    Socket *s;
    char service[10];

    if (eb->signal_service) {
        dbg_str(DBG_DETAIL, "evsig_init, service:%s", STR2A(eb->signal_service));
        sprintf(service, "%s", STR2A(eb->signal_service));
    } else {
        sprintf(service, "%d", SIGNAL_SERVICE + 5);
    }
    
    s = (Socket *)object_new(allocator, "Inet_Udp_Socket", NULL);
    s->bind(s, "127.0.0.1", service);
    s->setnonblocking(s);
    //?? set closeonexec
    eb->evsig.receiver = s;

    s = (Socket *)object_new(allocator, "Inet_Udp_Socket", NULL);
    s->connect(s, "127.0.0.1", service);
    s->setnonblocking(s);
    //?? set closeonexec
    eb->evsig.sender = s;

    event->ev_fd        = eb->evsig.receiver->fd;
    event->ev_events    = EV_READ | EV_PERSIST;
    event->ev_tv.tv_sec = 0;
    event->ev_tv.tv_sec = 0;
    event->ev_base      = eb;
    event->ev_callback  = evsig_cb;
    event->ev_arg       = event;
    event->ev_socket    = eb->evsig.receiver;
    eb->add(eb, event);

    evsig->map  = OBJECT_NEW(allocator, RBTree_Map, NULL);
    evsig->list = OBJECT_NEW(allocator, Linked_List, NULL);

    set_quit_signal(eb);
    set_segment_signal(eb);

    return 0;
}

int evsig_release(Event_Base *eb)
{
    struct evsig_s *evsig = &eb->evsig;
    event_t *event = &eb->evsig.signal_event;

    eb->del(eb, event);
    object_destroy(evsig->map);
    object_destroy(evsig->list);
    object_destroy(evsig->receiver);
    object_destroy(evsig->sender);
}

int evsig_add(Event_Base *eb, event_t *event)
{
    int evsignal = event->ev_fd;
    Map *map = eb->evsig.map;
 
    dbg_str(EV_DETAIL,"add sig:%d", evsignal);
    signal(evsignal, signal_handler);

    map->add(map, event->ev_fd, event);

    return 0;
}

int evsig_del(Event_Base *eb, event_t *event)
{
    int evsignal = event->ev_fd;
    Map *map = eb->evsig.map;

    dbg_str(DBG_DETAIL,"del sig:%d", evsignal);

    map->del(map, evsignal); //?? need del disignated event

    return 0;
}

static void
quit_signal_cb(int fd, short event_res, void *arg)
{
    struct event *event = (struct event *)arg;
    Event_Base* eb = (Event_Base*)event->ev_base;

    dbg_str(DBG_WARN, "quit_signal_cb, signal no:%d", event->ev_fd);
    /* 只有break_flag为0的时候才置1， 因为break_flag为2时，其它模块期望延迟退出。*/
    if (eb->break_flag == 0) eb->break_flag = 1;
}

int set_quit_signal(Event_Base* eb)
{
    struct event *ev = &eb->quit_event;

#if (defined(WINDOWS_USER_MODE))
    // ev->ev_fd              = SIGQUIT;
    ev->ev_fd              = SIGINT; // ??SIGQUIT windows 编译不过先改为SIGINT
#else
    ev->ev_fd              = SIGINT;
#endif

    ev->ev_events          = EV_SIGNAL;
    ev->ev_timeout.tv_sec  = 0;
    ev->ev_timeout.tv_usec = 0;
    ev->ev_callback        = quit_signal_cb;
    ev->ev_base            = eb;
    ev->ev_arg             = ev;

    dbg_str(EV_INFO,"set_break_signal, fd=%d, ev_callback=%p, break_flag=%d",
            ev->ev_fd, ev->ev_callback,  eb->break_flag);

    eb->add(eb, ev);

    return (0);
}

static void
segment_signal_cb(int fd, short event_res, void *arg)
{
    struct event *event = (struct event *)arg;
    Event_Base* eb = (Event_Base*)event->ev_base;

    dbg_str(DBG_FATAL, "segment_signal_cb, signal no:%d", event->ev_fd);
    eb->break_flag = 1;
}

int set_segment_signal(Event_Base* eb)
{
    struct event *ev = &eb->coredump_event;

    ev->ev_fd              = SIGSEGV;
    ev->ev_events          = EV_SIGNAL;
    ev->ev_timeout.tv_sec  = 0;
    ev->ev_timeout.tv_usec = 0;
    ev->ev_callback        = segment_signal_cb;
    ev->ev_base            = eb;
    ev->ev_arg             = ev;

    dbg_str(EV_INFO,"set_break_signal, fd=%d, ev_callback=%p, break_flag=%d",
            ev->ev_fd, ev->ev_callback,  eb->break_flag);

    eb->add(eb, ev);

    return (0);
}
