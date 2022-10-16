#ifndef __USR_MODEL_H__
#define __USR_MODEL_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/core/Number.h>
#include <libobject/database/orm/Model.h>

typedef struct Test_User_Model_s Test_User_Model;

struct Test_User_Model_s{
	Model parent;

	int (*construct)(Test_User_Model *,char *);
	int (*deconstruct)(Test_User_Model *);

	/*virtual methods reimplement*/
	int (*set)(Test_User_Model *model, char *attrib, void *value);
    void *(*get)(Test_User_Model *, char *attrib);
    char *(*to_json)(Test_User_Model *); 
    char *(*get_table_class_name)(Test_User_Model *model);
    int (*add)(Model *dst, Model *src);

    /*attribs*/
    Number *id;
    String *table_class_name;
    String *nickname;
    String *mobile;
    String *password;
    String *signature;
    String *portrait_url;
    Number *age;
    Number *count;
    String *sex;
    String *email;
    String *balance;
    String *verification_code;
    String *register_time;
};

#endif
