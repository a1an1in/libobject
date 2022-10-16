#ifndef __MONITOR_COMMAND_H__
#define __MONITOR_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Downloader_Command_s Downloader_Command;

struct Downloader_Command_s{
	Command parent;

	int (*construct)(Downloader_Command *command,char *init_str);
	int (*deconstruct)(Downloader_Command *command);
	int (*set)(Downloader_Command *command, char *attrib, void *value);
    void *(*get)(Downloader_Command *command, char *attrib);
    char *(*to_json)(Downloader_Command *command); 

	int (*run_action)(Downloader_Command *command);
};

#endif
