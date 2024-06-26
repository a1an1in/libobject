/**
 * @file Url.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "libobject/net/url/Url.h"

static int __construct(Url *url, char *init_str)
{
    allocator_t *allocator = url->parent.allocator;

    url->raw = (String *)object_new(allocator, "String", NULL);

    return 0;
}

static int __deconstruct(Url *url)
{
    if (url->raw != NULL) {
        object_destroy(url->raw);
    }

    if (url->scheme != NULL) {
        object_destroy(url->scheme);
    }
        
    if (url->user != NULL) {
        object_destroy(url->user);
    }

    if (url->password != NULL) {
        object_destroy(url->password);
    }

    if (url->host != NULL) {
        object_destroy(url->host);
    }

    if (url->port != NULL) {
        object_destroy(url->port);
    }

    if (url->path != NULL) {
        object_destroy(url->path);
    }

    if (url->query != NULL) {
        object_destroy(url->query);
    }

    if (url->fragment != NULL) {
        object_destroy(url->fragment);
    }

    return 0;
}

/*
 *scheme:[//[user[:password]@]host[:port]][/path][?query][#fragment]
 */
static int __parse_scheme(Url *url)
{
    allocator_t *allocator = url->parent.allocator;
    String *scheme, *raw = url->raw;
    String *user;
    int cnt;

    if (raw == NULL) return NULL;

    scheme = (String *)object_new(allocator, (char *)"String", NULL);
    scheme->assign(scheme, raw->get_cstr(raw));
    url->scheme = scheme;

    cnt = scheme->replace(scheme, "://", "/", -1);
    if (cnt != 1) {
        object_destroy(scheme);
        return -1;
    }

    cnt = scheme->split(scheme, (char *)"/", 1);
    user = (String *)object_new(allocator, (char *)"String", NULL);
    if (cnt == 2) {
        dbg_str(DBG_SUC, "scheme:%s", scheme->get_cstr(scheme));
        url->scheme = scheme;
        user->assign(user, scheme->get_splited_cstr(scheme, 1));
    } else {
        user->assign(user, scheme->get_splited_cstr(scheme, 0));
        object_destroy(scheme);
    }

    url->user = user;

    return 1;
}

static int __parse_user_and_password(Url *url)
{
    allocator_t *allocator = url->parent.allocator;
    String *user, *host, *password;
    int cnt;

    /*parse user and password*/
    user = url->user;
    if (user == NULL) return NULL;

    host = (String *)object_new(allocator, (char *)"String", NULL);
    password = (String *)object_new(allocator, (char *)"String", NULL);
    cnt= user->split(user, (char *)"@", 1);
    if (cnt == 2) {
        dbg_str(DBG_SUC, "user:%s", user->get_cstr(user));
        dbg_str(DBG_SUC, "host:%s",user->get_splited_cstr(user, 1));
        host->assign(host, user->get_splited_cstr(user, 1));

        cnt= user->split(user, (char *)":", 1);
        if (cnt == 2) {
            password->assign(password, user->get_splited_cstr(user, 1));
            dbg_str(DBG_SUC, "password:%s",user->get_splited_cstr(user, 1));
            url->password = password;
        } else {
            object_destroy(password);
        }
    } else {
        dbg_str(DBG_SUC, "host:%s",user->get_splited_cstr(user, 0));
        host->assign(host, user->get_splited_cstr(user, 0));
        object_destroy(user);
        object_destroy(password);
        dbg_str(DBG_SUC, "destroy user");
        dbg_str(DBG_DETAIL, "user addr:%p", url->user);
        url->user = NULL;
        dbg_str(DBG_DETAIL, "user addr:%p", url->user);
    }

    url->host = host;

    return 1;
}

static int __parse_host(Url *url)
{
    allocator_t *allocator = url->parent.allocator;
    String *host, *port, *path;
    int cnt;

    host = url->host;
    if (host == NULL) return 0;

    port = (String *)object_new(allocator, (char *)"String", NULL);
    path = (String *)object_new(allocator, (char *)"String", NULL);

    /*parse host and port*/
    cnt= host->split(host, (char *)":", 1);
    if (cnt == 2) {
        port->assign(port, host->get_splited_cstr(host, 1));
        dbg_str(DBG_SUC, "port:%s",host->get_splited_cstr(host, 1));
        url->port = port;
        cnt= port->split(port, (char *)"/", 1);
        if (cnt == 2) {
            path->assign(path, "/");
            path->append(path, port->get_splited_cstr(port, 1), -1);
            dbg_str(DBG_SUC, "path:%s",port->get_splited_cstr(port, 1));
            url->path = path;
        } else {
            object_destroy(path);
            return 1;
        }
    } else {
        object_destroy(port);
        cnt= host->split(host, (char *)"/", 1);
        if (cnt == 2) {
            path->assign(path, "/");
            path->append(path, host->get_splited_cstr(host, 1), -1);
            dbg_str(DBG_SUC, "path:%s",host->get_splited_cstr(host, 1));
            url->host = host;
        } else {
            object_destroy(path);
            url->host = host;
            return 1;
        }

        url->path = path;
    }

    return 1;
}

