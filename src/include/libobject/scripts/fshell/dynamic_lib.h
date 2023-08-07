#ifndef __DYNAMIC_LIB_H__
#define __DYNAMIC_LIB_H__

void * dl_get_func_addr_by_name(char *name);
int dl_get_func_name_by_addr(void *addr, char *name, unsigned int name_len);

#endif