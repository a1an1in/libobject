#include "libobject/core/utils/dbg/debug.h"
#include <libobject/core/utils/args/cmd_args.h>

typedef struct config_list_s{
}config_list_t;

typedef struct base_s{
    config_list_t config;
    args_processor_t *p;
}base_t;

static int args_process_port(void *base,int argc,char **argv)
{
    dbg_str(DBG_DETAIL,"args_process_port:%s",argv[0]);
    return 1;
}
static int args_process_ipaddr(void *base,int argc,char **argv)
{
    dbg_str(DBG_DETAIL,"args_process_ipaddr:%s",argv[0]);
    return 1;
}
static int args_process_help(void *base,int argc,char **argv)
{
    args_print_help_info(args_get_processor_globle_addr());
    return 0;
}

cmd_config_t cmds[]={
    {"port", args_process_port,1, "port", "NN(number)","udp port,using to send/rcv msg with neighbor"},
    {"ip", args_process_ipaddr,1, "ip addr", "xx.xx.xx.xx","ip addr,using to send/rcv msg with neighbor"},
    {"help", args_process_help,0, "help", "help","help info"},
    {NULL,NULL,0,NULL,NULL,NULL},
};

void test_args(int argc, char *argv[])
{
    base_t *base = NULL;

    args_process((void *)base,cmds,argc, argv);

    
}