static int __parse_path(Url *url)
{
    allocator_t *allocator = url->parent.allocator;
    String *path, *query;
    int cnt;
    
    path = url->path;

    if (path == NULL) return 0;

    query = (String *)object_new(allocator, (char *)"String", NULL);

    /*parse path*/
    cnt= path->split(path, (char *)"[?]", 1);
    if (cnt == 2) {
        dbg_str(DBG_SUC, "path:%s", path->get_cstr(path));
        query->assign(query, path->get_splited_cstr(path, 1));
        url->query = query;
        return 1;
    } else {
        object_destroy(query);
    }

    return 1;
}

static int __parse_query(Url *url)
{
    allocator_t *allocator = url->parent.allocator;
    String *query, *fragment;
    int cnt;
    
    query = url->query;

    if (query == NULL) return 0;
    printf("parse query: %p\n", query);

    fragment = (String *)object_new(allocator, (char *)"String", NULL);

    cnt= query->split(query, (char *)"#", 1);
    if (cnt == 2) {
        dbg_str(DBG_SUC, "query:%s", query->get_cstr(query));
        dbg_str(DBG_SUC, "fragment:%s", query->get_splited_cstr(query, 1));
        fragment->assign(fragment, query->get_splited_cstr(query, 1));
        url->query = query;
        url->fragment= fragment;
    } else {
        dbg_str(DBG_SUC, "query:%s", query->get_cstr(query));
        object_destroy(fragment);
    }

    return 1;
}

static int __parse(Url *url, char *u)
{
    String *raw = url->raw;
    int ret;
    
    raw->assign(raw, u);

     ret = __parse_scheme(url);
     if (ret < 0) {
         return ret;
     }

     ret = __parse_user_and_password(url);
     if (ret < 0) {
         return ret;
     }

     ret = __parse_host(url);
     if (ret < 0) {
         return ret;
     }

     ret = __parse_path(url);
     if (ret < 0) {
         return ret;
     }

     ret = __parse_query(url);
     if (ret < 0) {
         return ret;
     }

     return 1;
}

static int __info(Url *url)
{
    if (url->raw != NULL) {
        dbg_str(DBG_DETAIL, "raw:%s", url->raw->get_cstr(url->raw));
    }

    if (url->scheme != NULL) {
        dbg_str(DBG_DETAIL, "scheme:%s", url->scheme->get_cstr(url->scheme));
    }
        
    if (url->user != NULL) {
        dbg_str(DBG_DETAIL, "user:%s", url->user->get_cstr(url->user));
    }

    if (url->password != NULL) {
        dbg_str(DBG_DETAIL, "password:%s", url->password->get_cstr(url->password));
    }

    if (url->host != NULL) {
        dbg_str(DBG_DETAIL, "host:%s", url->host->get_cstr(url->host));
    }

    if (url->port != NULL) {
        dbg_str(DBG_DETAIL, "port:%s", url->port->get_cstr(url->port));
    }

    if (url->path != NULL) {
        dbg_str(DBG_DETAIL, "path:%s", url->path->get_cstr(url->path));
    }

    if (url->query != NULL) {
        dbg_str(DBG_DETAIL, "query:%s", url->query->get_cstr(url->query));
    }

    if (url->fragment != NULL) {
        dbg_str(DBG_DETAIL, "fragment:%s", url->fragment->get_cstr(url->fragment));
    }
}

static int __reset(Url *url)
{
    if (url->raw != NULL) {
        url->raw->reset(url->raw);
    }

    if (url->scheme != NULL) {
        object_destroy(url->scheme);
        url->scheme = NULL;
    }
        
    if (url->user != NULL) {
        object_destroy(url->user);
        url->user = NULL;
    }

    if (url->password != NULL) {
        object_destroy(url->password);
        url->password = NULL;
    }

    if (url->host != NULL) {
        object_destroy(url->host);
        url->host = NULL;
    }

    if (url->port != NULL) {
        object_destroy(url->port);
        url->port = NULL;
    }

    if (url->path != NULL) {
        object_destroy(url->path);
        url->path = NULL;
    }

    if (url->query != NULL) {
        object_destroy(url->query);
        url->query = NULL;
    }

    if (url->fragment != NULL) {
        object_destroy(url->fragment);
        url->fragment = NULL;
    }

    return 0;
}

static class_info_entry_t url_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Url, construct, __construct),
    Init_Nfunc_Entry(2, Url, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Url, parse, __parse),
    Init_Vfunc_Entry(4, Url, info, __info),
    Init_Vfunc_Entry(5, Url, reset, __reset),
    Init_End___Entry(6, Url),
};
REGISTER_CLASS(Url, url_class_info);

