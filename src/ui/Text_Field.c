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
#include <libobject/ui/Text_Field.h>
#include <libobject/ui/Sdl_Window.h>
#include <libobject/ui/Character.h>
#include <libobject/ui/Timer.h>

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
    Render *r               = ((Window *)window)->render;
    cursor_t *cursor       = &ta->cursor;
    text_line_t *line_info = NULL;
    char c                 = 0;
    Character *character;
    uint16_t cursor_line;

    cursor_line = get_row_at_cursor(component);
    line_info   = (text_line_t *)text->get_text_line_info(text, cursor_line);

    if (cursor->x > 0) {
        c                 = line_info->head[cursor->offset - 1];
        character         = (Character *)r->font->ascii[c].character;
        cursor->c         = c;
        cursor->x        -= character->width;
        cursor->width     = character->width;
        cursor->offset--;
    } else if (cursor->x == 0 && cursor->y == 0) {
        dbg_str(DBG_DETAIL, "already at head of text");
        return 0;
    }

    dbg_str(DBG_DETAIL, 
            "offset=%d, cursor_line=%d, last_line_num=%d, char =%c, "
            "x pos=%d, char_width =%d", 
            cursor->offset, cursor_line, text->last_line_num, 
            cursor->c, cursor->x, character->width);

    return 1;
}

static void move_cursor_right(Component *component) 
{
    Text_Field *ta          = (Text_Field *)component;
    Text *text             = ta->text;
    Window *window         = (Window *)ta->window;
    Render *r               = ((Window *)window)->render;
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
        character      = (Character *)r->font->ascii[c].character;

        cursor->c      = c;
        cursor->x     += cursor->width;
        cursor->width  = character->width;
        cursor->offset++;
    } else if (cursor->x + cursor->width == line_info->line_lenth)
    {
        dbg_str(DBG_VIP, "move cursor to adding char field");
        c               = ' ';
        character       = (Character *)r->font->ascii[c].character;
        cursor->c       = c;
        cursor->x      += cursor->width;
        cursor->offset++;
    } else {
        dbg_str(DBG_WARN, 
                "curor x=%d width=%d line_num=%d line_lenth=%d last_line_num=%d", 
                cursor->x, cursor->width, cursor_line, line_info->line_lenth, 
                text->last_line_num);
    }

    dbg_str(DBG_DETAIL, 
            "offset=%d, cursor_line=%d, last_line_num=%d, char =%c, "
            "x pos=%d, char_width =%d", 
            cursor->offset, cursor_line, text->last_line_num, 
            cursor->c, cursor->x, character->width);

    return ;
}

static int draw_cursor(Component *component, void *render)
{
    Text_Field *ta    = (Text_Field *)component;
    Subject *s        = (Subject *)component;
    Window *window    = (Window *)ta->window;
    Render *r          = ((Window *)window)->render;
    cursor_t *cursor  = &ta->cursor;
    color_t *ft_color = &ta->front_color;
    color_t *bg_color = &ta->background_color;
    static int count  = 0;
    Character *character;   
    char c;

#if 0
    c         = cursor->c;
    character = r->load_character(r, c, r->font, 
                                         bg_color->r, 
                                         bg_color->r, 
                                         bg_color->b, 
                                         bg_color->a); 

    r->set_color(r, ft_color->r, ft_color->r, ft_color->b, ft_color->a);
    r->fill_rect(r, cursor->x + s->x, cursor->y + s->y, character->width, character->height);
    r->write_character(r, cursor->x + s->x, cursor->y + s->y, character);
    r->present(r);

    object_destroy(character);   
#else
    r->set_color(r, ft_color->r, ft_color->r, ft_color->b, ft_color->a);
    r->draw_line(r, 
                        s->x + cursor->x, s->y + cursor->y, 
                        s->x + cursor->x, s->y + cursor->y + cursor->height);
    r->present(r);
#endif

}

static int reverse_cursor(Component *component, void *render)
{
    Text_Field *ta    = (Text_Field *)component;
    Subject *s        = (Subject *)component;
    Window *window    = (Window *)ta->window;
    Render *r          = ((Window *)window)->render;
    cursor_t *cursor  = &ta->cursor;
    color_t *ft_color = &ta->front_color;
    color_t *bg_color = &ta->background_color;
    Character *character;   
    char c;

#if 0
    c         = cursor->c;
    character = r->load_character(r, c, r->font, 
                                         ft_color->r, 
                                         ft_color->r, 
                                         ft_color->b, 
                                         ft_color->a); 

    r->set_color(r, bg_color->r, bg_color->r, bg_color->b, bg_color->a);
    r->fill_rect(r, cursor->x + s->x, cursor->y + s->y, character->width, character->height);

    r->write_character(r, cursor->x + s->x, cursor->y + s->y, character);
    r->present(r);

    object_destroy(character);   
#else
    r->set_color(r, bg_color->r, bg_color->r, bg_color->b, bg_color->a);
    r->draw_line(r, 
                        s->x + cursor->x, s->y + cursor->y, 
                        s->x + cursor->x, s->y + cursor->y + cursor->height);
    r->present(r);
#endif
}

