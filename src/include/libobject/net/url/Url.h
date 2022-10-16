#ifndef __URL_H__
#define __URL_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>

typedef struct Url_s Url;

/*
 *URL format：
 *scheme:[//[user[:password]@]host[:port]][/path][?query][#fragment]
 */
struct Url_s{
    Obj parent;

    int (*construct)(Url *,char *);
    int (*deconstruct)(Url *);

    /*virtual methods reimplement*/
    int (*set)(Url *url, char *attrib, void *value);
    void *(*get)(Url *, char *attrib);
    char *(*to_json)(Url *); 
    void *(*get_value)(Url *url,char *url_name, char *flag_name);
    int (*parse)(Url *url, char *u);
    int (*info)(Url *url);
    int (*reset)(Url *url);

    /*attribs*/
    String *raw;
    String *scheme;
    String *user;
    String *password;
    String *host;
    String *port;
    String *path;
    String *query;
    String *fragment;
};

#endif
