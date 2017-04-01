/**
 * @file enemy.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-21
 */
#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/ui/subject.h>

typedef struct enemy_s Enemy;

struct enemy_s{
	Subject subject;

	int (*construct)(Enemy *enemy,char *init_str);
	int (*deconstruct)(Enemy *enemy);
	int (*set)(Enemy *enemy, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
	int (*move)(Enemy *enemy);
#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN

};

static int __construct(Enemy *enemy,char *init_str)
{
	dbg_str(OBJ_SUC,"enemy construct, enemy addr:%p",enemy);

	return 0;
}

static int __deconstrcut(Enemy *enemy)
{
	dbg_str(OBJ_SUC,"enemy deconstruct,enemy addr:%p",enemy);

	return 0;
}

static int __move(Enemy *enemy)
{
	dbg_str(OBJ_SUC,"enemy move");
}

static int __set(Enemy *enemy, char *attrib, void *value)
{
	if(strcmp(attrib, "set") == 0) {
		enemy->set = value;
    } else if(strcmp(attrib, "get") == 0) {
		enemy->get = value;
	} else if(strcmp(attrib, "construct") == 0) {
		enemy->construct = value;
	} else if(strcmp(attrib, "deconstruct") == 0) {
		enemy->deconstruct = value;
	} else if(strcmp(attrib, "move") == 0) {
		enemy->move = value;
	} else if(strcmp(attrib, "name") == 0) {
        strncpy(enemy->name,value,strlen(value));
	} else {
		dbg_str(OBJ_DETAIL,"enemy set, not support %s setting",attrib);
	}

	return 0;
}

static void *__get(Enemy *obj, char *attrib)
{
    if(strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(OBJ_WARNNING,"enemy get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t enemy_class_info[] = {
	[0] = {ENTRY_TYPE_OBJ,"Subject","subject",NULL,sizeof(void *)},
	[1] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
	[2] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
	[3] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
	[4] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
	[5] = {ENTRY_TYPE_FUNC_POINTER,"","move",__move,sizeof(void *)},
	[6] = {ENTRY_TYPE_STRING,"char","name",NULL,0},
	[7] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Enemy",enemy_class_info);

void test_obj_enemy()
{
    Subject *subject;
	allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Enemy", e = cjson_create_object());{
            cjson_add_item_to_object(e, "Subject", s = cjson_create_object());{
                cjson_add_number_to_object(s, "x", 1);
                cjson_add_number_to_object(s, "y", 25);
                cjson_add_number_to_object(s, "width", 5);
                cjson_add_number_to_object(s, "height", 125);
            }
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str = cjson_print(root);

    /*
     *subject = OBJECT_ALLOC(allocator,Enemy);
     *object_set(subject, "Enemy", set_str);
     *dbg_str(OBJ_DETAIL,"x=%d y=%d width=%d height=%d",subject->x,subject->y,subject->width,subject->height);
     */

    subject = OBJECT_NEW(allocator, Enemy,set_str);

    /*
     *dbg_str(OBJ_DETAIL,"x=%d y=%d width=%d height=%d",subject->x,subject->y,subject->width,subject->height);
     *dbg_str(OBJ_DETAIL,"enemy nane=%s",((Enemy *)subject)->name);
     *subject->move(subject);
     */

    object_dump(subject, "Enemy", buf, 2048);
    dbg_str(OBJ_DETAIL,"Enemy dump: %s",buf);

    free(set_str);

}


