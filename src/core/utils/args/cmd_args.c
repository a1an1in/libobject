/**
 * @file cmd_args.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 1.0
 * @date 2016-09-08
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

#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/args/cmd_args.h>

args_processor_t processor_globle;
args_processor_t *args_get_processor_globle_addr()
{
    return &processor_globle;
}

args_processor_t *
args_init(args_processor_t *p, void *base, cmd_config_t *c)
{
    p->base = base;
    p->cmd_config = c;

    return p;
}
cmd_config_t * 
args_find_entry(args_processor_t *p, char *cmd)
{
    cmd_config_t *config_head;
    int i;

    config_head = p->cmd_config;
    for(i = 0; config_head[i].cmd != NULL; i++){
        if(!strcmp(cmd, config_head[i].cmd)){
            return &config_head[i];
        }
    }

    dbg_str(ARGS_DETAIL, "not find cmd");

    return NULL;
}
int 
args_parse_args(args_processor_t *p, int argc, char *argv[])
{
    int i = 0;
    cmd_config_t *c; 
    uint8_t args_count;

    dbg_str(ARGS_DETAIL, "argc =%d", argc);

    for(i = 0; i < argc;){
        c = args_find_entry(p, argv[i]);
        if(c == NULL){
            dbg_str(ARGS_WARNNING, 
                    "%s is not valid cmd, please check.We'll skip this cmd", 
                    argv[i]);
            i++;
            continue;
        } else {
            dbg_str(ARGS_DETAIL, "process cmd %s", argv[i]);
            i++;
            args_count = c->fn(p->base, argc - i, argv + i);
            if(args_count != c->args_count){
                /*
                 *dbg_str(ARGS_WARNNING, 
                 *        "the args funtion process args count is not equal the setting");
                 */
            }
            /*
             *i+= c->args_count;
             */
            i+= args_count;
            dbg_str(ARGS_DETAIL, "args count=%d", args_count);
        }
    }

    return 0;
}
void args_print_help_info(args_processor_t *p)
{
    cmd_config_t *config_head;
    int i;
    char r1[] = "cmd name";
    char r2[] = "format";
    char r3[] = "description";

    printf("help_info:(xxx represent the program name)\n");
    config_head = p->cmd_config;
    for(i = 0; config_head[i].cmd != NULL; i++){
        printf("%-20s", (config_head + i)->cmd);
        printf("%s%s:%s %s %s\n", "", r2, "xxx", 
               (config_head + i)->cmd, 
               (config_head + i)->args_readable_names);
        if(strlen((config_head + i)->help_info))
            printf("%-20s%s:%s\n", "", r3, (config_head + i)->help_info);
    }

    return;
}
void args_print_help_test_info(args_processor_t *p)
{
    cmd_config_t *config_head;
    int i;
    char r1[] = "cmd name";
    char r2[] = "format";
    char r3[] = "description";

    printf("help_test_info:(xxx represent the program name)\n");
    config_head = p->cmd_config;
    for(i = 0; config_head[i].cmd != NULL; i++){
        if(!strcmp((config_head + i)->cmd_type, "test")) {
            printf("%-20s", (config_head + i)->cmd);
            printf("%s%s:%s %s %s\n", "", r2, "xxx", (config_head + i)->cmd, (config_head + i)->args_readable_names);
            if(strlen((config_head + i)->help_info))
                printf("%-20s%s:%s\n", "", r3, (config_head + i)->help_info);
        }
    }

    return;
}
void args_process(void *base, cmd_config_t *cmd_configs, int argc, char *argv[])
{
    args_processor_t *processor = args_get_processor_globle_addr();

    args_init(processor, base, cmd_configs);
    /*
     *args_print_help_info(processor);
     */
    args_parse_args(processor, argc - 1, argv + 1);
}
