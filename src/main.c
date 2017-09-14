/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/19/2015 5:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
/*  
 * Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 *  
 *  
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
#include <libdbg/debug.h>
#include <libargs/cmd_args.h>
#include <attrib_priority.h>
#include <libbus/busd.h>
#include <libbus/bus.h>
#include <libobject/test.h>

#define LIBRARY_VERSION "libobject version: 2.0.0.0"

#ifndef MAKELIB

typedef struct config_list_s{
}config_list_t;

typedef struct base_s{
    config_list_t config;
    args_processor_t *p;
}base_t;


static int args_process_help(void *base,int argc,char **argv)
{
    args_print_help_info(args_get_processor_globle_addr());
    exit(1);
    return 0;
}

static int args_process_ipaddr(void *base,int argc,char **argv)
{
    dbg_str(DBG_DETAIL,"args_process_ipaddr:%s",argv[0]);
    return 1;
}

static int args_process_port(void *base,int argc,char **argv)
{
    dbg_str(DBG_DETAIL,"args_process_port:%s",argv[0]);
    return 1;
}

static int args_process_test_object_config(void *base,int argc,char **argv)
{
    test_object_config();

    return 0;
}

static int args_process_test_obj(void *base,int argc,char **argv)
{
    test_obj();
    return 0;
}

static int args_process_test_obj_subject(void *base,int argc,char **argv)
{
    test_obj_subject();
    return 0;
}

static int args_process_test_obj_enemy(void *base,int argc,char **argv)
{
    test_obj_enemy();
    return 0;
}

static int args_process_test_sdl(void *base,int argc,char **argv)
{
    /*
     *test_sdl(argc, argv);
     */
    return 0;
}

static int args_process_test_ui_container(void *base,int argc,char **argv)
{
    test_ui_container();
    return 0;
}

static int args_process_test_string(void *base,int argc,char **argv)
{
    test_obj_string();
    return 0;
}

static int args_process_test_Map(void *base,int argc,char **argv)
{
    test_obj_map();
    return 0;
}

static int args_process_test_Iterator(void *base,int argc,char **argv)
{
    test_obj_iter();
    return 0;
}

static int args_process_test_Hmap_Iterator(void *base,int argc,char **argv)
{
    test_obj_hiter();
    return 0;
}

static int args_process_test_Hmap(void *base,int argc,char **argv)
{
    test_obj_hash_map();
    return 0;
}

static int args_process_test_Graph(void *base,int argc,char **argv)
{
    test_ui_graph();
    return 0;
}
static int args_process_test_SDL_Window(void *base,int argc,char **argv)
{
    test_ui_sdl_window();
    return 0;
}

static int args_process_test_Image(void *base,int argc,char **argv)
{
    test_obj_image();
    return 0;
}

static int args_process_test_SDL_Image(void *base,int argc,char **argv)
{
    test_obj_sdl_image();
    return 0;
}

static int args_process_test_SDL_Font(void *base,int argc,char **argv)
{
    test_obj_sdl_font();
    return 0;
}

static int args_process_test_SDL_Text(void *base,int argc,char **argv)
{
    /*
     *test_obj_sdl_text();
     */
    return 0;
}

static int args_process_test_SDL_Character(void *base,int argc,char **argv)
{
    
    test_obj_sdl_character();
    return 0;
}

static int args_process_test_SDL_Event(void *base,int argc,char **argv)
{
    test_obj_sdl_event();
    return 0;
}

static int args_process_test_Label(void *base,int argc,char **argv)
{
    test_ui_label();
    return 0;
}

static int args_process_test_LList(void *base,int argc,char **argv)
{
    test_obj_llist_list();
    return 0;
}

static int args_process_test_Text(void *base,int argc,char **argv)
{
    test_obj_text();
    return 0;
}

static int args_process_test_Text_Area(void *base,int argc,char **argv)
{
    test_ui_text_area();
    return 0;
}

static int args_process_test_Text_Field(void *base,int argc,char **argv)
{
    test_ui_text_field();
    return 0;
}

static int args_process_test_sdl_timer(void *base,int argc,char **argv)
{
    test_obj_sdl_timer();
    return 0;
}

