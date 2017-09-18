#include <ctype.h>
#include <math.h>
#include "libobject/utils/dbg/debug.h"
#include "libobject/utils/alloc/allocator.h"
#include "libobject/utils/proto_analyzer/protocol_format_set.h"
#include "libobject/utils/proto_analyzer/protocol_analyzer.h"
#include "pdt_drv_proto_format.h"

extern int pfs_set_proto_format_3008(protocol_format_set_t *pfp_p);
extern int pfs_set_proto_format_3019(protocol_format_set_t *pfs_p);
extern void pfs_set_pdt_drv_proto_format(protocol_format_set_t *pfs_p);
debugger_t *debugger_gp;

int test_pdt_proto_3008(protocol_format_set_t *pfs_p,
		allocator_t *allocator)
{
	int ret = 0;
	struct protocol_analyzer_s *pa;
	uint8_t proto_data2[23] ={
		0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,
		0xb,0xc,0xd,0xe,0xf,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18
	};


	/*set and parse test*/
	pa = pa_create_protocol_analyzer(allocator);
	pa_init_protocol_analyzer(0x3008, pfs_p, pa);
	/*
	 *pa = proto_analyer_admin_get_pa(pa_admin,0x3008);
	 */

	pa_set_value(PROTO_NUM,0x3008,pa);
	pa_set_value(LAB,0x1834,pa);
	pa_set_value(SLOT_INFO,0x1234,pa);
	pa_set_value(SYNC_INFO,0,pa);
	pa_set_value(TACT_INFO,2,pa);
	pa_set_variable_length_value(SLC_INFO,4,pa);
	pa_set_value(SLC_DATA,0xaabbccdd,pa);
	pa_set_value(SYNC_INDEX,0x24,pa);
	pa_set_value(ST_INFO,0x32,pa);
	pa_set_variable_length_value(DC_INFO,23,pa);
	pa_set_buf(DC_DATA,proto_data2,23,pa);

	dbg_str(DBG_IMPORTANT,"print list for each,orinal data");
	pfs_print_list_for_each(pa->pa_list_head_p);

	pa_generate_protocol_data(pa);

	dbg_str(DBG_DETAIL,"pa_parse_protocol_data");
	pa_parse_protocol_data(pa);

	dbg_str(DBG_IMPORTANT,"print list for each after parse");
	pfs_print_list_for_each(pa->pa_list_head_p);

	dbg_str(DBG_DETAIL,"pa_destroy_protocol_analyzer");
	pa_destroy_protocol_analyzer(pa);

	return ret;
}

int __pdt_drv_proto_init_3019(uint16_t proto_id,uint16_t lab,
		uint16_t slot_info,uint16_t sync_index,uint8_t st_info,
		uint8_t dc_info,uint8_t *dc_data,
		struct protocol_analyzer_s *pa)
{
	pa_set_value(PROTO_NUM,proto_id,pa);
	pa_set_value(LAB,lab,pa);
	pa_set_value(SLOT_INFO,slot_info,pa);
	pa_set_value(SYNC_INDEX,sync_index,pa);
	pa_set_value(ST_INFO,st_info,pa);
	pa_set_variable_length_value(DC_INFO,dc_info,pa);
	pa_set_buf(DC_DATA,dc_data,dc_info,pa);

	return 0;
}
int pdt_drv_proto_init_3019(uint16_t proto_id,uint16_t lab,
		uint16_t slot_info,uint16_t sync_index,uint8_t st_info,
		uint8_t buffer_count,buffer_t *buffer_p,
		struct protocol_analyzer_s *pa)
{
#define MAX_PROTO_BUFFER_LEN 1024
	uint8_t proto_buffer[MAX_PROTO_BUFFER_LEN];
#undef MAX_PROTO_BUFFER_LEN
	uint16_t len,os = 0;
	int i;

	for(i = 0; i < buffer_count; i++){
		__pdt_drv_proto_init_3019(proto_id,lab, slot_info + i*2,sync_index, st_info,
				buffer_p[i].len ,buffer_p[i].data_p,pa);
		pa_generate_protocol_data(pa);
		len = pa->protocol_data_len - 5;
		memcpy(proto_buffer + os,pa->protocol_data + 5, len);
		proto_buffer[os] = len;
		os += len;
		pa_reset_vlen_flag(pa);//the varible len may not equal the var len before,so it need be cleared
	}
	pa->protocol_data[4] = buffer_count;
	memcpy(pa->protocol_data + 5,proto_buffer,os); 
	pa->protocol_data_len = os + 5;
	dbg_buf(DBG_DETAIL,"proto_data:",pa->protocol_data,pa->protocol_data_len);
}
typedef struct pa_array_s{
	int count;
	struct protocol_analyzer_s **pa_p2;
}pa_array_t;