static int draw_character(Component *component, char c, void *render)
{
    Text_Field *ta   = (Text_Field *)component;
    Render *r         = (Render *)render;
    cursor_t *cursor = &ta->cursor;
    Subject *s       = (Subject *)component;
    Character *character;

    dbg_str(DBG_DETAIL, "draw %c", c);

    character = (Character *)r->font->ascii[c].character;
    if (character->height == 0) {
        dbg_str(DBG_WARN, "text list may have problem, draw id=%d, c=%c", c, c);
        return -1;
    }

    r->write_character(r, cursor->x + s->x, cursor->y + s->y, character);
    cursor->x      += character->width;
    cursor->width   = character->width;
    cursor->height  = character->height;
    cursor->offset++;

    cursor->c = ' ';

    return 0;
}

static int erase_character(Component *component, char c, void *render)
{
    Text_Field *ta    = (Text_Field *)component;
    Subject *s        = (Subject *)component;
    Window *window    = (Window *)ta->window;
    Render *r          = ((Window *)window)->render;
    cursor_t *cursor  = &ta->cursor;
    color_t *ft_color = &ta->front_color;
    color_t *bg_color = &ta->background_color;
    Character *character;   

    character = r->load_character(r, c, r->font, 
                                         ft_color->r, 
                                         ft_color->r, 
                                         ft_color->b, 
                                         ft_color->a); 

    r->set_color(r, bg_color->r, bg_color->r, bg_color->b, bg_color->a);
    r->fill_rect(r, cursor->x + s->x, cursor->y + s->y, character->width, character->height);

    r->present(r);

    object_destroy(character);   
}

static uint32_t cursor_timer_callback(uint32_t interval, void* param )
{
    __Timer *timer = (__Timer *)param;
    Text_Field *ta  = (Text_Field *)timer->opaque;
    Window *window = (Window *)ta->window;
    Render *r       = ((Window *)window)->render;

    if ((ta->cursor_count++ % 2) == 0) {
        reverse_cursor((Component *)ta, r);
    } else {
        draw_cursor((Component *)ta, r);
    }

    window->remove_timer(window, timer);
    timer->reuse(timer);
}

static int __construct(Text_Field *ta, char *init_str)
{
    allocator_t *allocator = ((Obj *)ta)->allocator;
    cursor_t *cursor       = &ta->cursor;

    dbg_str(DBG_VIP, "ta construct");

    ta->string             = OBJECT_NEW(allocator, String, NULL);
    ta->string->assign(ta->string, "hello world!");

    ta->text               = OBJECT_NEW(allocator, Text, "");
    ta->text->content      = ta->string->value;
    ta->start_line         = 0;

    ta->front_color.r      = 0;
    ta->front_color.r      = 0;
    ta->front_color.b      = 0;
    ta->front_color.a      = 0xff;

    ta->background_color.r = 0xff;
    ta->background_color.r = 0xff;
    ta->background_color.b = 0xff;
    ta->background_color.a = 0xff;

    ta->cursor_count       = 0;
    cursor->x              = 0;
    cursor->y              = 0;

    return 0;
}

static int __deconstrcut(Text_Field *ta)
{
    dbg_str(DBG_VIP, "ta deconstruct");

    object_destroy(ta->string);
    object_destroy(ta->text);
    if (ta->timer) {
        object_destroy(ta->timer);
    }

    return 0;
}

static int __load_resources(Component *component, void *window)
{
    Render *r      = ((Window *)window)->render;
    Text_Field *ta = (Text_Field *)component;
    Text *text    = ta->text;
    Character *character;

    dbg_str(DBG_VIP, "%s load load_resources", component->name);

    ta->window          = window;

    text->last_line_num = text->write_text(text, 0, text->content, r->font) - 1;

    ta->timer           = ((Window *)window)->create_timer(window);
    ta->timer->opaque   = component;
    ta->timer->set_timer(ta->timer, 1 * 500, cursor_timer_callback); 

    character           = (Character *)r->font->ascii['i'].character;
    ta->char_min_width  = character->width;
    ta->char_height     = character->height;
    ta->cursor.height   = character->height;

    dbg_str(DBG_DETAIL, "cursor height =%d", ta->cursor.height);

    return 0;
}

static int __unload_resources(Component *component, void *window)
{
    //...........
}