static int args_process_test_gridlayout(void *base,int argc,char **argv)
{
    test_ui_grid_layout();
    return 0;
}

static int args_process_test_borderlayout(void *base,int argc,char **argv)
{
    test_ui_border_layout();
    return 0;
}

static int args_process_test_ui_button(void *base,int argc,char **argv)
{
    test_ui_button();
    return 0;
}

static int args_process_test_event_base(void *base,int argc,char **argv)
{
    test_event_io();
    /*
     *test_obj_select_base();
     */
    /*
     *test_obj_eb();
     */
    return 0;
}
static cmd_config_t cmds[]={
    {"Event", args_process_test_event_base,0, "test", "N/A","test"},
    {"Button", args_process_test_ui_button,0, "test", "N/A","test"},
    {"Border", args_process_test_borderlayout,0, "test", "N/A","test"},
    {"Grid", args_process_test_gridlayout,0, "test", "N/A","test"},
    {"Sdl_Timer", args_process_test_sdl_timer,0, "test", "N/A","test"},
    {"TF", args_process_test_Text_Field,0, "test", "N/A","test"},
    {"TA", args_process_test_Text_Area,0, "test", "N/A","test"},
    {"Text", args_process_test_Text,0, "test", "N/A","test"},
    {"LList", args_process_test_LList,0, "test", "N/A","test"},
    {"Label", args_process_test_Label,0, "test", "N/A","test"},
    {"SDL_Event", args_process_test_SDL_Event,0, "test", "N/A","test"},
    {"SDL_Char", args_process_test_SDL_Character,0, "test", "N/A","test"},
    {"SDL_Text", args_process_test_SDL_Text,0, "test", "N/A","test"},
    {"SDL_Font", args_process_test_SDL_Font,0, "test", "N/A","test"},
    {"SDL_Image", args_process_test_SDL_Image,0, "test", "N/A","test"},
    {"Image", args_process_test_Image,0, "test", "N/A","test"},
    {"SDL_Window", args_process_test_SDL_Window,0, "test", "N/A","test"},
    {"Graph", args_process_test_Graph,0, "test", "N/A","test"},
    {"HMap", args_process_test_Hmap,0, "test", "N/A","test"},
    {"HIter", args_process_test_Hmap_Iterator,0, "test", "N/A","test"},
    {"Iter", args_process_test_Iterator,0, "test", "N/A","test"},
    {"Map", args_process_test_Map,0, "test", "N/A","test"},
    {"String", args_process_test_string,0, "test", "N/A","test"},
    {"Container", args_process_test_ui_container,0, "test", "N/A","test"},
    {"sdl", args_process_test_sdl,0, "test", "N/A","test"},
    {"Enemy", args_process_test_obj_enemy,0, "test", "N/A","obj"},
    {"Subject", args_process_test_obj_subject,0, "test", "N/A","obj"},
    {"Obj", args_process_test_obj,0, "test", "N/A","obj"},
    {"object_config", args_process_test_object_config,0, "test", "N/A","obj"},
    {"port", args_process_port,1, "config", "NN(number)","udp port,using to send/rcv msg with neighbor"},
    {"ip", args_process_ipaddr,1, "config", "xx.xx.xx.xx","ip addr,using to send/rcv msg with neighbor"},
    {"help", args_process_help,0, "help", "N/A","help info"},
    {NULL,NULL,0,NULL,NULL,NULL},
};

/*
 * The libs used modules has been registered before main func,
 * and debugger has been construct before main too. so can use
 * it derectly.
 */
int main(int argc, char *argv[])
{
    int ret = 0;

    dbg_str(DBG_DETAIL,"main func start");

    args_process(NULL,cmds,argc, argv);

    dbg_str(DBG_DETAIL,"main func end");

#if 0
    pause();
    dbg_str(DBG_DETAIL,"main func out,but pause breaked");
#endif

    return ret;
}
#endif

__attribute__((constructor(ATTRIB_PRIORITY_VERSION))) void
print_library_version()
{
    ATTRIB_PRINT("constructor ATTRIB_PRIORITY_VERSION=%d,%s\n",
                 ATTRIB_PRIORITY_VERSION,
                 LIBRARY_VERSION);
}
