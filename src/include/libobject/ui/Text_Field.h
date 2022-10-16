#ifndef __TEXT_FIELD_H__
#define __TEXT_FIELD_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/String.h>
#include <libobject/ui/Component.h>
#include <libobject/ui/Text.h>
#include <libobject/ui/Timer.h>
#include <libobject/ui/color.h>
#include <libobject/ui/Cursor.h>

typedef struct text_field_s Text_Field;

typedef struct text_field_line_info_s{
    int paragraph_num;
    int offset;
    int max_height;
}text_field_line_info_t;

struct text_field_s{
    Component component;

    int (*construct)(Text_Field *text_field,char *init_str);
    int (*deconstruct)(Text_Field *text_field);
    int (*set)(Text_Field *text_field, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*draw)(Component *component, void *graph);
    int (*load_resources)(Component *component, void *graph);
    int (*load_ascii_info)(Component *component,void *graph);

    int (*on_key_text_pressed)(Component *component,char c, void *graph);
    int (*on_key_backspace_pressed)(Component *component,void *graph);
    int (*on_key_up_pressed)(Component *component,void *graph);
    int (*on_key_down_pressed)(Component *component,void *graph);
    int (*on_key_left_pressed)(Component *component,void *graph);
    int (*on_key_right_pressed)(Component *component,void *graph);
    int (*on_key_pageup_pressed)(Component *component,void *graph);
    int (*on_key_pagedown_pressed)(Component *component,void *graph);
    int (*on_key_onelineup_pressed)(Component *component,void *graph);
    int (*on_key_onelinedown_pressed)(Component *component,void *graph);

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