static int __draw(Component *component, void *render)
{
    Text_Field *tf         = (Text_Field *)component;
    Text *text             = tf->text;
    Render *r              = (Render *)render;
    Subject *s             = (Subject *)component;
    cursor_t *cursor       = &tf->cursor;
    text_line_t *line_info = NULL;
    color_t *ft_color      = &tf->front_color;
    color_t *bg_color      = &tf->background_color;
    int i;
    char c, c_bak;
    int width_bak;

    dbg_str(DBG_DETAIL, "draw text_field");

    r->set_color(r, bg_color->r, bg_color->r, bg_color->b, bg_color->a);
    r->draw_rect(r, s->x, s->y, s->width, s->height);
    r->clear(r);
    r->set_color(r, ft_color->r, ft_color->r, ft_color->b, ft_color->a);
    r->draw_rect(r, s->x, s->y, s->width, s->height);

    cursor->x = 0; cursor->y = 0; cursor->width = 0; 

    line_info = (text_line_t *)text->get_text_line_info(text, 0);
    if (line_info == NULL) return -1;

    for (i = 0; i < line_info->tail - line_info->head + 1; i++) {
        c = line_info->head[i];
        if (cursor->x + cursor->width > s->width) break;

        draw_character(component, c, render);

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

    r->present(r);

    return 0;
}

static int __on_key_text_pressed(Component *component, char c, void *render)
{
    Render *r                 = (Render *)render;
    Text_Field *ta           = (Text_Field *)component;
    Subject *s               = (Subject *)component;
    Text *text               = ta->text;
    cursor_t *cursor         = &ta->cursor, cursor_bak;
    uint16_t cursor_line     = 0;
    Character *character;

    if (c == '\n') return 0;

    text->write_char(text, cursor_line, cursor->offset, cursor->x, c, r->font);

    character              = (Character *)r->font->ascii[c].character;
    cursor->c              = c;
    cursor->width          = character->width;

    /*draw lines disturbed by writing a char*/
    if (cursor->x + cursor->width < s->width) {
        move_cursor_right(component);
        cursor_bak             = *cursor;
        ta->draw(component, r);
        *cursor                = cursor_bak;
        draw_cursor(component, r);
    } else if (cursor->x + cursor->width == s->width) {
        /*
         *cursor->x = s->width - cursor->width;
         */
    }


    return 0;
}

static int __on_key_backspace_pressed(Component *component, void *render)
{
    Render *r             = (Render *)render;
    Text_Field *ta       = (Text_Field *)component;
    Text *text           = ta->text;
    cursor_t *cursor     = &ta->cursor, cursor_bak;
    uint16_t cursor_line = 0;
    Character *character;
    char c;
    int ret;

    dbg_str(DBG_DETAIL, "on_key_backspace_pressed");

    c                    = cursor->c;

    ret                  = move_cursor_left(component);
    if (ret < 1) { return 0;}

    text->delete_char(text, cursor_line, cursor->offset, cursor->x, r->font);

    character            = (Character *)r->font->ascii[c].character;
    cursor->c            = c;
    cursor->width        = character->width;

    cursor_bak           = *cursor;

    ta->draw(component, r);

    *cursor              = cursor_bak;
    draw_cursor(component, r);

    return 0;
}

static int __on_key_left_pressed(Component *component, void *render)
{
    Render *r         = (Render *)render;
    Text_Field *ta    = (Text_Field *)component;
    cursor_t *cursor = &ta->cursor;

    reverse_cursor(component, r);
    move_cursor_left(component);
    draw_cursor(component, r);

    ta->cursor_count = 0;
}

static int __on_key_right_pressed(Component *component, void *render)
{
    Render *r         = (Render *)render;
    Text_Field *ta    = (Text_Field *)component;
    cursor_t *cursor = &ta->cursor;

    reverse_cursor(component, r);
    move_cursor_right(component);
    draw_cursor(component, r);

    ta->cursor_count = 0;
}

static class_info_entry_t text_field_class_info[] = {
    Init_Obj___Entry(0 , Component, component),
    Init_Nfunc_Entry(1 , Text_Field, construct, __construct),
    Init_Nfunc_Entry(2 , Text_Field, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Text_Field, set, NULL),
    Init_Vfunc_Entry(4 , Text_Field, get, NULL),
    Init_Vfunc_Entry(5 , Text_Field, draw, __draw),
    Init_Vfunc_Entry(6 , Text_Field, load_resources, __load_resources),
    Init_Vfunc_Entry(7 , Text_Field, on_key_text_pressed, __on_key_text_pressed),
    Init_Vfunc_Entry(8 , Text_Field, on_key_backspace_pressed, __on_key_backspace_pressed),
    Init_Vfunc_Entry(9 , Text_Field, on_key_left_pressed, __on_key_left_pressed),
    Init_Vfunc_Entry(10, Text_Field, on_key_right_pressed, __on_key_right_pressed),
    Init_Str___Entry(11, Text_Field, name, NULL),
    Init_End___Entry(12, Text_Field),
};
REGISTER_CLASS(Text_Field, text_field_class_info);

char *gen_text_field_setting_str()
{
    cjson_t *root, *b, *c, *e, *s;
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
static int text_field()
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

#if 1
    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL, "Window dump: %s", buf);

    set_str   = gen_text_field_setting_str();
    subject   = OBJECT_NEW(allocator, Text_Field, set_str);

    object_dump(subject, "Text_Field", buf, 2048);
    dbg_str(DBG_DETAIL, "Text_Field dump: %s", buf);

    container->add_component(container, NULL, subject);
    dbg_str(DBG_DETAIL, "window container :%p", container);


    window->load_resources(window);
    window->update_window(window);
#endif

    event->poll_event(event, window);

    pause();
    object_destroy(window);

    free(set_str);
    return 1;
}
REGISTER_TEST_CMD(text_field);
