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

// 类型信息初始化宏版本v2
#define Init_Obj_Entry(id, type_name, value_name)                                                                                    \
    [id] = {ENTRY_TYPE_OBJ, #type_name, #value_name}
#define Init_Nfunc_Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_FUNC_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}
#define Init_Vfunc_Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_VFUNC_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}
#define Init_Point_Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_NORMAL_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}
#define Init_OP____Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_OBJ_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name), id}
#define Init_S8____Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_INT8_T, "", #value_name, value, sizeof(uint8_t), offset_of_class(class_name, value_name), id}
#define Init_U8____Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_UINT8_T, "", #value_name, value, sizeof(uint8_t), offset_of_class(class_name, value_name), id}
#define Init_S16___Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_INT16_T, "", #value_name, value, sizeof(uint16_t), offset_of_class(class_name, value_name), id}
#define Init_U16___Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_UINT16_T, "", #value_name, value, sizeof(uint16_t), offset_of_class(class_name, value_name), id}
#define Init_S32___Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_INT32_T, "", #value_name, value, sizeof(uint32_t), offset_of_class(class_name, value_name), id}
#define Init_U32___Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_UINT32_T, "", #value_name, value, sizeof(uint32_t), offset_of_class(class_name, value_name), id}
#define Init_Float_Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_FLOAT_T, "", #value_name, value, sizeof(float), offset_of_class(class_name, value_name), id}
#define Init_Str___Entry(id, class_name, value_name, value)                                                                          \
    [id] = {ENTRY_TYPE_STRING, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name), id}
#define Init_Vec___Entry(id, class_name, value_name, value, value_type)                                                              \
    [id] = {ENTRY_TYPE_VECTOR, value_type, #value_name, value, sizeof(void *), offset_of_class(class_name, value_name), id}
#define Init_End_Entry(id, class_name)                                                                                               \
    [id] = {ENTRY_TYPE_END, #class_name, "", NULL, sizeof(class_name), 0, id}
#define Init_Obj___Entry Init_Obj_Entry
#define Init_End___Entry Init_End_Entry


// 类型信息初始化宏版本v3
#define INIT_FUNC_ENTRY(type, class_name, value_name, value)                                                                         \
    {type, "", #value_name, value, sizeof(void *), offset_of_class(class_name, value_name)}
#define INIT_VFUNC_ENTRY(class_name, value_name, value)                                                                              \
    INIT_FUNC_ENTRY(ENTRY_TYPE_VFUNC_POINTER, class_name, value_name, value)
#define INIT_NFUNC_ENTRY(class_name, value_name, value)                                                                              \
    INIT_FUNC_ENTRY(ENTRY_TYPE_FUNC_POINTER, class_name, value_name, value)
#define INIT_END___ENTRY(class_name)                                                                                                 \
    {ENTRY_TYPE_END, #class_name, "", NULL, sizeof(class_name), 0}

#define CLASS_OBJ___ENTRY(type_name, value_name) {ENTRY_TYPE_OBJ, #type_name, #value_name}
#define CLASS_NFUNC_ENTRY(value_name, value)  INIT_FUNC_ENTRY(ENTRY_TYPE_FUNC_POINTER, __current_class_t, value_name, value)
#define CLASS_VFUNC_ENTRY(value_name, value)  INIT_FUNC_ENTRY(ENTRY_TYPE_VFUNC_POINTER, __current_class_t, value_name, value)

// 修改后的 DEFINE_CLASS：在 __VA_ARGS__ 后自动附加结束条目
#define DEFINE_CLASS(cls, ...)                                                                                                       \
	typedef cls __current_class_t;                                                                                                   \
	static const char * const current_class_name = #cls;                                                                             \
	static class_info_entry_t cls##_class_info[] = {                                                                                 \
			__VA_ARGS__,                                                                                                             \
			INIT_END___ENTRY(cls)                                                                                                    \
	};                                                                                                                               \
	REGISTER_CLASS(cls, cls##_class_info)

#define DEFINE_COMMAND(cls, ...)                                                                                                     \
	typedef cls __current_class_t;                                                                                                   \
	static const char * const current_class_name = #cls;                                                                             \
	static class_info_entry_t cls##_class_info[] = {                                                                                 \
			__VA_ARGS__,                                                                                                             \
			INIT_END___ENTRY(cls)                                                                                                    \
	};                                                                                                                               \
	REGISTER_APP_CMD(cls, cls##_class_info)

#endif
