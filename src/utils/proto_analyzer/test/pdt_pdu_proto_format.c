/*
 * =====================================================================================
 *
 *       Filename:  pdu_proto_format.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/18/2015 04:20:56 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "libobject/utils/proto_analyzer/protocol_format_set.h"
#include "libobject/utils/dbg/debug.h"

int pfs_set_pdu_format_ackd(protocol_format_set_t *pfs_p)
{
	struct list_head *hl_head;
	int llist_num;

	hl_head = pfs_create_head_list(pfs_p->allocator);
	pfs_init_head_list(hl_head);
	/*
	 *PFS_SET_PROTO_INFO( "LB",    "0",  "7", "1", "1",  NULL,     hl_head);
	 *PFS_SET_PROTO_INFO( "PF",    "0",  "6", "1", "1",  NULL,     hl_head);
	 *PFS_SET_PROTO_INFO( "CSBKO", "0",  "0", "6", "1",  NULL,     hl_head);
	 *PFS_SET_PROTO_INFO( "FID",   "1",  "0", "8", "1",  NULL,     hl_head);
	 *PFS_SET_PROTO_INFO( "RI",    "2",  "1", "7", "1",  NULL,     hl_head);
	 *PFS_SET_PROTO_INFO( "ARC",   "3",  "1", "8", "1",  NULL,     hl_head);
	 *PFS_SET_PROTO_INFO( "RSVD",  "3", "0", "1", "1",  NULL,     hl_head);
	 *PFS_SET_PROTO_INFO( "TADDR", "6", "0", "24", "1",  NULL,     hl_head);
	 *PFS_SET_PROTO_INFO( "SADDR", "9", "0", "24", "1",  NULL,     hl_head);
	 */

	PFS_SET_PROTO_INFO( "LB",    "0",  "7", "1", "1",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( "PF",    "0",  "6", "1", "1",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( "CSBKO", "0",  "5", "6", "1",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( "FID",   "1",  "7", "8", "1",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( "RI",    "2",  "7", "7", "1",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( "ARC",   "2",  "0", "8", "1",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( "RSVD",  "3", "0", "1",  "1",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( "TADDR", "4", "7", "24", "1",  NULL,     hl_head);
	PFS_SET_PROTO_INFO( "SADDR", "7", "7", "24", "1",  NULL,     hl_head);

	llist_num =0x8 - pfs_p->proto_base_addr;
	dbg_str(DBG_DETAIL,"llist_num=%d",llist_num);
	pfs_add_proto_link_list(llist_num,
			hl_head,//struct list_head *new_proto_llist,
			pfs_p);//struct list_head **list_head)
}
