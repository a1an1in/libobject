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

// 类型信息初始化v1版本，如下直接定义。
// static class_info_entry_t obj_class_info[] = {
//     [0] = {ENTRY_TYPE_NORMAL_POINTER, "allocator_t", "allocator", NULL, sizeof(void *), offset_of_class(Obj, allocator)}, 
//     [1] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *), offset_of_class(Obj, construct)}, 
//     [2] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *), offset_of_class(Obj, deconstruct)}, 
//     [3] = {ENTRY_TYPE_VFUNC_POINTER, "", "set", __set, sizeof(void *), offset_of_class(Obj, set)}, 
//     [4] = {ENTRY_TYPE_VFUNC_POINTER, "", "get", __get, sizeof(void *), offset_of_class(Obj, get)}, 
//     [5] = {ENTRY_TYPE_VFUNC_POINTER, "", "to_json", __to_json, sizeof(void *), offset_of_class(Obj, to_json)}, 
//     [6] = {ENTRY_TYPE_VFUNC_POINTER, "", "reset", __reset, sizeof(void *), offset_of_class(Obj, reset)}, 
//     [7] = {ENTRY_TYPE_VFUNC_POINTER, "", "assign", __assign, sizeof(void *), offset_of_class(Obj, assign)}, 
//     [8] = {ENTRY_TYPE_END}, 
// };
// REGISTER_CLASS(Obj, obj_class_info);

// 类型信息初始化v2版本
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

// 类型信息初始化v3版本
#define Class_Obj___Entry(type_name, value_name) {ENTRY_TYPE_OBJ, #type_name, #value_name}
#define Class_End___Entry(class_name) {ENTRY_TYPE_END, #class_name, "", NULL, sizeof(class_name), 0}
#define Class_VFunc_Entry(value_name, value) {ENTRY_TYPE_VFUNC_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(__current_class_t, value_name)}
#define Class_Ptr___Entry(value_name, value) {ENTRY_TYPE_NORMAL_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(__current_class_t, value_name)}
#define Class_OPtr__Entry(value_name, value) {ENTRY_TYPE_OBJ_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(__current_class_t, value_name)}
#define Class_NFunc_Entry(value_name, value) {ENTRY_TYPE_FUNC_POINTER, "", #value_name, value, sizeof(void *), offset_of_class(__current_class_t, value_name)}
#define Class_S8____Entry(value_name, value) {ENTRY_TYPE_INT8_T, "", #value_name, value, sizeof(int8_t), offset_of_class(__current_class_t, value_name)}
#define Class_U8____Entry(value_name, value) {ENTRY_TYPE_UINT8_T, "", #value_name, value, sizeof(uint8_t), offset_of_class(__current_class_t, value_name)}
#define Class_S16___Entry(value_name, value) {ENTRY_TYPE_INT16_T, "", #value_name, value, sizeof(int16_t), offset_of_class(__current_class_t, value_name)}
#define Class_U16___Entry(value_name, value) {ENTRY_TYPE_UINT16_T, "", #value_name, value, sizeof(uint16_t), offset_of_class(__current_class_t, value_name)}
#define Class_S32___Entry(value_name, value) {ENTRY_TYPE_INT32_T, "", #value_name, value, sizeof(int32_t), offset_of_class(__current_class_t, value_name)}
#define Class_U32___Entry(value_name, value) {ENTRY_TYPE_UINT32_T, "", #value_name, value, sizeof(uint32_t), offset_of_class(__current_class_t, value_name)}
#define Class_S64___Entry(value_name, value) {ENTRY_TYPE_INT64_T, "", #value_name, value, sizeof(int64_t), offset_of_class(__current_class_t, value_name)}
#define Class_U64___Entry(value_name, value) {ENTRY_TYPE_UINT64_T, "", #value_name, value, sizeof(uint64_t), offset_of_class(__current_class_t, value_name)}
#define Class_Float_Entry(value_name, value) {ENTRY_TYPE_FLOAT_T, "", #value_name, value, sizeof(float), offset_of_class(__current_class_t, value_name)}
#define Class_Dbl___Entry(value_name, value) {ENTRY_TYPE_DOUBLE_T, "", #value_name, value, sizeof(double), offset_of_class(__current_class_t, value_name)}
#define Class_Str___Entry(value_name, value) {ENTRY_TYPE_STRING, "", #value_name, value, sizeof(void *), offset_of_class(__current_class_t, value_name)}
#define Class_Vec___Entry(value_name, value, value_type) {ENTRY_TYPE_VECTOR, value_type, #value_name, value, sizeof(void *), offset_of_class(__current_class_t, value_name)}

// 修改后的 DEFINE_CLASS：在 __VA_ARGS__ 后自动附加结束条目
#define DEFINE_CLASS(cls, ...)                                                                                                       \
	typedef cls __current_class_t;                                                                                                   \
	static const char * const current_class_name = #cls;                                                                             \
	static class_info_entry_t cls##_class_info[] = {                                                                                 \
			__VA_ARGS__,                                                                                                             \
			Class_End___Entry(cls)                                                                                                   \
	};                                                                                                                               \
	REGISTER_CLASS(cls, cls##_class_info)

#define DEFINE_COMMAND(cls, ...)                                                                                                     \
	typedef cls __current_class_t;                                                                                                   \
	static const char * const current_class_name = #cls;                                                                             \
	static class_info_entry_t cls##_class_info[] = {                                                                                 \
			__VA_ARGS__,                                                                                                             \
			Class_End___Entry(cls)                                                                                                   \
	};                                                                                                                               \
	REGISTER_APP_CMD(cls, cls##_class_info)

#endif
