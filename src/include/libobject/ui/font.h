#ifndef __FONT_H__
#define __FONT_H__

#include <libobject/obj.h>
#include <libobject/string.h>
/*
 *#include <libui/graph.h>
 */

typedef struct font_s Font;

typedef struct ascii_code_info{
	int width;
	int height;
	void *character;
} ascii_character_t;

struct font_s{
	Obj obj;

	/*normal methods*/
	int (*construct)(Font *font,char *init_str);
	int (*deconstruct)(Font *font);
	int (*set)(Font *font, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods*/
	int (*load_font)(Font *font);
	int (*unload_font)(Font *font);
	int (*load_ascii_character)(Font *font, void *graph);
	int (*unload_ascii_character)(Font *font, void *graph);
	int (*get_character_width)(Font *font, char c);
	int (*get_character_height)(Font *font, char c);

	/*attribs*/
	String *path;
	ascii_character_t ascii[128];
};

#endif
