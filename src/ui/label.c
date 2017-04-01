/**
 * @file label.c
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
#include <libobject/ui/label.h>
#include <libobject/ui/sdl_window.h>
#include <libobject/ui/character.h>
#include <libobject/ui/timer.h>

static int 
draw_character(Component *component,char c, void *graph)
{
    Label *label     = (Label *)component;
    Graph *g         = (Graph *)graph;
    cursor_t *cursor = &label->cursor;
    Character *character;

    character = (Character *)g->font->ascii[c].character;
    if (character->height == 0) {
        dbg_str(DBG_WARNNING,"text list may have problem, draw id=%d, c=%c", c,c);
        return -1;
    }

    g->render_write_character(g,cursor->x, cursor->y,character);
    cursor->x      += character->width;
    cursor->width   = character->width;
    cursor->height  = character->height;
    cursor->offset++;

    cursor->c = ' ';

    return 0;
}

static int __construct(Label *label,char *init_str)
{
    allocator_t *allocator = ((Obj *)label)->allocator;
    cursor_t *cursor       = &label->cursor;

    dbg_str(DBG_IMPORTANT,"label construct");

    label->string             = OBJECT_NEW(allocator, String, NULL);
    label->string->assign(label->string,"b1234567890");

    label->front_color.r      = 0;
    label->front_color.g      = 0;
    label->front_color.b      = 0;
    label->front_color.a      = 0xff;

    label->background_color.r = 0xff;
    label->background_color.g = 0xff;
    label->background_color.b = 0xff;
    label->background_color.a = 0xff;

    cursor->x                 = 0;
    cursor->y                 = 0;
    dbg_str(DBG_SUC, "width=%d, height=%d", ((Subject *)label)->width, ((Subject *)label)->height);

    return 0;
}

static int __deconstrcut(Label *label)
{
    dbg_str(DBG_IMPORTANT,"label deconstruct");

    object_destroy(label->string);

    return 0;
}

static int __set(Label *label, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        label->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        label->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        label->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        label->deconstruct = value;
    }
    /*vitual methods*/
    else if (strcmp(attrib, "draw") == 0) {
        label->draw = value;
    } else if (strcmp(attrib, "load_resources") == 0) {
        label->load_resources = value;
    } else if (strcmp(attrib, "unload_resources") == 0) {
        label->unload_resources = value;
    }
    /*attribs*/
    else if (strcmp(attrib, "name") == 0) {
        strncpy(label->name,value,strlen(value));
    } else if (strcmp(attrib, "text_overflow_flag") == 0) {
        label->text_overflow_flag = *((uint8_t *)value);
    } else {
        dbg_str(DBG_DETAIL,"label set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Label *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else if (strcmp(attrib, "text_overflow_flag") == 0) {
        return &obj->text_overflow_flag;
    } else {
        dbg_str(DBG_WARNNING,"label get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __load_resources(Component *component,void *window)
{
    Graph *g     = ((Window *)window)->graph;
    Label *label = (Label *)component;
    Character *character;

    dbg_str(DBG_DETAIL,"%s load resources",component->name);

    label->window          = window;

    character             = (Character *)g->font->ascii['i'].character;
    label->char_min_width = character->width;
    label->char_height    = character->height;
    label->cursor.height  = character->height;

    dbg_str(DBG_DETAIL,"cursor height =%d",label->cursor.height);

    return 0;
}

static int __unload_resources(Component *component,void *window)
{
    Graph *g     = ((Window *)window)->graph;

    dbg_str(DBG_DETAIL,"%s unload resources",component->name);
}

static int __draw(Component *component, void *graph)
{
    Label *label       = (Label *)component;
    Graph *g           = (Graph *)graph;
    Subject *s         = (Subject *)component;
    cursor_t *cursor   = &label->cursor;
    char text_overflow = 1;
    unsigned int i, j, str_len, count = 0;
    Character *character;
    int dot_width, draw_width;
    char c;

    dbg_str(DBG_DETAIL,"%s draw component", ((Obj *)component)->name);

    if (label->text_overflow_flag == 1) {
        character = (Character *)g->font->ascii['.'].character;
        dot_width = character->width;
        draw_width = s->width - 3 * dot_width;
    } else {
        draw_width = s->width;
    }

    /*
     *g->render_set_color(g,0xff,0xff,0xff,0xff);
     *g->render_draw_rect(g,s->x,s->y,s->width,label->char_height);
     */
    /*
     *g->render_clear(g);
     */
    g->render_set_color(g,0,0,0,0xff);
    g->render_draw_rect(g,s->x,s->y,s->width,label->char_height);
    /*
     *g->render_draw_rect(g,s->x,s->y,s->width,s->height);
     */

    cursor->x = s->x; cursor->y = s->y; cursor->width = 0; 

    str_len = strlen(label->string->value);

    for (i = 0; cursor->x + cursor->width < draw_width + s->x; i++) {
        c = label->string->at(label->string, count++);
        draw_character(component,c, graph);
        if (count == str_len){
            dbg_str(DBG_DETAIL,"count =%d",count);
            goto end;
        } 
    }

    if (    cursor->x + cursor->width >= draw_width + s->x &&
            label->text_overflow_flag == 1)
    {
        c = '.';
        draw_character(component,c, graph);
        draw_character(component,c, graph);
        draw_character(component,c, graph);
    }

    cursor->x  = s->x;
    cursor->y += cursor->height;

end:
    g->render_present(g);
}

static class_info_entry_t label_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Component","component",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","load_resources",__load_resources,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","unload_resources",__unload_resources,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","draw",__draw,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_STRING,"char","name",NULL,0},
    [9 ] = {ENTRY_TYPE_INT8_T,"char","text_overflow_flag",NULL,0},
    [10] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Label",label_class_info);

#if 1
void gen_label_setting_str(int x, int y, int width, int height, char *name, void *out)
{
    char *set_str;

    set_str = "{\
                    \"Subject\": {\
                        \"x\":%d,\
                        \"y\":%d,\
                        \"width\":%d,\
                        \"height\":%d\
                    },\
                    \"Container\": {\
                        \"map_type\":0\
                    },\
                    \"Component\": {\
                        \"name\": \"%s\"\
                    },\
                    \"Label\": {\
                        \"text_overflow_flag\": 0\
                    }\
                }";

    sprintf(out, set_str, x, y, width, height, name);

    return ;
}

void *new_label(allocator_t *allocator, int x, int y, int width, int height, char *name)
{
    Subject *subject;
    char *set_str;
    char buf[2048];

    gen_label_setting_str(x, y, width, height, name, (void *)buf);
    subject   = OBJECT_NEW(allocator, Label,buf);

    object_dump(subject, "Label", buf, 2048);
    /*
     *dbg_str(DBG_DETAIL,"Label dump: %s",buf);
     */

    return subject;
}
#endif

void test_ui_label()
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

    dbg_str(DBG_SUC, "test_ui_label begin alloc count =%d",allocator->alloc_count);
    subject = new_label(allocator,0, 0, 80, 20, "label");

    object_dump(subject, "Label", buf, 2048);
    dbg_str(DBG_DETAIL,"Label dump: %s",buf);

    container->add_component(container, NULL, subject);

    dbg_str(DBG_DETAIL,"window container :%p",container);

    window->load_resources(window);
    window->update_window(window);

    event->poll_event(event, window);

    dbg_str(DBG_SUC, "test_ui_label end alloc count =%d",allocator->alloc_count);

    object_destroy(window);
}
