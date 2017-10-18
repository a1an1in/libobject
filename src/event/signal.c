#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include <time.h>
#include <sys/stat.h>
#include <libobject/event/select_base.h>
#include <libobject/event/event_compat.h>

static int gloable_evsig_send_fd;
static int gloable_evsig_rcv_fd;

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

    dbg_str(EV_DETAIL,"evsig_cb, NSIG=%d", NSIG);
    memset(&ncaught, 0, sizeof(ncaught));

    n = recv(fd, signals, sizeof(signals), 0);
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

static void signal_handler(int sig)
{
    char msg = (char) sig;

    dbg_str(EV_IMPORTANT,"signal_handler %d, gloable_evsig_send_fd =%d, sig=%d",
            signal, gloable_evsig_send_fd, sig);

    send(gloable_evsig_send_fd, (char*)&msg, 1, 0);
}

int
evsig_socketpair(int fd[2])
{
    struct sockaddr_in listen_addr;
    struct sockaddr_in connect_addr;
    int listener    = -1;
    int connector   = -1;
    int acceptor    = -1;
    int saved_errno = -1;
    int protocol    = 0;
    int type        = SOCK_STREAM;
    int size;

    if (!fd) {
        return -1;
    }

    listener = socket(AF_INET, type, 0);
    if (listener < 0)
        return -1;

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family      = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    listen_addr.sin_port        = 0;   /* kernel chooses port.  */

    if (bind(listener, (struct sockaddr *) &listen_addr, sizeof (listen_addr)) == -1)
        goto error;

    if (listen(listener, 1) == -1)
        goto error;

    if ((connector = socket(AF_INET, type, 0)) < 0)
        goto error;

    /* We want to find out the port number to connect to.  */
    size = sizeof(connect_addr);
    if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1)
        goto error;
    if (size != sizeof (connect_addr))
        goto error;

    if (connect(connector, (struct sockaddr *) &connect_addr,
                sizeof(connect_addr)) == -1)
        goto error;

    size = sizeof(listen_addr);
    acceptor = accept(listener, (struct sockaddr *) &listen_addr, &size);
    if (acceptor < 0)
        goto error;
    if (size != sizeof(listen_addr))
        goto error;

    /* Now check we are talking to ourself by matching port and host on the
       two sockets.  */
    if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) == -1)
        goto error;
    if (    size != sizeof (connect_addr) || 
            listen_addr.sin_family != connect_addr.sin_family || 
            listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr || 
            listen_addr.sin_port != connect_addr.sin_port) 
    {
        goto error;
    }

    fd[0] = connector;
    fd[1] = acceptor;

    close(listener);

    return 0;

error:
    if (listener != -1)
        close(listener);
    if (connector != -1)
        close(connector);
    if (acceptor != -1)
        close(acceptor);

    return -1;
}

int
evsig_make_socket_closeonexec(int fd)
{
    int flags;

    if ((flags = fcntl(fd, F_GETFD, NULL)) < 0) {
        dbg_str(EV_WARNNING,"fcntl(%d, F_GETFD)", fd);
        return -1;
    }
    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        dbg_str(EV_WARNNING,"fcntl(%d, F_SETFD)", fd);
        return -1;
    }

    return 0;
}

int
evsig_make_socket_nonblocking(int fd)
{
    int flags;

    if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
        dbg_str(EV_WARNNING,"fcntl(%d, F_GETFL)", fd);
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        dbg_str(EV_WARNNING,"fcntl(%d, F_SETFD)", fd);
        return -1;
    }

    return 0;
}

int evsig_init(Event_Base *eb)
{
    int fds[2];
    event_t *event = &eb->evsig.fd_rcv_event;
    struct evsig_s *evsig = &eb->evsig;
    allocator_t *allocator = eb->obj.allocator;

#if 0
    if (pipe(fds)) {
        dbg_str(SM_ERROR,"cannot create pipe");
        return -1;
    }

#else
    evsig_socketpair(fds);
#endif

    eb->evsig.fd_rcv = fds[1];
    eb->evsig.fd_snd = fds[0];

    dbg_str(EV_DETAIL,"evsig_init, gloable_evsig_send_fd=%d, fd_rcv =%d",
            eb->evsig.fd_snd, eb->evsig.fd_rcv);

    gloable_evsig_send_fd = eb->evsig.fd_snd;
    gloable_evsig_rcv_fd = eb->evsig.fd_rcv;
    evsig_make_socket_closeonexec(eb->evsig.fd_snd);
    evsig_make_socket_closeonexec(eb->evsig.fd_rcv);
    evsig_make_socket_nonblocking(eb->evsig.fd_snd);
    evsig_make_socket_nonblocking(eb->evsig.fd_rcv);

    event->ev_fd        = eb->evsig.fd_rcv;
    event->ev_events    = EV_READ | EV_PERSIST;
    event->ev_tv.tv_sec = 0;
    event->ev_tv.tv_sec = 0;
    event->ev_base      = eb;
    event->ev_callback  = evsig_cb;
    event->ev_arg       = event;
    eb->add(eb, event);

    evsig->sig_map = rbtree_map_alloc(allocator);
    evsig->sig_map->key_cmp_func = numeric_key_cmp_func;
    rbtree_map_init(evsig->sig_map, 4,sizeof(void *)); 

    return 0;
}

int evsig_release(Event_Base *eb)
{
    struct evsig_s *evsig = &eb->evsig;

    rbtree_map_destroy(evsig->sig_map);

    close(evsig->fd_rcv);
    close(evsig->fd_snd);
}

int evsig_add(Event_Base *eb, event_t *event)
{
    struct sigaction sa; 
    int evsignal = event->ev_fd;
    rbtree_map_t *sig_map = eb->evsig.sig_map;

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
    signal(evsignal,signal_handler);
#endif

    rbtree_map_insert_with_numeric_key(sig_map,evsignal,event);

    return 0;
}

static void
break_signal_cb(int fd, short event_res, void *arg)
{
    struct event *event = (struct event *)arg;
    Event_Base* eb = (Event_Base*)event->ev_base;

    dbg_str(EV_SUC, "signal_cb, signal no:%d", event->ev_fd);
    eb->break_flag = 1;
}

int set_break_signal(Event_Base* eb)
{
    struct event *ev = &eb->break_event;

    ev->ev_fd              = SIGINT;
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
