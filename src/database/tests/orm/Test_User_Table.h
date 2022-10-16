#ifndef __USR_TABLE_H__
#define __USR_TABLE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/database/orm/Table.h>

typedef struct Test_User_Table_s Test_User_Table;

struct Test_User_Table_s{
	Table parent;

	int (*construct)(Test_User_Table *,char *);
	int (*deconstruct)(Test_User_Table *);

	/*virtual methods reimplement*/
	int (*set)(Test_User_Table *table, char *attrib, void *value);
    void *(*get)(Test_User_Table *, char *attrib);
    char *(*to_json)(Test_User_Table *); 
    int (*add_field)(Test_User_Table *table, char *field_name, void *value);
    int (*get_table_name)(Test_User_Table *table);
    int (*set_table_name)(Test_User_Table *table, char *table_name);
    char *(*get_model_name)(Test_User_Table *table);
    int (*set_model_name)(Test_User_Table *table, char *model_name);

    /*attribs*/
};

#endif
