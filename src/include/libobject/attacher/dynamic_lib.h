#ifndef __DYNAMIC_LIB_H__
#define __DYNAMIC_LIB_H__

#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <libobject/core/Interval_Tree.h>

void *dl_get_func_addr_by_name(char *name, char *lib_name);
int dl_get_func_name_by_addr(void *addr, char *name, unsigned int name_len);
void *dl_get_dynamic_lib_base_address(pid_t pid, const char *module_name);
void *dl_get_remote_function_adress(pid_t target_pid, const char* module_name, void* local_addr);
int dl_get_dynamic_lib_path(pid_t pid, const char *module_name, char *path, int len);
int dl_get_dynamic_name(pid_t pid, void *func_addr, char *module_name, int len);
int dl_parse_dynamic_table(pid_t pid, Interval_Tree *tree);
int dl_get_dynamic_lib_name_from_interval_tree(Interval_Tree *tree, void *func_addr, char *module_name, int len);

#endif