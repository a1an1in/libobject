/**
 * @file Url_Test.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
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
#include <stdio.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/mockery/mockery.h>
#include "Url_Test.h"

static int __construct(Url_Test *test, char *init_str)
{
    test->url = object_new(test->parent.obj.allocator, "Url", NULL);

    return 0;
}

static int __deconstruct(Url_Test *test)
{
    if (test->url != NULL) {
        object_destroy(test->url);
    }

    return 0;
}

static int __setup(Url_Test *test, char *init_str)
{
    dbg_str(DBG_DETAIL,"Url_Test set up");

    return 1;
}

static int __teardown(Url_Test *test)
{
    test->url->reset(test->url);
    return 1;
}

static void __test_parse_scheme(Url_Test *test)
{
    Url *url = test->url;
    char *scheme = "http";
    char *host = "www.libobject.com";
    char buf[1024];

    sprintf(buf, "%s://%s", scheme, host);
    url->parse(url, buf);
    url->info(url);

    ASSERT_EQUAL(test, scheme, url->scheme->get_cstr(url->scheme), 
                 strlen(scheme));
}

static void __test_parse_user(Url_Test *test)
{
    Url *url = test->url;
    char *scheme = "http";
    char *user = "alan";
    char *host = "www.libobject.com";
    char buf[1024];

    sprintf(buf, "%s://%s@%s", scheme, user, host);
    url->parse(url, buf);
    url->info(url);

    ASSERT_EQUAL(test, user, url->user->get_cstr(url->user), 
                 strlen(user));
}

static void __test_parse_password(Url_Test *test)
{
    Url *url = test->url;
    char *scheme = "http";
    char *user = "alan";
    char *password = "123456";
    char *host = "www.libobject.com";
    char buf[1024];

    sprintf(buf, "%s://%s:%s@%s", scheme, user, password, host);
    url->parse(url, buf);
    url->info(url);

    ASSERT_EQUAL(test, password, url->password->get_cstr(url->password), 
                 strlen(password));
}

static void __test_parse_host(Url_Test *test)
{
    Url *url = test->url;
    char *scheme = "http";
    char *user = "alan";
    char *password = "123456";
    char *host = "www.libobject.com";
    char buf[1024];

    sprintf(buf, "%s://%s:%s@%s", scheme, user, password, host);
    url->parse(url, buf);
    url->info(url);

    ASSERT_EQUAL(test, host, url->host->get_cstr(url->host), 
                 strlen(host));
}

static void __test_parse_port(Url_Test *test)
{
    Url *url = test->url;
    char *scheme = "http";
    char *user = "alan";
    char *password = "123456";
    char *host = "www.libobject.com";
    char *port = "8080";
    char buf[1024];

    sprintf(buf, "%s://%s:%s@%s:%s", scheme, user, password, host, port);
    url->parse(url, buf);
    url->info(url);

    ASSERT_EQUAL(test, port, url->port->get_cstr(url->port), 
                 strlen(port));
}

static void __test_parse_path(Url_Test *test)
{
    Url *url = test->url;
    char *scheme = "http";
    char *user = "alan";
    char *password = "123456";
    char *host = "www.libobject.com";
    char *port = "8080";
    char *path = "net/abc";
    char *expect_path = "/net/abc";
    char buf[1024];

    sprintf(buf, "%s://%s:%s@%s:%s/%s", scheme, user, password,
                                        host, port, path);
    url->parse(url, buf);
    url->info(url);

    ASSERT_EQUAL(test, expect_path, url->path->get_cstr(url->path), 
                 strlen(expect_path));
}

static void __test_parse_query(Url_Test *test)
{
    Url *url = test->url;
    char *scheme = "http";
    char *user = "alan";
    char *password = "123456";
    char *host = "www.libobject.com";
    char *port = "8080";
    char *path = "net/abc";
    char *query = "name=alan";
    char buf[1024];

    sprintf(buf, "%s://%s:%s@%s:%s/%s?%s", scheme, user, password,
                                           host, port, path, query);
    url->parse(url, buf);
    url->info(url);

    ASSERT_EQUAL(test, query, url->query->get_cstr(url->query), 
                 strlen(query));
}

static void __test_parse_fragment(Url_Test *test)
{
    Url *url = test->url;
    char *scheme = "http";
    char *user = "alan";
    char *password = "123456";
    char *host = "www.libobject.com";
    char *port = "8080";
    char *path = "net/abc";
    char *query = "name=alan";
    char *fragment = "ldb34";
    char buf[1024];

    sprintf(buf, "%s://%s:%s@%s:%s/%s?%s#%s",
            scheme, user, password, host, port,
            path, query, fragment);
    url->parse(url, buf);
    url->info(url);

    ASSERT_EQUAL(test, fragment, url->fragment->get_cstr(url->fragment), 
                 strlen(fragment));
}

static class_info_entry_t url_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Url_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Url_Test, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Url_Test, set, NULL),
    Init_Vfunc_Entry(4 , Url_Test, get, NULL),
    Init_Vfunc_Entry(5 , Url_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Url_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Url_Test, test_parse_scheme, __test_parse_scheme),
    Init_Vfunc_Entry(8 , Url_Test, test_parse_user, __test_parse_user),
    Init_Vfunc_Entry(9 , Url_Test, test_parse_password, __test_parse_password),
    Init_Vfunc_Entry(10, Url_Test, test_parse_host, __test_parse_host),
    Init_Vfunc_Entry(11, Url_Test, test_parse_port, __test_parse_port),
    Init_Vfunc_Entry(12, Url_Test, test_parse_path, __test_parse_path),
    Init_Vfunc_Entry(13, Url_Test, test_parse_query, __test_parse_query),
    Init_Vfunc_Entry(14, Url_Test, test_parse_fragment, __test_parse_fragment),
    Init_End___Entry(15, Url_Test),
};
REGISTER_CLASS("Url_Test", url_test_class_info);
