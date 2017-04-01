#ifndef __TEXT_AREA_H__
#define __TEXT_AREA_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/string.h>
#include <libobject/ui/component.h>
#include <libobject/ui/text.h>
#include <libobject/ui/timer.h>
#include <libobject/ui/color.h>
#include <libobject/ui/cursor.h>

typedef struct text_area_s Text_Area;

typedef struct text_area_line_info_s{
    int paragraph_num;
    int offset;
    int max_height;
}text_area_line_info_t;

struct text_area_s{
	Component component;

	int (*construct)(Text_Area *text_area,char *init_str);
	int (*deconstruct)(Text_Area *text_area);
	int (*set)(Text_Area *text_area, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
	int (*draw)(Component *component, void *graph);
	int (*load_resources)(Component *component, void *graph);
	int (*load_ascii_info)(Component *component,void *graph);

	int (*key_text_pressed)(Component *component,char c, void *graph);
	int (*key_backspace_pressed)(Component *component,void *graph);
	int (*key_up_pressed)(Component *component,void *graph);
	int (*key_down_pressed)(Component *component,void *graph);
	int (*key_left_pressed)(Component *component,void *graph);
	int (*key_right_pressed)(Component *component,void *graph);
	int (*key_pageup_pressed)(Component *component,void *graph);
	int (*key_pagedown_pressed)(Component *component,void *graph);
	int (*key_onelineup_pressed)(Component *component,void *graph);
	int (*key_onelinedown_pressed)(Component *component,void *graph);

#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN

    String *string;
	Text *text;
    cursor_t cursor;
    int to_x, to_y, to_max_height;
	int start_line;
	__Timer *timer;
    void *window;
    color_t front_color;
    color_t background_color;
    int cursor_count;
	int char_min_width;
    int char_height;
};

#endif
