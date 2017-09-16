#ifndef __PROTOCOL_FORMAT_SET__
#define __PROTOCOL_FORMAT_SET__

#include "list.h"
#include "my_types.h"
#include "libobject/utils/alloc/allocator.h"

typedef struct proto_head_list_s{
	/*
	 *pthread_rwlock_t head_lock;//delete lock,beacause proto analyzing not have cocurrent acktion ,
	 *								and kernel lock and user lock are not same,which is hard to unify;
	 */
	uint8_t list_count;
	struct list_head list_head;
	allocator_t *allocator;
}proto_head_list_t;

typedef void (*free_mem_fptr)(void *mem_used);

typedef struct buffer_s{
	uint16_t len;
	uint8_t *data_p;
}buffer_t;

typedef struct proto_info_list_s{
#define MAX_NAME_LEN 20
#define MAX_DATA_LEN 20
	char name[MAX_NAME_LEN];
	char vlenth_index[MAX_NAME_LEN];
	uint8_t vlenth_flag:1;
	uint8_t vlenth_flag_bak:1;
	uint8_t vlenth_value_flag:1;
	uint8_t vlenth_value_assigned_flag:1;
	uint8_t byte_pos;
	uint8_t byte_pos_bak;
	uint8_t bit_pos;
	uint8_t bit_pos_bak;
	uint16_t len;//len value
	uint8_t len_unit;//len unit
	uint32_t bit_len;//len of bit
	//char data[MAX_DATA_LEN];
	uint32_t data;
	/*
	 *uint8_t *data_ptr;
	 */
	buffer_t buf;
	free_mem_fptr free_mem;
	struct list_head list_head;
	void *mem_used;
#undef MAX_NAME_LEN
#undef MAX_DATA_LEN
}proto_info_list_t;

typedef struct protocol_format_set_s{
#define MAX_NAME_LEN 20
	struct list_head **list_head_p2;
	uint32_t proto_total_num;
	uint32_t proto_max_num;
	char *pfs_name[MAX_NAME_LEN];
	uint32_t proto_base_addr;
	allocator_t *allocator;
#undef MAX_NAME_LEN
}protocol_format_set_t;

enum{
	PROTO_0x3000 = 0x3000,	
	PROTO_0x3001,	
	PROTO_0x3002,	
	PROTO_0x3003,	
	PROTO_0x3004,	
	PROTO_0x3005,	
	PROTO_0x3006,	
	PROTO_0x3007,	
	PROTO_0x3008 ,	
	PROTO_0x3009 ,	
	PROTO_0x300A,	
	PROTO_0x300B,	
};

int hexstr2num(const char* const_str);
int str2num(const char *const_str);
void free_proto_info_list_mem(void *mem_used);
void init_proto_info_list(proto_info_list_t *info_list);
void print_info_list(proto_info_list_t *info_list);
struct list_head *pfs_create_head_list(allocator_t *allocator);
void pfs_init_head_list(struct list_head *hl_head);
void pfs_release_head_list(struct list_head *hl_head);
void pfs_add_list(struct list_head *new_head,struct list_head *hl_head);
void pfs_del_list(struct list_head *del_head,struct list_head *hl_head);
void pfs_print_list_for_each(struct list_head *hl_head);
void pfs_del_list_for_each(struct list_head *hl_head);
void pfs_print_proto_link_lists(protocol_format_set_t *pfp);
void pfs_del_proto_link_list(protocol_format_set_t *pfp,uint32_t llist_num);
void set_proto_info_attribs(char *name,void *attr_value,proto_info_list_t *info_list);
int pfs_add_proto_link_list(uint32_t llist_num, struct list_head *new_proto_llist, protocol_format_set_t *pfs_ptr);
int pfs_save_rootelement_attributes(char *name, char *value,protocol_format_set_t *pfs_p);
int pfs_create_protocol_format_database(char *filename,protocol_format_set_t *pfs_p);
protocol_format_set_t *pfs_create_proto_format_set(allocator_t *allocator);
void pfs_destroy_protocol_format_set(protocol_format_set_t *pfp);

proto_info_list_t *pfs_set_proto_info(char *name,char *name_value,
				   					  char *byte_pos,char *byte_pos_value,
				   					  char *bit_pos,char *bit_pos_value,
				   					  char *len,char *len_value,
				   					  char *len_unit,char *len_unit_value,
				   					  char *vlenth_index,char *vlenth_index_value,
				   					  struct list_head *hl_head);

void init_proto_format_set(int proto_base_addr,
						   int max_proto_num,
						   protocol_format_set_t *pfs_p);

#define PFS_SET_PROTO_INFO(name,byte_pos,bit_pos,len,len_unit,vlenth_index,hl_head) \
	pfs_set_proto_info("name",name,\
			"byte_pos",byte_pos,\
			"bit_pos",bit_pos,\
			"len",len,\
			"len_unit",len_unit,\
			"vlenth_index",vlenth_index,\
			hl_head)

#endif
