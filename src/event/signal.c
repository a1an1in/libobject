#if (defined(WINDOWS_USER_MODE))
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libobject/event/Event_Base.h>
#include <libobject/event/event_compat.h>
#include <libobject/core/Rbtree_Map.h>
#include <libobject/core/Linked_List.h>
#include <libobject/core/utils/config.h>

int evsig_init(Event_Base *eb)
{
    return 0;
}

int evsig_release(Event_Base *eb)
{
}

int evsig_add(Event_Base *eb, event_t *event)
{
    return 0;
}

int evsig_del(Event_Base *eb, event_t *event)
{
    return 0;
}


int set_break_signal(Event_Base* eb)
{
    return (0);
}

int set_segment_signal(Event_Base* eb)
{
    return (0);
}
#else
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
#include <libobject/event/Select_Base.h>
#include <libobject/event/event_compat.h>
#include <libobject/core/Rbtree_Map.h>
#include <libobject/core/Linked_List.h>
#include <libobject/core/utils/config.h>

static int numeric_key_cmp_func(void *key1,void *key2,uint32_t size)
{
    int k1, k2;

    k1 = *((int *)key1);
    k2 = *((int *)key2);

    if (k1 > k2 ) return 1;
    else if (k1 < k2) return -1;
    else return 0;
}

static void
evsig_cb(int fd, short what, void *arg)
{
    static char signals[1024];
    int n;
    int i;
    int ncaught[NSIG] = {0};
    event_t *event = (event_t *)arg;
    Event_Base *eb = (Event_Base *)event->ev_base;
    Socket *receiver = event->ev_socket;

    dbg_str(EV_DETAIL,"evsig_cb in, eb:%p", eb);
    memset(&ncaught, 0, sizeof(ncaught));

    n = receiver->recv(receiver, signals, sizeof(signals), 0);
    if (n == -1) {
        dbg_str(EV_ERROR,"evsig_cb recv");
        return ;
    } else if (n == 0) {
    }

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

    dbg_str(EV_IMPORTANT,"evsig_send signal=%d, event base:%p", *(char *)c, eb);
    sender->send(sender, (char *)c, 1, 0);

    return ;
}

static void signal_handler(int sig)
{
    char msg        = (char) sig;
    pthread_t   tid = pthread_self();

    List *list = global_event_base_list;
    dbg_str(EV_IMPORTANT,"thread %lu receive signal=%d", tid, sig);
    dbg_str(EV_IMPORTANT,"list count=%d", list->count(list));

    list->for_each_arg(list, __evsig_send, (void *)&msg);
}

int evsig_init(Event_Base *eb)
{
    event_t *event = &eb->evsig.signal_event;
    struct evsig_s *evsig = &eb->evsig;
    allocator_t *allocator = eb->obj.allocator;
    List *list = global_event_base_list;
    Socket *s;
    char service[10];

    sprintf(service, "%d", 11011 + list->count(list));
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
    /*
     *event->ev_flags     = EV_SIGNAL_FD; //signal receiver fd
     */
    event->ev_socket    = eb->evsig.receiver;
    eb->add(eb, event);

    evsig->map  = OBJECT_NEW(allocator, RBTree_Map, NULL);
    evsig->map->set_cmp_func(evsig->map, default_key_cmp_func);
    evsig->list = OBJECT_NEW(allocator, Linked_List, NULL);

    return 0;
}

int evsig_release(Event_Base *eb)
{
    struct evsig_s *evsig = &eb->evsig;

    object_destroy(evsig->map);
    object_destroy(evsig->list);
    object_destroy(evsig->receiver);
    object_destroy(evsig->sender);
}

int evsig_add(Event_Base *eb, event_t *event)
{
    struct sigaction sa; 
    int evsignal = event->ev_fd;
    Map *map = eb->evsig.map;

#if 0
    sa.sa_handler = signal_handler;                    
    /*
     *sa.sa_flags |= SA_RESTART;                             
     */
    sa.sa_flags |= SA_INTERRUPT;                             
    sigfillset(&sa.sa_mask);                      

    if (sigaction(evsignal, &sa, NULL) == -1) {
        return (-1);
    }
#else 
    dbg_str(EV_DETAIL,"add sig:%d", evsignal);
    signal(evsignal, signal_handler);
#endif

    map->add(map, event->ev_fd, event);

    return 0;
}

int evsig_del(Event_Base *eb, event_t *event)
{
    struct sigaction sa; 
    int evsignal = event->ev_fd;
    Map *map = eb->evsig.map;

    dbg_str(DBG_DETAIL,"del sig:%d", evsignal);

    map->del(map, evsignal); //?? need del disignated event

    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  break event base loop singal callback
 *
 * @Param fd
 * @Param event_res
 * @Param arg
 */
/* ----------------------------------------------------------------------------*/
static void
break_signal_cb(int fd, short event_res, void *arg)
{
    struct event *event = (struct event *)arg;
    Event_Base* eb = (Event_Base*)event->ev_base;

    dbg_str(EV_IMPORTANT, "signal_cb, signal no:%d", event->ev_fd);
    eb->break_flag = 1;
}

int set_break_signal(Event_Base* eb)
{
    struct event *ev = &eb->break_event;

    ev->ev_fd              = SIGQUIT;
    ev->ev_events          = EV_SIGNAL|EV_PERSIST;
    ev->ev_timeout.tv_sec  = 0;
    ev->ev_timeout.tv_usec = 0;
    ev->ev_callback        = break_signal_cb;
    ev->ev_base            = eb;
    ev->ev_arg             = ev;

    dbg_str(EV_IMPORTANT,"set_break_signal, fd=%d, ev_callback=%p, break_flag=%d",
            ev->ev_fd, ev->ev_callback,  eb->break_flag);

    eb->add(eb, ev);

    return (0);
}

static void
segment_signal_cb(int fd, short event_res, void *arg)
{
    print_backtrace();
}
int set_segment_signal(Event_Base* eb)
{
    struct event *ev = &eb->break_event;

    ev->ev_fd              = SIGSEGV;
    ev->ev_events          = EV_SIGNAL;
    ev->ev_timeout.tv_sec  = 0;
    ev->ev_timeout.tv_usec = 0;
    ev->ev_callback        = segment_signal_cb;
    ev->ev_base            = eb;
    ev->ev_arg             = ev;

    dbg_str(EV_IMPORTANT,"set_break_signal, fd=%d, ev_callback=%p, break_flag=%d",
            ev->ev_fd, ev->ev_callback,  eb->break_flag);

    eb->add(eb, ev);

    return (0);
}
#endif
