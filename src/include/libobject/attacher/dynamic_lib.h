#ifndef __DYNAMIC_LIB_H__
#define __DYNAMIC_LIB_H__

void * dl_get_func_addr_by_name(char *name);
int dl_get_func_name_by_addr(void *addr, char *name, unsigned int name_len);
void *dl_get_dynamic_lib_base_address(pid_t pid, const char *module_name);
void* dl_get_remote_function_adress(pid_t target_pid, const char* module_name, void* local_addr);

#endif