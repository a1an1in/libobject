/**
 * @file text_field.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-21
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <libobject/ui/text_field.h>
#include <libobject/ui/sdl_window.h>
#include <libobject/ui/character.h>
#include <libobject/ui/timer.h>

static char get_row_at_cursor(Component *component) 
{
    Text_Field *ta    = (Text_Field *)component;
    cursor_t *cursor = &ta->cursor;
    uint16_t row;

    row = ta->start_line + cursor->y / cursor->height;

    return row;
}

static int move_cursor_left(Component *component) 
{
    Text_Field *ta          = (Text_Field *)component;
    Text *text             = ta->text;
    Window *window         = (Window *)ta->window;
    Graph *g               = ((Window *)window)->graph;
    cursor_t *cursor       = &ta->cursor;
    text_line_t *line_info = NULL;
    char c                 = 0;
    Character *character;
    uint16_t cursor_line;

    cursor_line = get_row_at_cursor(component);
    line_info   = (text_line_t *)text->get_text_line_info(text, cursor_line);

    if (cursor->x > 0) {
        c                 = line_info->head[cursor->offset - 1];
        character         = (Character *)g->font->ascii[c].character;
        cursor->c         = c;
        cursor->x        -= character->width;
        cursor->width     = character->width;
        cursor->offset--;
    } else if (cursor->x == 0 && cursor->y == 0) {
        dbg_str(DBG_DETAIL,"already at head of text");
        return 0;
    }

    dbg_str(DBG_DETAIL,
            "offset=%d,cursor_line=%d,last_line_num=%d,char =%c,"
            "x pos=%d,char_width =%d",
            cursor->offset,cursor_line, text->last_line_num, 
            cursor->c, cursor->x, character->width);

    return 1;
}

static void move_cursor_right(Component *component) 
{
    Text_Field *ta          = (Text_Field *)component;
    Text *text             = ta->text;
    Window *window         = (Window *)ta->window;
    Graph *g               = ((Window *)window)->graph;
    cursor_t *cursor       = &ta->cursor;
    int width              = ((Subject *)component)->width;
    text_line_t *line_info = NULL;
    Character *character;
    uint16_t cursor_line;
    char c = 0;

    cursor_line = get_row_at_cursor(component);
    line_info   = (text_line_t *)text->get_text_line_info(text, cursor_line);

    if (cursor->x + cursor->width < line_info->line_lenth) {
        c              = line_info->head[cursor->offset + 1];
        character      = (Character *)g->font->ascii[c].character;

        cursor->c      = c;
        cursor->x     += cursor->width;
        cursor->width  = character->width;
        cursor->offset++;
    } else if (cursor->x + cursor->width == line_info->line_lenth)
    {
        dbg_str(DBG_SUC,"move cursor to adding char field");
        c               = ' ';
        character       = (Character *)g->font->ascii[c].character;
        cursor->c       = c;
        cursor->x      += cursor->width;
        cursor->offset++;
    } else {
        dbg_str(DBG_WARNNING,
                "curor x=%d width=%d line_num=%d line_lenth=%d last_line_num=%d",
                cursor->x, cursor->width, cursor_line, line_info->line_lenth, 
                text->last_line_num);
    }

    dbg_str(DBG_DETAIL,
            "offset=%d,cursor_line=%d,last_line_num=%d,char =%c,"
            "x pos=%d,char_width =%d",
            cursor->offset,cursor_line, text->last_line_num, 
            cursor->c, cursor->x, character->width);

    return ;
}

static int draw_cursor(Component *component,void *graph)
{
    Text_Field *ta    = (Text_Field *)component;
    Subject *s        = (Subject *)component;
    Window *window    = (Window *)ta->window;
    Graph *g          = ((Window *)window)->graph;
    cursor_t *cursor  = &ta->cursor;
    color_t *ft_color = &ta->front_color;
    color_t *bg_color = &ta->background_color;
    static int count  = 0;
    Character *character;   
    char c;

#if 0
    c         = cursor->c;
    character = g->render_load_character(g,c,g->font,
                                         bg_color->r,
                                         bg_color->g, 
                                         bg_color->b, 
                                         bg_color->a); 

    g->render_set_color(g,ft_color->r,ft_color->g,ft_color->b,ft_color->a);
    g->render_fill_rect(g,cursor->x + s->x, cursor->y + s->y, character->width, character->height);
    g->render_write_character(g,cursor->x + s->x,cursor->y + s->y,character);
    g->render_present(g);

    object_destroy(character);   
#else
    g->render_set_color(g,ft_color->r,ft_color->g,ft_color->b,ft_color->a);
    g->render_draw_line(g,
                        s->x + cursor->x, s->y + cursor->y,
                        s->x + cursor->x, s->y + cursor->y + cursor->height);
    g->render_present(g);
#endif

}

static int reverse_cursor(Component *component,void *graph)
{
    Text_Field *ta    = (Text_Field *)component;
    Subject *s        = (Subject *)component;
    Window *window    = (Window *)ta->window;
    Graph *g          = ((Window *)window)->graph;
    cursor_t *cursor  = &ta->cursor;
    color_t *ft_color = &ta->front_color;
    color_t *bg_color = &ta->background_color;
    Character *character;   
    char c;

#if 0
    c         = cursor->c;
    character = g->render_load_character(g,c,g->font,
                                         ft_color->r,
                                         ft_color->g,
                                         ft_color->b,
                                         ft_color->a); 

    g->render_set_color(g,bg_color->r, bg_color->g, bg_color->b, bg_color->a);
    g->render_fill_rect(g,cursor->x + s->x, cursor->y + s->y, character->width, character->height);

    g->render_write_character(g,cursor->x + s->x,cursor->y + s->y,character);
    g->render_present(g);

    object_destroy(character);   
#else
    g->render_set_color(g,bg_color->r, bg_color->g, bg_color->b, bg_color->a);
    g->render_draw_line(g,
                        s->x + cursor->x, s->y + cursor->y,
                        s->x + cursor->x, s->y + cursor->y + cursor->height);
    g->render_present(g);
#endif
}

static int draw_character(Component *component,char c, void *graph)
{
    Text_Field *ta   = (Text_Field *)component;
    Graph *g         = (Graph *)graph;
    cursor_t *cursor = &ta->cursor;
    Subject *s       = (Subject *)component;
    Character *character;

    character = (Character *)g->font->ascii[c].character;
    if (character->height == 0) {
        dbg_str(DBG_WARNNING,"text list may have problem, draw id=%d, c=%c", c,c);
        return -1;
    }

    g->render_write_character(g,cursor->x + s->x, cursor->y + s->y,character);
    cursor->x      += character->width;
    cursor->width   = character->width;
    cursor->height  = character->height;
    cursor->offset++;

    cursor->c = ' ';

    return 0;
}

static int erase_character(Component *component,char c, void *graph)
{
    Text_Field *ta    = (Text_Field *)component;
    Subject *s        = (Subject *)component;
    Window *window    = (Window *)ta->window;
    Graph *g          = ((Window *)window)->graph;
    cursor_t *cursor  = &ta->cursor;
    color_t *ft_color = &ta->front_color;
    color_t *bg_color = &ta->background_color;
    Character *character;   

    character = g->render_load_character(g,c,g->font,
                                         ft_color->r,
                                         ft_color->g,
                                         ft_color->b,
                                         ft_color->a); 

    g->render_set_color(g,bg_color->r, bg_color->g, bg_color->b, bg_color->a);
    g->render_fill_rect(g,cursor->x + s->x, cursor->y + s->y, character->width, character->height);

    g->render_present(g);

    object_destroy(character);   
}

static uint32_t cursor_timer_callback(uint32_t interval, void* param )
{
    __Timer *timer = (__Timer *)param;
    Text_Field *ta  = (Text_Field *)timer->opaque;
    Window *window = (Window *)ta->window;
    Graph *g       = ((Window *)window)->graph;

    if ((ta->cursor_count++ % 2) == 0) {
        reverse_cursor((Component *)ta,g);
    } else {
        draw_cursor((Component *)ta,g);
    }

    window->remove_timer(window, timer);
    timer->reuse(timer);
}

static int __construct(Text_Field *ta,char *init_str)
{
    allocator_t *allocator = ((Obj *)ta)->allocator;
    cursor_t *cursor       = &ta->cursor;

    dbg_str(DBG_SUC,"ta construct");

    ta->string             = OBJECT_NEW(allocator, String, NULL);
    ta->string->assign(ta->string,"hello world!");

    ta->text               = OBJECT_NEW(allocator, Text,"");
    ta->text->content      = ta->string->value;
    ta->start_line         = 0;

    ta->front_color.r      = 0;
    ta->front_color.g      = 0;
    ta->front_color.b      = 0;
    ta->front_color.a      = 0xff;

    ta->background_color.r = 0xff;
    ta->background_color.g = 0xff;
    ta->background_color.b = 0xff;
    ta->background_color.a = 0xff;

    ta->cursor_count       = 0;
    cursor->x              = 0;
    cursor->y              = 0;

    return 0;
}

static int __deconstrcut(Text_Field *ta)
{
    dbg_str(DBG_SUC,"ta deconstruct");

    object_destroy(ta->string);
    object_destroy(ta->text);
    if (ta->timer) {
        object_destroy(ta->timer);
    }

    return 0;
}

static int __set(Text_Field *ta, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        ta->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        ta->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        ta->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        ta->deconstruct = value;
    }
    /*vitual methods*/
    else if (strcmp(attrib, "draw") == 0) {
        ta->draw = value;
    } else if (strcmp(attrib, "load_resources") == 0) {
        ta->load_resources = value;
    } else if (strcmp(attrib, "key_text_pressed") == 0) {
        ta->key_text_pressed = value;
    } else if (strcmp(attrib, "key_backspace_pressed") == 0) {
        ta->key_backspace_pressed = value;
    } else if (strcmp(attrib, "key_left_pressed") == 0) {
        ta->key_left_pressed = value;
    } else if (strcmp(attrib, "key_right_pressed") == 0) {
        ta->key_right_pressed = value;
    }
    /*attribs*/
    else if (strcmp(attrib, "name") == 0) {
        strncpy(ta->name,value,strlen(value));
    } else {
        dbg_str(DBG_DETAIL,"ta set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Text_Field *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(DBG_WARNNING,"text_field get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __load_resources(Component *component,void *window)
{
    Graph *g      = ((Window *)window)->graph;
    Text_Field *ta = (Text_Field *)component;
    Text *text    = ta->text;
    Character *character;

    dbg_str(DBG_SUC,"%s load load_resources",component->name);

    ta->window          = window;

    text->last_line_num = text->write_text(text,0, text->content, g->font) - 1;

    ta->timer           = ((Window *)window)->create_timer(window);
    ta->timer->opaque   = component;
    ta->timer->set_timer(ta->timer, 1 * 500, cursor_timer_callback); 

    character           = (Character *)g->font->ascii['i'].character;
    ta->char_min_width  = character->width;
    ta->char_height     = character->height;
    ta->cursor.height   = character->height;

    dbg_str(DBG_DETAIL,"cursor height =%d",ta->cursor.height);

    return 0;
}

static int __unload_resources(Component *component,void *window)
{
    //...........
}

static int __draw(Component *component, void *graph)
{
    Text_Field *ta          = (Text_Field *)component;
    Text *text             = ta->text;
    Graph *g               = (Graph *)graph;
    Subject *s             = (Subject *)component;
    cursor_t *cursor       = &ta->cursor;
    text_line_t *line_info = NULL;
    int i;
    char c, c_bak;
    int width_bak;

    g->render_set_color(g,0xff,0xff,0xff,0xff);
    g->render_draw_rect(g,s->x,s->y,s->width,s->height);
    g->render_clear(g);
    g->render_set_color(g,0x0,0x0,0x0,0xff);
    g->render_draw_rect(g,s->x,s->y,s->width,s->height);

    cursor->x = 0; cursor->y = 0; cursor->width = 0; 

    line_info = (text_line_t *)text->get_text_line_info(text,0);
    if (line_info == NULL) return -1;

    for (i = 0; i < line_info->tail - line_info->head + 1; i++) {
        c = line_info->head[i];
        if (cursor->x + cursor->width > s->width) break;

        draw_character(component,c, graph);

        if (i == 0) { /*bak cursor info of first char of the screen*/
            c_bak     = line_info->head[0];
            width_bak = cursor->width;
        }
    }

    cursor->x      = 0;
    cursor->y      = 0;
    cursor->offset = 0;
    cursor->c      = c_bak;
    cursor->width  = width_bak;

    g->render_present(g);

    return 0;
}

static int __key_text_pressed(Component *component,char c, void *graph)
{
    Graph *g                 = (Graph *)graph;
    Text_Field *ta           = (Text_Field *)component;
    Subject *s               = (Subject *)component;
    Text *text               = ta->text;
    cursor_t *cursor         = &ta->cursor, cursor_bak;
    uint16_t cursor_line     = 0;
    Character *character;

    if (c == '\n') return 0;

    text->write_char(text,cursor_line, cursor->offset, cursor->x, c, g->font);

    character              = (Character *)g->font->ascii[c].character;
    cursor->c              = c;
    cursor->width          = character->width;

    /*draw lines disturbed by writing a char*/
    if (cursor->x + cursor->width < s->width) {
        move_cursor_right(component);
        cursor_bak             = *cursor;
        ta->draw(component, g);
        *cursor                = cursor_bak;
        draw_cursor(component,g);
    } else if (cursor->x + cursor->width == s->width) {
        /*
         *cursor->x = s->width - cursor->width;
         */
    }


    return 0;
}

static int __key_backspace_pressed(Component *component,void *graph)
{
    Graph *g             = (Graph *)graph;
    Text_Field *ta       = (Text_Field *)component;
    Text *text           = ta->text;
    cursor_t *cursor     = &ta->cursor, cursor_bak;
    uint16_t cursor_line = 0;
    Character *character;
    char c;
    int ret;

    dbg_str(DBG_DETAIL,"key_backspace_pressed");

    c                    = cursor->c;

    ret                  = move_cursor_left(component);
    if (ret < 1) { return 0;}

    text->delete_char(text,cursor_line, cursor->offset, cursor->x, g->font);

    character            = (Character *)g->font->ascii[c].character;
    cursor->c            = c;
    cursor->width        = character->width;

    cursor_bak           = *cursor;

    ta->draw(component, g);

    *cursor              = cursor_bak;
    draw_cursor(component,g);

    return 0;
}

static int __key_left_pressed(Component *component,void *graph)
{
    Graph *g         = (Graph *)graph;
    Text_Field *ta    = (Text_Field *)component;
    cursor_t *cursor = &ta->cursor;

    reverse_cursor(component,g);
    move_cursor_left(component);
    draw_cursor(component,g);

    ta->cursor_count = 0;
}

static int __key_right_pressed(Component *component,void *graph)
{
    Graph *g         = (Graph *)graph;
    Text_Field *ta    = (Text_Field *)component;
    cursor_t *cursor = &ta->cursor;

    reverse_cursor(component,g);
    move_cursor_right(component);
    draw_cursor(component,g);

    ta->cursor_count = 0;
}

static class_info_entry_t text_field_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Component","component",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","draw",__draw,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","load_resources",__load_resources,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","key_text_pressed",__key_text_pressed,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","key_backspace_pressed",__key_backspace_pressed,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","key_left_pressed",__key_left_pressed,sizeof(void *)},
    [10] = {ENTRY_TYPE_FUNC_POINTER,"","key_right_pressed",__key_right_pressed,sizeof(void *)},
    [11] = {ENTRY_TYPE_STRING,"char","name",NULL,0},
    [12] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Text_Field",text_field_class_info);

char *gen_text_field_setting_str()
{
    cjson_t *root,*b, *c, *e, *s;
    char *set_str;

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Text_Field", b = cjson_create_object());{
            cjson_add_item_to_object(root, "Component", c = cjson_create_object());{
                cjson_add_item_to_object(root, "Container", e = cjson_create_object());{
                    cjson_add_item_to_object(e, "Subject", s = cjson_create_object());{
                        cjson_add_number_to_object(s, "x", 10);
                        cjson_add_number_to_object(s, "y", 20);
                        cjson_add_number_to_object(s, "width", 100);
                        cjson_add_number_to_object(s, "height", 20);
                    }
                }
                cjson_add_string_to_object(c, "name", "text_field");
            }
        }
    }
    set_str = cjson_print(root);

    return set_str;
}
void test_ui_text_field()
{
    Window *window;
    Container *container;
    Graph *g;
    Subject *subject;
    __Event *event;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    char buf[2048];

    set_str   = gen_window_setting_str();
    window    = OBJECT_NEW(allocator, Sdl_Window,set_str);
    g         = window->graph;
    event     = window->event;
    container = (Container *)window;

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL,"Window dump: %s",buf);

    set_str   = gen_text_field_setting_str();
    subject   = OBJECT_NEW(allocator, Text_Field,set_str);

    object_dump(subject, "Text_Field", buf, 2048);
    dbg_str(DBG_DETAIL,"Text_Field dump: %s",buf);

    container->add_component(container, NULL, subject);
    dbg_str(DBG_DETAIL,"window container :%p",container);

    window->load_resources(window);
    window->update_window(window);

    event->poll_event(event, window);

    object_destroy(window);

    free(set_str);
}
