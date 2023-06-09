#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/net/http/Request.h>
#include <libobject/net/http/Response.h>
#include <libobject/concurrent/net/api.h>
#include <libobject/argument/Command.h>

typedef struct http_client_s Http_Client;

struct http_client_s{
    Obj obj;

    int (*construct)(Http_Client *,char *init_str);
    int (*deconstruct)(Http_Client *);
    int (*set)(Http_Client *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    Request *(*get_request)(Http_Client *client);
    Response *(*get_response)(Http_Client *client);
    int (*request)(Http_Client *client, int (*request_cb)(void *, void *), void *arg);
    Response *(*request_sync)(Http_Client *client);

    /*attribs*/
    Request *req;
    Response *resp;
    int (*request_cb)(void *resp, void *arg);
    void *request_cb_arg;
    Client *c;
    char *host;
    char *remote_host;
    char *remote_service;
    Command *command;
    Object_Chain *chain;
};

#endif
