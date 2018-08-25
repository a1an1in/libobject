#ifndef __CMD_ARGS_H__
#define __CMD_ARGS_H__

#include <libobject/core/utils/data_structure/link_list.h>

typedef struct cmd_config_s{
	char *cmd;
	int (*fn)(void *base, int argc, char **argv);
	uint8_t args_count;
	char *cmd_type;
	char *args_readable_names;
	char *help_info;
}cmd_config_t;

typedef struct args_processor_s{
	cmd_config_t *cmd_config;
	void *base;
}args_processor_t;

args_processor_t * args_init(args_processor_t *p,void *base,cmd_config_t *c);
cmd_config_t * args_find_entry(args_processor_t *p,char *cmd);
int args_parse_args(args_processor_t *p,int argc, char *argv[]);
void args_print_help_info(args_processor_t *p);
void args_process(void *base,cmd_config_t *cmd_configs,int argc, char *argv[]);
args_processor_t *args_get_processor_globle_addr();
void test_args(int argc, char *argv[]);
#endif
