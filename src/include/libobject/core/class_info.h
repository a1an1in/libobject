#ifndef __CLASS_INFO_H__
#define __CLASS_INFO_H__

#include <libobject/basic_types.h>

enum class_info_type_e{
	UNDEFINED = 0, 
	ENTRY_TYPE_INT8_T,
	ENTRY_TYPE_UINT8_T,
	ENTRY_TYPE_INT16_T,
	ENTRY_TYPE_UINT16_T,
	ENTRY_TYPE_INT32_T,
	ENTRY_TYPE_UINT32_T,
	ENTRY_TYPE_INT64_T,
	ENTRY_TYPE_UINT64_T,
	ENTRY_TYPE_FLOAT_T,
	ENTRY_TYPE_STRING,
	ENTRY_TYPE_OBJ,
	ENTRY_TYPE_NORMAL_POINTER,
	ENTRY_TYPE_FUNC_POINTER,/*normal func pointer*/
	ENTRY_TYPE_VFUNC_POINTER,/*virtual func pointer*/
	ENTRY_TYPE_IFUNC_POINTER,/*inherit from ...*/
	ENTRY_TYPE_OBJ_POINTER,
	ENTRY_TYPE_END,
	ENTRY_TYPE_MAX_TYPE,
};

typedef struct class_info_entry_s{
	uint8_t type;
	char *type_name;
	char *value_name;
	void *value;
	int value_len;
	int offset;
}class_info_entry_t;

#define Init_Obj_Entry(id, type_name, value_name) \
    [id] = {ENTRY_TYPE_OBJ, #type_name, #value_name}
#define Init_Nfunc_Entry(id, class_name, value_name, value) \
    [id] = {ENTRY_TYPE_FUNC_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}
#define Init_Vfunc_Entry(id, class_name, value_name, value) \
    [id] = {ENTRY_TYPE_VFUNC_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}
/*
 *#define Init_Ifunc_Entry(id, class_name, value_name, value) \
 *    [id] = {ENTRY_TYPE_IFUNC_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}
 */
#define Init_End_Entry(id) \
    [id] = {ENTRY_TYPE_END}
#define Init__Obj__Entry Init_Obj_Entry
#define Init__End__Entry Init_End_Entry
#endif
