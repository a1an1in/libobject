/*
 * =====================================================================================
 *
 *       Filename:  chc_pa_admin.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/04/2015 05:18:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __PA_ADMIN_H__
#define __PA_ADMIN_H__

#include <libobject/utils/alloc/allocator.h>
#include <libobject/utils/data_structure/hash_list_struct.h>

#define PA_0X3000  "pa3000"
#define PA_0X3001  "pa3001"
#define PA_0X3002  "pa3002"
#define PA_0X3003  "pa3003"
#define PA_0X3004  "pa3004"
#define PA_0X3005  "pa3005"
#define PA_0X3006  "pa3006"
#define PA_0X3007  "pa3007"
#define PA_0X3008  "pa3008"
#define PA_0X3009  "pa3009"
#define PA_0X300A  "pa300a"
#define PA_0X300B  "pa300b"
#define PA_0X300C  "pa300c"
#define PA_0X300D  "pa300d"
#define PA_0X300E  "pa300e"
#define PA_0X300F  "pa300f"

#define PA_0X3010  "pa3010"
#define PA_0X3011  "pa3011"
#define PA_0X3012  "pa3012"
#define PA_0X3013  "pa3013"
#define PA_0X3014  "pa3014"
#define PA_0X3015  "pa3015"
#define PA_0X3016  "pa3016"
#define PA_0X3017  "pa3017"
#define PA_0X3018  "pa3018"
#define PA_0X3019  "pa3019"
#define PA_0X301A  "pa301a"
#define PA_0X301B  "pa301b"
#define PA_0X301C  "pa301c"
#define PA_0X301D  "pa301d"
#define PA_0X301E  "pa301e"
#define PA_0X301F  "pa301f"

#define PA_0X3020  "pa3020"
#define PA_0X3021  "pa3021"
#define PA_0X3022  "pa3022"
#define PA_0X3023  "pa3023"
#define PA_0X3024  "pa3024"
#define PA_0X3025  "pa3025"
#define PA_0X3026  "pa3026"
#define PA_0X3027  "pa3027"
#define PA_0X3028  "pa3028"
#define PA_0X3029  "pa3029"
#define PA_0X302A  "pa302a"
#define PA_0X302B  "pa302b"
#define PA_0X302C  "pa302c"
#define PA_0X302D  "pa302d"
#define PA_0X302E  "pa302e"
#define PA_0X302F  "pa302f"

#define PA_0X3030  "pa3030"
#define PA_0X3031  "pa3031"
#define PA_0X3032  "pa3032"
#define PA_0X3033  "pa3033"
#define PA_0X3034  "pa3034"
#define PA_0X3035  "pa3035"
#define PA_0X3036  "pa3036"
#define PA_0X3037  "pa3037"
#define PA_0X3038  "pa3038"
#define PA_0X3039  "pa3039"
#define PA_0X303A  "pa303a"
#define PA_0X303B  "pa303b"
#define PA_0X303C  "pa303c"
#define PA_0X303D  "pa303d"
#define PA_0X303E  "pa303e"
#define PA_0X303F  "pa303f"

#define PA_0X3040  "pa3040"
#define PA_0X3041  "pa3041"
#define PA_0X3042  "pa3042"
#define PA_0X3043  "pa3043"
#define PA_0X3044  "pa3044"
#define PA_0X3045  "pa3045"
#define PA_0X3046  "pa3046"
#define PA_0X3047  "pa3047"
#define PA_0X3048  "pa3048"
#define PA_0X3049  "pa3049"
#define PA_0X304A  "pa304a"
#define PA_0X304B  "pa304b"
#define PA_0X304C  "pa304c"
#define PA_0X304D  "pa304d"
#define PA_0X304E  "pa304e"
#define PA_0X304F  "pa304f"

#define PA_0X3050  "pa3050"
#define PA_0X3051  "pa3051"
#define PA_0X3052  "pa3052"
#define PA_0X3053  "pa3053"
#define PA_0X3054  "pa3054"
#define PA_0X3055  "pa3055"
#define PA_0X3056  "pa3056"
#define PA_0X3057  "pa3057"
#define PA_0X3058  "pa3058"
#define PA_0X3059  "pa3059"
#define PA_0X305A  "pa305a"
#define PA_0X305B  "pa305b"
#define PA_0X305C  "pa305c"
#define PA_0X305D  "pa305d"
#define PA_0X305E  "pa305e"
#define PA_0X305F  "pa305f"

typedef struct pa_admin_s{
	allocator_t *allocator;
	pair_t *pair;
	hash_map_t *hash_map;
	protocol_format_set_t *pfs;
}pa_admin_t;

pa_admin_t *pa_admin_create(allocator_t *allocator,
                            uint8_t key_size,uint8_t bucket_size);
void pa_admin_destroy(pa_admin_t *admin);
void pa_admin_register_protocol_analyzers(pa_admin_t *admin,
                                          protocol_format_set_t *pfs);
void pa_admin_add_protocol_analyzer(pa_admin_t *admin,void *key,struct protocol_analyzer_s *pa);
struct protocol_analyzer_s * pa_admin_get_protocol_analyzer(pa_admin_t *admin ,void *key);
#endif
