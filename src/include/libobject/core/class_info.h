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
	ENTRY_TYPE_DOUBLE_T,
	ENTRY_TYPE_SN8,
	ENTRY_TYPE_UN8,
	ENTRY_TYPE_SN16,
	ENTRY_TYPE_UN16,
	ENTRY_TYPE_SN32,
	ENTRY_TYPE_UN32,
	ENTRY_TYPE_SN64,
	ENTRY_TYPE_UN64,
	ENTRY_TYPE_FN,
	ENTRY_TYPE_DN,
	ENTRY_TYPE_STRING,
    ENTRY_TYPE_VECTOR,
	ENTRY_TYPE_OBJ,
    ENTRY_TYPE_OBJ_POINTER,
	ENTRY_TYPE_NORMAL_POINTER,
	ENTRY_TYPE_FUNC_POINTER,/*normal func pointer*/
	ENTRY_TYPE_VFUNC_POINTER,/*virtual func pointer*/
	ENTRY_TYPE_END,
	ENTRY_TYPE_MAX_TYPE,
};

typedef struct class_info_entry_s {
	uint8_t type;
	char *type_name;   //value type name
	char *value_name;
	void *value;
	int value_len;
	int offset;
    int index;
} class_info_entry_t;

#define Init_Obj_Entry(id, type_name, value_name)                                                                        \
    [id] = {ENTRY_TYPE_OBJ, #type_name, #value_name}
#define Init_Nfunc_Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_FUNC_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}
#define Init_Vfunc_Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_VFUNC_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}
#define Init_Point_Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_NORMAL_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}

#define Init_OP____Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_OBJ_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name), id}
#define Init_S8____Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_INT8_T, "", #value_name, value, sizeof(uint8_t), offset_of_class(class_name, value_name), id}
#define Init_U8____Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_UINT8_T, "", #value_name, value, sizeof(uint8_t), offset_of_class(class_name, value_name), id}
#define Init_S16___Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_INT16_T, "", #value_name, value, sizeof(uint16_t), offset_of_class(class_name, value_name), id}
#define Init_U16___Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_UINT16_T, "", #value_name, value, sizeof(uint16_t), offset_of_class(class_name, value_name), id}
#define Init_S32___Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_INT32_T, "", #value_name, value, sizeof(uint32_t), offset_of_class(class_name, value_name), id}
#define Init_U32___Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_UINT32_T, "", #value_name, value, sizeof(uint32_t), offset_of_class(class_name, value_name), id}
#define Init_Float_Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_FLOAT_T, "", #value_name, value, sizeof(float), offset_of_class(class_name, value_name), id}
#define Init_Str___Entry(id, class_name, value_name, value)                                                              \
    [id] = {ENTRY_TYPE_STRING, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name), id}
#define Init_Vec___Entry(id, class_name, value_name, value, value_type)                                                  \
    [id] = {ENTRY_TYPE_VECTOR, value_type, #value_name, value, sizeof(void *), offset_of_class(class_name, value_name), id}
#define Init_End_Entry(id, class_name)                                                                                   \
    [id] = {ENTRY_TYPE_END, #class_name, "", NULL, sizeof(class_name), 0, id}
#define Init_Obj___Entry Init_Obj_Entry
#define Init_End___Entry Init_End_Entry

#endif
