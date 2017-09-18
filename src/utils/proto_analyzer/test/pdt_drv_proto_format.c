/*
 * =====================================================================================
 *
 *       Filename:  pdt_proto_format.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/10/2015 19:35:28 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan 
 *        Company:  vigor
 *
 * =====================================================================================
 */
#include "libobject/utils/dbg/debug.h"
#include "libobject/utils/proto_analyzer/protocol_format_set.h"
#include "pdt_drv_proto_format.h"

int pfs_set_proto_format_3008(protocol_format_set_t *pfs_p)
{
	struct list_head *hl_head;
	int llist_num;

	hl_head = pfs_create_head_list(pfs_p->allocator);
	pfs_init_head_list(hl_head);

	/*
	 *PFS_SET_PROTO_INFO(name,byte_pos,bit_pos,len,len_unit,vlenth_index,hl_head) 
	 */
	PFS_SET_PROTO_INFO( PROTO_NUM,    "0", "0", "2", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( LAB,          "2", "0", "2", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( SLOT_INFO,    "4", "0", "2", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( SYNC_INFO,    "6", "0", "2", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( TACT_INFO,    "8", "0", "1", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( SLC_INFO, 	  "9", "0", "1", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( SLC_DATA,     "10","0", "0", "8",  SLC_INFO, hl_head);
	PFS_SET_PROTO_INFO( SYNC_INDEX,   "10","0", "1", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( ST_INFO,      "11","0", "1", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( DC_INFO,      "12","0", "1", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( DC_DATA,      "13","0", "0", "8",  DC_INFO,  hl_head);

	llist_num =0x3008 - pfs_p->proto_base_addr;
	pfs_add_proto_link_list(llist_num,
			hl_head,//struct list_head *new_proto_llist,
			pfs_p);//struct list_head **list_head)
}
int pfs_set_proto_format_3019(protocol_format_set_t *pfs_p)
{
	struct list_head *hl_head;
	int llist_num;

	hl_head = pfs_create_head_list(pfs_p->allocator);
	pfs_init_head_list(hl_head);

	/*
	 *PFS_SET_PROTO_INFO(name,byte_pos,bit_pos,len,len_unit,vlenth_index,hl_head) 
	 */
	PFS_SET_PROTO_INFO( PROTO_NUM,    "0", "0", "2", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( LAB,          "2", "0", "2", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( LEN,          "5", "0", "1", "8",  NULL,     hl_head);

	PFS_SET_PROTO_INFO( SLOT_INFO,    "6", "0", "2", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( SYNC_INDEX,   "8", "0", "1", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( ST_INFO,      "9","0", "1", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( DC_INFO,      "10","0", "1", "8",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( DC_DATA,      "11","0", "0", "8",  DC_INFO,  hl_head);

	llist_num =0x3019 - pfs_p->proto_base_addr;
	pfs_add_proto_link_list(llist_num,
			hl_head,//struct list_head *new_proto_llist,
			pfs_p);//struct list_head **list_head)

}
