#ifndef __LABEL_H__
#define __LABEL_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/ui/component.h>
#include <libobject/ui/text.h>
#include <libobject/ui/color.h>
#include <libobject/ui/cursor.h>

typedef struct label_s Label;

struct label_s{
	Component component;

	int (*construct)(Label *label,char *init_str);
	int (*deconstruct)(Label *label);
	int (*set)(Label *label, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
	int (*draw)(Component *component, void *graph);
	int (*load_resources)(Component *component, void *graph);
	int (*unload_resources)(Component *component, void *graph);

#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    String *string;
    cursor_t cursor;
    void *window;
    color_t front_color;
    color_t background_color;
	int char_min_width;
    int char_height;
    char text_overflow_flag;
};

void *new_label(allocator_t *allocator, int x, int y, int width, int height, char *name);
void gen_label_setting_str(int x, int y, int width, int height, char *name, void *out);
#endif
