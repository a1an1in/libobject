/**
 * @file busd.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 
 * @date 2016-10-28
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
#if (!defined(WINDOWS_USER_MODE))
#include <stdio.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/miscellany/buffer.h>
#include <libobject/net/bus/busd.h>
#include <libobject/net/bus/bus.h>
#include <libobject/core/utils/registry/registry.h>

int test_bus_daemon()
{
    allocator_t *allocator = allocator_get_default_instance();
    busd_t *busd;
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
    int ret;

    TRY {
        dbg_str(BUS_DETAIL,"test_busd_daemon");

        busd = busd_create(allocator,
                        deamon_host,
                        deamon_srv, 
                        SERVER_TYPE_INET_TCP);
    } CATCH (ret) {} FINALLY {
        /*
        *while(1) sleep(1);
        */
#if (defined(WINDOWS_USER_MODE))
        system("pause");
#else
        pause();
#endif
        dbg_str(DBG_DETAIL,"run at here");
        busd_destroy(busd);
        dbg_str(DBG_DETAIL,"run at here");
    }

    return ret;
}
REGISTER_TEST_CMD(test_bus_daemon);
#endif
