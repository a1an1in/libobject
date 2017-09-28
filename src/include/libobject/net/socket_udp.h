#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/socket.h>

typedef struct udp_socket_s Udp_Socket;

struct udp_socket_s{
	Socket parent;

	int (*construct)(Udp_Socket *,char *init_str);
	int (*deconstruct)(Udp_Socket *);
	int (*set)(Udp_Socket *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
