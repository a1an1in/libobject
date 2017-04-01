#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <libobject/obj.h>
#include <libobject/string.h>
#include <libobject/ui/graph.h>

typedef struct character_s Character;
struct character_s{
	Obj obj;

	/*normal methods*/
	int (*construct)(Character *character,char *init_str);
	int (*deconstruct)(Character *character);
	int (*set)(Character *character, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
	int (*assign)(Character *character, uint32_t code);

	/*virtual methods*/
	int (*load_character)(Character *character,void *graph);

	/*attribs*/
	uint32_t code;
	int width;
	int height;
};

#endif
