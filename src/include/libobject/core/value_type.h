#ifndef __VALUE_TYPE_H__
#define __VALUE_TYPE_H__

enum value_type_e{
	VALUE_TYPE_UNDEFINED = 0, 
	VALUE_TYPE_INT8_T,
	VALUE_TYPE_UINT8_T,
	VALUE_TYPE_INT16_T,
	VALUE_TYPE_UINT16_T,
	VALUE_TYPE_INT32_T,
	VALUE_TYPE_UINT32_T,
	VALUE_TYPE_INT64_T,
	VALUE_TYPE_UINT64_T,
	VALUE_TYPE_FLOAT_T,
	VALUE_TYPE_STRING,
	VALUE_TYPE_OBJ,
	VALUE_TYPE_NORMAL_POINTER,
	VALUE_TYPE_ALLOC_POINTER,
	VALUE_TYPE_UNKNOWN_POINTER,
	VALUE_TYPE_FUNC_POINTER,/*normal func pointer*/
	VALUE_TYPE_VFUNC_POINTER,/*virtual func pointer*/
	VALUE_TYPE_IFUNC_POINTER,/*inherit from ...*/
	VALUE_TYPE_OBJ_POINTER,
	VALUE_TYPE_END,
	VALUE_TYPE_MAX_TYPE,
};

#endif
