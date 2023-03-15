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
#include <libobject/ui/Label.h>
#include <libobject/ui/Sdl_Window.h>
#include <libobject/ui/Character.h>
#include <libobject/ui/Timer.h>

static int 
draw_character(Component *component, char c, void *render)
{
    Label *label     = (Label *)component;
    Render *r         = (Render *)render;
    cursor_t *cursor = &label->cursor;
    Character *character;

    character = (Character *)r->font->ascii[c].character;
    if (character->height == 0) {
        dbg_str(DBG_WARNNING, "text list may have problem, draw id=%d, c=%c", c, c);
        return -1;
    }

    dbg_str(DBG_DETAIL, "draw char c %c, at %d, %d", c, cursor->x, cursor->y);
    r->write_character(r, cursor->x, cursor->y, character);
    cursor->x      += character->width;
    cursor->width   = character->width;
    cursor->height  = character->height;
    cursor->offset++;

    cursor->c = ' ';
    r->present(r);

    return 0;
}

static int __construct(Label *label, char *init_str)
{
    allocator_t *allocator = ((Obj *)label)->allocator;
    cursor_t *cursor       = &label->cursor;

    dbg_str(DBG_IMPORTANT, "label construct");

    label->string             = OBJECT_NEW(allocator, String, NULL);
    label->string->assign(label->string, "b1234567890");

    label->front_color.r      = 0;
    label->front_color.r      = 0;
    label->front_color.b      = 0;
    label->front_color.a      = 0xff;

    label->background_color.r = 0xff;
    label->background_color.r = 0xff;
    label->background_color.b = 0xff;
    label->background_color.a = 0xff;

    cursor->x                 = 0;
    cursor->y                 = 0;
    dbg_str(DBG_SUC, "width=%d, height=%d", ((Subject *)label)->width, ((Subject *)label)->height);

    return 0;
}

static int __deconstrcut(Label *label)
{
    dbg_str(DBG_IMPORTANT, "label deconstruct");

    object_destroy(label->string);

    return 0;
}

static int __load_resources(Component *component, void *window)
{
    Render *r     = ((Window *)window)->render;
    Label *label = (Label *)component;
    Character *character;

    dbg_str(DBG_DETAIL, "%s load resources", component->name);

    label->window          = window;

    character             = (Character *)r->font->ascii['i'].character;
    label->char_min_width = character->width;
    label->char_height    = character->height;
    label->cursor.height  = character->height;

    dbg_str(DBG_DETAIL, "cursor height =%d", label->cursor.height);

    return 0;
}

static int __unload_resources(Component *component, void *window)
{
    Render *r     = ((Window *)window)->render;

    dbg_str(DBG_DETAIL, "%s unload resources", component->name);
}

static int __draw(Component *component, void *render)
{
    Label *label       = (Label *)component;
    Render *r           = (Render *)render;
    Subject *s         = (Subject *)component;
    cursor_t *cursor   = &label->cursor;
    char text_overflow = 1;
    unsigned int i, j, str_len, count = 0;
    Character *character;
    int dot_width, draw_width;
    char c;

    dbg_str(DBG_DETAIL, "%s draw component", ((Obj *)component)->name);

    if (label->text_overflow_flag == 1) {
        character = (Character *)r->font->ascii['.'].character;
        dot_width = character->width;
        draw_width = s->width - 3 * dot_width;
    } else {
        draw_width = s->width;
    }

    /*
     *r->set_color(r, 0xff, 0xff, 0xff, 0xff);
     *r->draw_rect(r, s->x, s->y, s->width, label->char_height);
     */
    r->clear(r);
    r->set_color(r, 0, 0, 0, 0xff);
    r->draw_rect(r, s->x, s->y, s->width, label->char_height);
    /*
     *r->present(r);
     */
    /*
     *r->draw_rect(r, s->x, s->y, s->width, s->height);
     */

    cursor->x = s->x; cursor->y = s->y; cursor->width = 0; 

    str_len = strlen(label->string->value);

    for (i = 0; cursor->x + cursor->width < draw_width + s->x; i++) {
        c = label->string->at(label->string, count++);
        dbg_str(DBG_DETAIL, "count =%d", count);
        draw_character(component, c, render);
        if (count == str_len){
            dbg_str(DBG_DETAIL, "count =%d", count);
            goto end;
        } 
    }

    if (    cursor->x + cursor->width >= draw_width + s->x &&
            label->text_overflow_flag == 1)
    {
        c = '.';
        draw_character(component, c, render);
        draw_character(component, c, render);
        draw_character(component, c, render);
    }

    cursor->x  = s->x;
    cursor->y += cursor->height;

end:
    dbg_str(DBG_DETAIL, "run at here");
    r->present(r);
}

static class_info_entry_t label_class_info[] = {
    Init_Obj___Entry(0 , Component, component),
    Init_Nfunc_Entry(1 , Label, construct, __construct),
    Init_Nfunc_Entry(2 , Label, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Label, set, NULL),
    Init_Vfunc_Entry(4 , Label, get, NULL),
    Init_Vfunc_Entry(5 , Label, load_resources, __load_resources),
    Init_Vfunc_Entry(6 , Label, unload_resources, __unload_resources),
    Init_Vfunc_Entry(7 , Label, draw, __draw),
    Init_Str___Entry(8 , Label, name, NULL),
    Init_U8____Entry(9 , Label, text_overflow_flag, NULL),
    Init_End___Entry(10, Label),
};
REGISTER_CLASS("Label", label_class_info);

#if 1
void gen_label_setting_str(int x, int y, int width, int height, char *name, void *out)
{
    char *set_str;

    set_str = "{\
                    \"Subject\": {\
                        \"x\":%d, \
                        \"y\":%d, \
                        \"width\":%d, \
                        \"height\":%d\
                    }, \
                    \"Container\": {\
                        \"map_type\":0\
                    }, \
                    \"Component\": {\
                        \"name\": \"%s\"\
                    }, \
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
    subject   = OBJECT_NEW(allocator, Label, buf);

    object_dump(subject, "Label", buf, 2048);
    /*
     *dbg_str(DBG_DETAIL, "Label dump: %s", buf);
     */

    return subject;
}
#endif

int label()
{
    Window *window;
    Container *container;
    Render *r;
    Subject *subject;
    __Event *event;
    allocator_t *allocator = allocator_get_default_instance();
    char *set_str;
    char buf[2048];

    set_str   = gen_window_setting_str();
    window    = OBJECT_NEW(allocator, Sdl_Window, set_str);
    r         = window->render;
    event     = window->event;
    container = (Container *)window;

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL, "Window dump: %s", buf);

    dbg_str(DBG_SUC, "test_ui_label begin alloc count =%d", allocator->alloc_count);
    subject = new_label(allocator, 0, 0, 80, 20, "label");

    object_dump(subject, "Label", buf, 2048);
    dbg_str(DBG_DETAIL, "Label dump: %s", buf);

    container->add_component(container, NULL, subject);

    dbg_str(DBG_DETAIL, "window container :%p", container);

    window->load_resources(window);
    window->update_window(window);

    event->poll_event(event, window);

    dbg_str(DBG_SUC, "test_ui_label end alloc count =%d", allocator->alloc_count);

    object_destroy(window);
}
REGISTER_TEST_CMD(label);
