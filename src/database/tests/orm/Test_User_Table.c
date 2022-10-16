/**
 * @file Test_User_Table.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Test_User_Table.h"

static int __construct(Test_User_Table *table, char *init_str)
{
    Add_Field(table, "id", "BIGINT(20) UNSIGNED",
                     "NOT NULL AUTO_INCREMENT PRIMARY KEY"); 
    Add_Field(table, "nickname", "VARCHAR(255) UNIQUE", "NOT NULL"); 
    Add_Field(table, "mobile", "VARCHAR(255)", "NOT NULL"); 
    Add_Field(table, "password", "VARCHAR(255)", "NOT NULL"); 
    Add_Field(table, "age", "BIGINT(20) UNSIGNED", "NOT NULL"); 
    Add_Field(table, "signature", "VARCHAR(255)", "NULL"); 
    Add_Field(table, "portrait_url", "VARCHAR(255)", "NULL"); 
    Add_Field(table, "email", "VARCHAR(255)", "NULL"); 

    table->set_table_name(table, "test_user_table");
    table->set_model_name(table, "Test_User_Model");

    return 0;
}

static int __deconstruct(Test_User_Table *table)
{
    return 0;
}

static class_info_entry_t test_user_table_class_info[] = {
    Init_Obj___Entry(0, Table, parent),
    Init_Nfunc_Entry(1, Test_User_Table, construct, __construct),
    Init_Nfunc_Entry(2, Test_User_Table, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Test_User_Table, set, NULL),
    Init_Vfunc_Entry(4, Test_User_Table, get, NULL),
    Init_Vfunc_Entry(5, Test_User_Table, set_table_name, NULL),
    Init_Vfunc_Entry(6, Test_User_Table, get_table_name, NULL),
    Init_Vfunc_Entry(7, Test_User_Table, set_model_name, NULL),
    Init_Vfunc_Entry(8, Test_User_Table, get_model_name, NULL),
    Init_End___Entry(9, Test_User_Table),
};
REGISTER_CLASS("Test_User_Table", test_user_table_class_info);