int get_cont_pdu_num(buffer_t *buffer_p) 
{
	return buffer_p->data_p[4];
}

int get_cont_pdu_len(buffer_t *buffer_p,uint16_t os) 
{
	return buffer_p->data_p[os];
}
pa_array_t 
pdt_drv_proto_parse_3019(protocol_format_set_t *pfs_p,
		allocator_t *allocator,buffer_t *buffer_p)
{
	struct protocol_analyzer_s *pa;
#define MAX_PROTO_BUFFER_LEN 1024
	uint8_t proto_buffer[MAX_PROTO_BUFFER_LEN];
	uint16_t proto_buffer_len;
#undef MAX_PROTO_BUFFER_LEN
	uint8_t count,i;
	uint16_t os = 5,pdu_len;


	dbg_buf(DBG_DETAIL,"parse_3019:",buffer_p->data_p,buffer_p->len);
	pa = pa_create_protocol_analyzer(allocator);
	pa_init_protocol_analyzer(0x3019, pfs_p, pa);

	count = get_cont_pdu_num(buffer_p);
	for(i = 0; i < count; i++){
		pdu_len = get_cont_pdu_len(buffer_p,os);
		memcpy(pa->protocol_data,buffer_p->data_p,5);
		dbg_str(DBG_DETAIL,"os=%d",os);
		memcpy(pa->protocol_data + 5,buffer_p->data_p + os,pdu_len);
		pa->protocol_data_len = pdu_len + 5;
		dbg_buf(DBG_DETAIL,"parse data:",pa->protocol_data,pa->protocol_data_len);
		pa_parse_protocol_data(pa);
		dbg_str(DBG_IMPORTANT,"print list for each after parse");
		pfs_print_list_for_each(pa->pa_list_head_p);
		os += pdu_len;
	}
}
int test_pdt_proto_3019(protocol_format_set_t *pfs_p,
		allocator_t *allocator)
{
	int ret = 0;
	struct protocol_analyzer_s *pa;
	uint8_t proto_data2[50] ={
		0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,
		0xb,0xc,0xd,0xe,0xf,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18
	};
	uint16_t len;
	buffer_t buf[4];
	uint8_t i;
	uint8_t size = 23;
	buffer_t data;


	/*set and parse test*/
	pa = pa_create_protocol_analyzer(allocator);
	pa_init_protocol_analyzer(0x3019, pfs_p, pa);

	
	for(i = 0; i < 4; i++){
		buf[i].data_p = allocator_mem_alloc(allocator,size);
		memcpy(buf[i].data_p,proto_data2,size);
		buf[i].len = size;
	}

	pdt_drv_proto_init_3019(0x3019,0x1234,
			0x5678,0x9012,0x34, 4,buf, pa);

	data.data_p = allocator_mem_alloc(allocator,pa->protocol_data_len);
	memcpy(data.data_p,pa->protocol_data,pa->protocol_data_len);
	data.len = pa->protocol_data_len;
	pdt_drv_proto_parse_3019(pfs_p,allocator,&data);
	/*
	 *dbg_str(DBG_DETAIL,"pa_destroy_protocol_analyzer");
	 *pa_destroy_protocol_analyzer(pa);
	 */

	return ret;

}
void pfs_set_pdt_drv_proto_format(protocol_format_set_t *pfs_p)
{
	pfs_set_proto_format_3008(pfs_p);
	pfs_set_proto_format_3019(pfs_p);
}
int test_pdt_drv_proto(protocol_format_set_t *pfs_p, allocator_t *allocator)
{
	test_pdt_proto_3008(pfs_p,allocator);
	/*
	 *test_pdt_proto_3019(pfs_p,allocator);
	 */
}
int test_pdt_proto_analyzer()
{
	protocol_format_set_t *pfs_p;
	uint32_t ret = 0;

	allocator_t *allocator = allocator_get_default_alloc();

	pfs_p = pfs_create_proto_format_set(allocator);
	init_proto_format_set(0x3000,100,pfs_p);
	pfs_set_pdt_drv_proto_format(pfs_p);

	test_pdt_drv_proto(pfs_p,allocator);

	dbg_str(DBG_DETAIL,"pfs_destroy_protocol_format_set");
	pfs_destroy_protocol_format_set(pfs_p);


	return ret;
}

