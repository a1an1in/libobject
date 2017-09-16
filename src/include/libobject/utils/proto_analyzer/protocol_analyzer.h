#ifndef __PROTOCOLO_ANALYZER__
#define __PROTOCOLO_ANALYZER__

#include "libobject/utils/alloc/allocator.h"
#include "libobject/utils/proto_analyzer/protocol_format_set.h"
#include "libobject/utils/data_structure/hash_list.h"

typedef struct protocol_analyzer_s{
#define MAX_PROTO_DATA_LEN 100
	protocol_format_set_t *pfs_p;
	uint8_t protocol_data[MAX_PROTO_DATA_LEN];
	uint16_t protocol_data_len;
	uint32_t protocol_num;
	struct list_head *pf_list_head_p;
	struct list_head *pa_list_head_p;
	struct proto_analyzer_admin_s *pa_admin;
	allocator_t *allocator;
	hash_map_t *hmap;

	int (*find_protocol_format)(struct protocol_analyzer_s *pa);
	int (*copy_protocol_format)(struct protocol_analyzer_s *pa); 
	int (*set_value)(char *key,char *value); 
	int (*get_value)(char *key,char *value); 
	struct list_head * (*find_key)(char *key); 
#undef MAX_PROTO_DATA_LEN
}protocol_analyzer_t;


int pa_check_protocol_num(struct protocol_analyzer_s *pa);
int pa_find_protocol_format(struct protocol_analyzer_s *pa);
int pa_copy_protocol_format(struct protocol_analyzer_s *pa);
proto_info_list_t * pa_find_key(const char *key,struct protocol_analyzer_s *pa);
int pa_set_value(const char *key,uint32_t value,struct protocol_analyzer_s *pa);
int pa_get_value(const char *key,struct protocol_analyzer_s *pa);
int pa_set_buf(const char *key,uint8_t *data,uint32_t len,struct protocol_analyzer_s *pa);
uint8_t get_bit_data(uint32_t data,uint8_t bit_pos,uint8_t len);
uint8_t set_bit_data(uint8_t *buf,uint8_t data,uint8_t bit_pos,uint8_t len);
int pa_set_variable_length_flag(struct protocol_analyzer_s *pa);
int pa_set_variable_length_value(char *key,uint32_t value, struct protocol_analyzer_s *pa);
void pa_set_protocol_byte_data(uint32_t data, uint8_t byte_pos, uint8_t bit_pos, uint16_t len,uint8_t *dp);
int pa_recompute_byte_pos(struct list_head *cur,struct protocol_analyzer_s *pa);
int pa_generate_protocol_data(struct protocol_analyzer_s *pa);
void pa_get_protocol_byte_data(proto_info_list_t *info_list, struct protocol_analyzer_s *pa);
int pa_parse_protocol_data(struct protocol_analyzer_s *pa);
struct protocol_analyzer_s *pa_create_protocol_analyzer(allocator_t *allocator);
void pa_init_protocol_analyzer(uint32_t proto_no, protocol_format_set_t *pfp, struct protocol_analyzer_s *pa);
void pa_destroy_protocol_analyzer(struct protocol_analyzer_s *pa);
int pa_reset_vlen_flag(struct protocol_analyzer_s *pa);

#endif
