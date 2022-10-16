#ifndef __ORM_TABLE_H__
#define __ORM_TABLE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/List.h>
#include <libobject/core/Vector.h>
#include <libobject/database/orm/Field.h>

typedef struct Table_s Table;

struct Table_s{
    Obj parent;

    int (*construct)(Table *,char *);
    int (*deconstruct)(Table *);

    /*virtual methods reimplement*/
    int (*set)(Table *table, char *attrib, void *value);
    void *(*get)(Table *, char *attrib);
    char *(*to_json)(Table *); 
    int (*add_model)(Table *table, void *model);
    int (*remove_model)(Table *table, void **model);
    int (*peek_at_model)(Table *table, int index, void **model);
    int (*count_model)(Table *table);
    int (*add_field)(Table *table, void *value);
    Field *(*get_field)(Table *table, char *field_name);
    char *(*get_table_name)(Table *table);
    int (*set_table_name)(Table *table, char *table_name);
    char *(*get_model_name)(Table *table);
    int (*set_model_name)(Table *table, char *model_name);
    int (*reset)(Table *table);
    int (*for_each)(Table *table, int (*func)(int index, void *element));
    int (*for_each_arg)(Table *table, int (*func)(int index, void *element, void *arg), void *arg);
    int (*assign)(Table *table, char *json);
    int (*merge)(Table *table, int (*cmp)(void *element, void *key));
    int (*sort)(Table *table, enum vector_sort_type_e type, int (*func)(void *e1, void *e2));
    int (*reset_from)(Table *table, int index);
    int (*filter)(Table *table, int (*condition)(void *model, void *cond), void *cond, Table *out);
    int (*add_table)(Table *table, Table *src);
    int (*copy_table)(Table *table, Table *out);
    int (*search)(Table *table, int (*cmp)(void *element, void *key), void *key, void **out, int *index);

    /*attribs*/
    Vector *meta_info;
    Vector *models;
    String *table_name;
    String *model_name;
};

#define Add_Field(table, sql_field_name, sql_field_type, sql_constraint) \
({\
    Table *t = (Table *)table;\
    allocator_t *allocator = t->parent.allocator;\
    Field *field;\
    \
    field = object_new(allocator, "Field", NULL);\
    field->set(field, "name", sql_field_name);\
    field->set(field, "type", sql_field_type);\
    field->set(field, "constraint", sql_constraint);\
    t->add_field(t, field);\
})

#endif
