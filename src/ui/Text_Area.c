/**
 * @file text_area.c
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
#include <libobject/ui/text_area.h>
#include <libobject/ui/sdl_window.h>
#include <libobject/ui/character.h>
#include <libobject/ui/timer.h>

extern void print_line_info(Iterator *iter);
extern char *global_text;

static char get_row_at_cursor(Component *component) 
{
    Text_Area *ta    = (Text_Area *)component;
    cursor_t *cursor = &ta->cursor;
    uint16_t row;

    row = ta->start_line + cursor->y / cursor->height;

    return row;
}

static int move_cursor_left(Component *component) 
{
    Text_Area *ta          = (Text_Area *)component;
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
    } else if (cursor->x == 0 && cursor->y > 0) {
        line_info         = (text_line_t *)text->get_text_line_info(text, cursor_line - 1);
        c                 = *line_info->tail;
        character         = (Character *)r->font->ascii[c].character;
        cursor->c         = c;
        cursor->x         = line_info->line_lenth - character->width;
        cursor->y        -= character->height;
        cursor->width     = character->width;
        cursor->offset    = line_info->tail - line_info->head;
    } else if (cursor->x == 0 && cursor->y == 0 && ta->start_line > 0) {
        dbg_str(DBG_DETAIL, "at head of screan");
        return 0;
    } else if (cursor->x == 0 && cursor->y == 0 && ta->start_line == 0) {
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
    Text_Area *ta          = (Text_Area *)component;
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
    } 
    /*when insert char at end of line, may be the case:
     * 1.cursor->x == line_info->line_lenth
     * 2.cursor->x + cursor->width > line_info->line_lenth;
     *if just move cursor in text, only has one case:
     * 1.cursor->x + cursor->width == line_info->line_lenth
     * */
    else if (cursor->x + cursor->width > line_info->line_lenth &&
             cursor_line < text->last_line_num)
    {
        line_info       = (text_line_t *)text->get_text_line_info(text, cursor_line + 1);

        c               = line_info->head[0];
        character       = (Character *)r->font->ascii[c].character;
        cursor->c       = c;
        cursor->offset  = 0;
        cursor->x       = 0;
        cursor->y      += character->height;
        cursor->width   = character->width;

        c               = line_info->head[1];
        character       = (Character *)r->font->ascii[c].character;
        cursor->c       = c;
        cursor->x      += cursor->width;
        cursor->width   = character->width;
        cursor->offset++;
        return;
    }
    else if ((  cursor->x + cursor->width == line_info->line_lenth ||
                cursor->x == line_info->line_lenth) &&
                cursor_line < text->last_line_num)
    {
        line_info       = (text_line_t *)text->get_text_line_info(text, cursor_line + 1);
        c               = line_info->head[0];
        character       = (Character *)r->font->ascii[c].character;
        cursor->c       = c;
        cursor->offset  = 0;
        cursor->x       = 0;
        cursor->y      += character->height;
        cursor->width   = character->width;
        return;
    } else if (cursor->x + cursor->width == line_info->line_lenth &&
               cursor_line == text->last_line_num)
    {
        dbg_str(DBG_SUC, "move cursor to adding char field");
        c               = ' ';
        character       = (Character *)r->font->ascii[c].character;
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
            "offset=%d, cursor_line=%d, last_line_num=%d, char =%c, "
            "x pos=%d, char_width =%d", 
            cursor->offset, cursor_line, text->last_line_num, 
            cursor->c, cursor->x, character->width);

    return ;
}

static void move_cursor_up(Component *component) 
{
    Text_Area *ta        = (Text_Area *)component;
    Text *text             = ta->text;
    Window *window         = (Window *)ta->window;
    Render *r               = ((Window *)window)->render;
    cursor_t *cursor       = &ta->cursor;
    int component_width    = ((Subject *)component)->width;
    int head_offset, i     = 0, width_sum = 0;
    text_line_t *line_info = NULL;
    Character *character;
    uint16_t cursor_line;
    char c = 0;

    cursor_line = get_row_at_cursor(component);
    if (ta->start_line == cursor_line && cursor_line == 0) {
        return;
    }

    line_info = (text_line_t *)text->get_text_line_info(text, cursor_line - 1);
    /*
     *dbg_str(DBG_DETAIL, "cursor line=%d, data:%s", cursor_line, line_info->string->value);
     */

    if (cursor->y >= cursor->height) {
        cursor->y -= cursor->height;

        /*case:upper line at cursor pos has character*/
        if (line_info->line_lenth > cursor->x) {
            /*modulate proper cursor pos*/
            for ( i = 0; width_sum < line_info->line_lenth; i++) { 
                c         = line_info->head[i];
                character = (Character *)r->font->ascii[c].character;

                if (    cursor->x >= width_sum &&
                        cursor->x <  character->width  + width_sum)
                {
                    cursor->x      = width_sum;
                    cursor->c      = c;
                    cursor->offset = i;
                    cursor->width  = character->width;
                    cursor->height = character->height;

                    break;
                } else {
                    width_sum += character->width;
                }
            }
        }
        /*case:upper line at cursor pos doesn' has character*/ 
        else {
            /*last char is '\n'*/
            if ((c = *line_info->tail) == '\n') { 
                cursor->offset = line_info->tail - line_info->head;
            } else {
                cursor->offset = line_info->tail - line_info->head - 1;
            }
            character      = (Character *)r->font->ascii[c].character;
            cursor->x      = line_info->line_lenth - character->width;
            cursor->c      = c;
            cursor->width  = character->width;
            cursor->height = character->height;

        }

        dbg_str(DBG_DETAIL, 
                "offset=%d, cursor_line=%d, last_line_num=%d, char =%c, "
                "x pos=%d, char_width =%d", 
                cursor->offset, cursor_line - 1, text->last_line_num, 
                cursor->c, cursor->x, character->width);

    } else { //line down
        cursor_t bak  = *cursor;
        bak.y        += bak.height;
        ta->on_key_onelinedown_pressed(component, r);
        *cursor       = bak;
        move_cursor_up(component);
        return ;
    }

    return ;
}

static void move_cursor_down(Component *component) 
{
    Text_Area *ta          = (Text_Area *)component;
    Text *text             = ta->text;
    Window *window         = (Window *)ta->window;
    Render *r               = ((Window *)window)->render;
    cursor_t *cursor       = &ta->cursor;
    int component_height   = ((Subject *)component)->height;
    int component_width    = ((Subject *)component)->width;
    int head_offset, i     = 0, width_sum = 0;
    char c                 = 0;
    text_line_t *line_info = NULL;
    Character *character;
    uint16_t cursor_line,  last_line_num;

    cursor_line   = get_row_at_cursor(component);
    last_line_num = text->get_line_count(text);

    if (cursor_line == last_line_num) {
        dbg_str(DBG_DETAIL, "move_cursor_down, already last line, "
                "cursor_line =%d, last_line_num =%d", cursor_line, last_line_num);
        return;
    }

    line_info = (text_line_t *)text->get_text_line_info(text, cursor_line + 1);

    if (cursor->y + cursor->height * 2 < component_height) {/*not last line*/
        if (line_info->line_lenth > cursor->x) {/*next line has char at cursor pos*/ 
            cursor->y   += cursor->height;
            for (i = 0; ; i++) {
                c         = line_info->head[i];
                character = (Character *)r->font->ascii[c].character;
                if (    cursor->x >= width_sum &&
                        cursor->x < width_sum + character->width)
                {
                    cursor->x      = width_sum;
                    cursor->c      = c;
                    cursor->offset = i;
                    cursor->width  = character->width;
                    cursor->height = character->height;

                    break;
                } else {
                    width_sum += character->width;
                }
            }
        }  else {
            if ((c = *line_info->tail) == '\n') {
                cursor->offset = line_info->tail - line_info->head;
            } else {
                cursor->offset = line_info->tail - line_info->head - 1;
            }
            cursor->y      += cursor->height;
            character       = (Character *)r->font->ascii[c].character;
            cursor->x       = line_info->line_lenth - character->width;
            cursor->c       = c;
            cursor->width   = character->width;
            cursor->height  = character->height;

        }

        dbg_str(DBG_DETAIL, 
                "offset=%d, cursor_line=%d, last_line_num=%d, char =%c, "
                "x pos=%d, char_width =%d", 
                cursor->offset, cursor_line + 1, text->last_line_num, 
                cursor->c, cursor->x, character->width);
    }  else { /*last line*/
        cursor_t bak  = *cursor;
        bak.y        -= bak.height;
        ta->on_key_onelineup_pressed(component, r);
        *cursor       = bak;
        move_cursor_down(component);
    }


    return ;
}

static int draw_cursor(Component *component, void *render)
{
    Text_Area *ta     = (Text_Area *)component;
    Window *window    = (Window *)ta->window;
    Render *r          = ((Window *)window)->render;
    cursor_t *cursor  = &ta->cursor;
    color_t *ft_color = &ta->front_color;
    color_t *bg_color = &ta->background_color;
    static int count  = 0;
    Character *character;   
    char c;

    c         = cursor->c;
    character = r->load_character(r, c, r->font, 
                                         bg_color->r, 
                                         bg_color->r, 
                                         bg_color->b, 
                                         bg_color->a); 
    /*
     *dbg_str(DBG_DETAIL, "draw character c :%c", c);
     */
    r->set_color(r, ft_color->r, ft_color->r, ft_color->b, ft_color->a);
    r->fill_rect(r, cursor->x, cursor->y, character->width, character->height);
    r->write_character(r, cursor->x, cursor->y, character);
    r->present(r);

    object_destroy(character);   
}

static int reverse_cursor(Component *component, void *render)
{
    Text_Area *ta     = (Text_Area *)component;
    Window *window    = (Window *)ta->window;
    Render *r          = ((Window *)window)->render;
    cursor_t *cursor  = &ta->cursor;
    color_t *ft_color = &ta->front_color;
    color_t *bg_color = &ta->background_color;
    Character *character;   
    char c;

    c         = cursor->c;
    character = r->load_character(r, c, r->font, 
                                         ft_color->r, 
                                         ft_color->r, 
                                         ft_color->b, 
                                         ft_color->a); 

    r->set_color(r, bg_color->r, bg_color->r, bg_color->b, bg_color->a);
    r->fill_rect(r, cursor->x, cursor->y, character->width, character->height);

    r->write_character(r, cursor->x, cursor->y, character);
    r->present(r);

    object_destroy(character);   
}

static int draw_character(Component *component, char c, void *render)
{
    Text_Area *ta    = (Text_Area *)component;
    Render *r         = (Render *)render;
    cursor_t *cursor = &ta->cursor;
    Character *character;

    character = (Character *)r->font->ascii[c].character;
    if (character->height == 0) {
        dbg_str(DBG_WARNNING, "text list may have problem, draw id=%d, c=%c", c, c);
        return -1;
    }

    r->write_character(r, cursor->x, cursor->y, character);
    cursor->x      += character->width;
    cursor->width   = character->width;
    cursor->height  = character->height;
    cursor->offset++;

    cursor->c = ' ';

    return 0;
}

static int erase_character(Component *component, char c, void *render)
{
    Text_Area *ta     = (Text_Area *)component;
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
    r->fill_rect(r, cursor->x, cursor->y, character->width, character->height);

    r->present(r);

    object_destroy(character);   
}

static int draw_n_lines_of_text(Component *component, int from, int to, void *render)
{
    Text_Area *ta          = (Text_Area *)component;
    Text *text             = ta->text;
    Render *r               = (Render *)render;
    Subject *s             = (Subject *)component;
    cursor_t *cursor       = &ta->cursor;
    text_line_t *line_info = NULL;
    int i, j;
    char c;

    cursor->x     = 0;
    cursor->y     = from * ta->char_height;
    cursor->width = 0;

    for (   j = ta->start_line + from;
            cursor->y < ((Subject *)component)->height && j <= ta->start_line + to;
            j++) 
    {
        line_info = (text_line_t *)text->get_text_line_info(text, j);
        if (line_info == NULL) break;

        /*
         *dbg_str(DBG_DETAIL, 
         *        "draw line=%d, len=%d, cursor->x =%d, cursor->y =%d, str=%s", 
         *        j, 
         *        line_info->tail - line_info->head, 
         *        cursor->x, cursor->y, 
         *        line_info->head);
         */

        for (i = 0; i < line_info->tail - line_info->head + 1; i++) {
            c = line_info->head[i];
            draw_character(component, c, render);
        }
        cursor->x       = 0;
        cursor->y      += cursor->height;
    }

    r->present(r);
}

static int erase_n_lines_of_text(Component *component, int from, int to, void *render)
{
    Text_Area *ta     = (Text_Area *)component;
    Text *text        = ta->text;
    Render *r          = (Render *)render;
    Subject *s        = (Subject *)component;
    color_t *bg_color = &ta->background_color;
    int height        = ta->char_height;

    r->set_color(r, bg_color->r, bg_color->r, bg_color->b, bg_color->a);
    r->fill_rect(r, 0, from * height, s->width, (to - from + 1) * height);
}

static uint32_t cursor_timer_callback(uint32_t interval, void* param )
{
    __Timer *timer = (__Timer *)param;
    Text_Area *ta  = (Text_Area *)timer->opaque;
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

static int __construct(Text_Area *ta, char *init_str)
{
    allocator_t *allocator = ((Obj *)ta)->allocator;
    cursor_t *cursor       = &ta->cursor;

    dbg_str(DBG_SUC, "ta construct");

    ta->string             = OBJECT_NEW(allocator, String, NULL);
    ta->string->assign(ta->string, global_text);

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

static int __deconstrcut(Text_Area *ta)
{
    dbg_str(DBG_SUC, "ta deconstruct");

    object_destroy(ta->string);
    object_destroy(ta->text);
    if (ta->timer) {
        object_destroy(ta->timer);
    }

    return 0;
}

static int __set(Text_Area *ta, char *attrib, void *value)
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
    } else if (strcmp(attrib, "on_key_text_pressed") == 0) {
        ta->on_key_text_pressed = value;
    } else if (strcmp(attrib, "on_key_backspace_pressed") == 0) {
        ta->on_key_backspace_pressed = value;
    } else if (strcmp(attrib, "on_key_up_pressed") == 0) {
        ta->on_key_up_pressed = value;
    } else if (strcmp(attrib, "on_key_down_pressed") == 0) {
        ta->on_key_down_pressed = value;
    } else if (strcmp(attrib, "on_key_left_pressed") == 0) {
        ta->on_key_left_pressed = value;
    } else if (strcmp(attrib, "on_key_right_pressed") == 0) {
        ta->on_key_right_pressed = value;
    } else if (strcmp(attrib, "on_key_pageup_pressed") == 0) {
        ta->on_key_pageup_pressed = value;
    } else if (strcmp(attrib, "on_key_pagedown_pressed") == 0) {
        ta->on_key_pagedown_pressed = value;
    } else if (strcmp(attrib, "on_key_onelineup_pressed") == 0) {
        ta->on_key_onelineup_pressed = value;
    } else if (strcmp(attrib, "on_key_onelinedown_pressed") == 0) {
        ta->on_key_onelinedown_pressed = value;
    }
    /*attribs*/
    else if (strcmp(attrib, "name") == 0) {
        strncpy(ta->name, value, strlen(value));
    } else {
        dbg_str(DBG_DETAIL, "ta set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Text_Area *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(DBG_WARNNING, "text_area get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __load_resources(Component *component, void *window)
{
    Render *r      = ((Window *)window)->render;
    Text_Area *ta = (Text_Area *)component;
    Text *text    = ta->text;
    Character *character;

    dbg_str(DBG_SUC, "%s load load_resources", component->name);

    ta->window          = window;

    /*
     *r->font->load_ascii_character(r->font, r);
     */
    text->last_line_num = text->write_text(text, 0, text->content, r->font) - 1;
    /*
     *ta->text->line_info->for_each(ta->text->line_info, print_line_info);
     */

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
    Text_Area *ta          = (Text_Area *)component;
    Text *text             = ta->text;
    Render *r               = (Render *)render;
    Subject *s             = (Subject *)component;
    cursor_t *cursor       = &ta->cursor;
    text_line_t *line_info = NULL;
    int i, j;
    char c, c_bak;
    int width_bak;

    /*
     *dbg_str(DBG_DETAIL, "%s draw", ((Obj *)component)->name);
     */
    r->set_color(r, 0xff, 0xff, 0xff, 0xff);
    r->clear(r);

    r->draw_rect(r, s->x, s->y, s->width, s->height);

    cursor->x = 0; cursor->y = 0; cursor->width = 0; 

    for (j = ta->start_line; cursor->y < ((Subject *)component)->height; j++) {
        line_info = (text_line_t *)text->get_text_line_info(text, j);
        if (line_info == NULL) break;

        /*
         *dbg_str(DBG_DETAIL, "draw line=%d, len=%d, cursor->x =%d, cursor->y =%d, str=%s", 
         *j, line_info->tail - line_info->head, cursor->x, cursor->y,  line_info->head);
         */
        for (i = 0; i < line_info->tail - line_info->head + 1; i++) {
            c = line_info->head[i];
            draw_character(component, c, render);

            if (i == 0 && j == ta->start_line) { /*bak cursor info of first char of the screen*/
                c_bak     = line_info->head[0];
                width_bak = cursor->width;
            }
        }
        cursor->x       = 0;
        cursor->y      += cursor->height;
    }

    cursor->x      = 0;
    cursor->y      = 0;
    cursor->offset = 0;
    cursor->c      = c_bak;
    cursor->width  = width_bak;

    r->present(r);
}

static int __on_key_text_pressed(Component *component, char c, void *render)
{
    Render *r                 = (Render *)render;
    Text_Area *ta            = (Text_Area *)component;
    Text *text               = ta->text;
    cursor_t *cursor         = &ta->cursor, cursor_bak;
    int disturbed_line_count = 0;
    Character *character;
    uint16_t cursor_line;
    int from, to;
    int line_count_of_a_screen;

    line_count_of_a_screen = ((Subject *)component)->height / ta->char_height;
    cursor_line            = get_row_at_cursor(component);
    disturbed_line_count   = text->write_char(text, cursor_line , 
                                              cursor->offset, 
                                              cursor->x, 
                                              c, r->font);

    character              = (Character *)r->font->ascii[c].character;
    cursor->c              = c;
    cursor->width          = character->width;
    from                   = cursor->y / cursor->height;
    to                     = from + disturbed_line_count - 1;

    if (to > line_count_of_a_screen - 1) {
        to = line_count_of_a_screen - 1;
    }

    move_cursor_right(component);

    cursor_bak             = *cursor;

    /*draw lines disturbed by writing a char*/
    erase_n_lines_of_text(component, from, to, render);
    draw_n_lines_of_text(component, from, to, r);

    *cursor                = cursor_bak;
    draw_cursor(component, r);

    return 0;
}

static int __on_key_backspace_pressed(Component *component, void *render)
{
    Render *r                 = (Render *)render;
    Text_Area *ta            = (Text_Area *)component;
    Text *text               = ta->text;
    cursor_t *cursor         = &ta->cursor, cursor_bak;
    int disturbed_line_count = 0;
    Character *character;
    uint16_t cursor_line;
    char c;
    int ret;
    int from, to;
    int line_count_of_a_screen;

    dbg_str(DBG_DETAIL, "on_key_backspace_pressed");

    line_count_of_a_screen = ((Subject *)component)->height / ta->char_height;
    c                      = cursor->c;

    ret                    = move_cursor_left(component);
    if (ret < 1) { return 0;}

    cursor_line            = get_row_at_cursor(component);
    disturbed_line_count   = text->delete_char(text, cursor_line , 
                                               cursor->offset, 
                                               cursor->x, 
                                               r->font);

    character              = (Character *)r->font->ascii[c].character;
    cursor->c              = c;
    cursor->width          = character->width;
    from                   = cursor->y / cursor->height;
    to                     = from + disturbed_line_count - 1;

    if (to > line_count_of_a_screen - 1) {
        to = line_count_of_a_screen - 1;
    }

    cursor_bak             = *cursor;

    erase_n_lines_of_text(component, from, to, r);
    draw_n_lines_of_text(component, from, to, r);

    *cursor                = cursor_bak;
    draw_cursor(component, r);

    return 0;
}

static int __on_key_up_pressed(Component *component, void *render)
{
    Render *r          = (Render *)render;
    Text_Area *ta     = (Text_Area *)component;
    cursor_t *cursor  = &ta->cursor;
    color_t *bg_color = &ta->background_color;

    reverse_cursor(component, r);
    move_cursor_up(component);
    draw_cursor(component, r);

    ta->cursor_count  = 0;

    return 0;
}

static int __on_key_down_pressed(Component *component, void *render)
{
    Render *r         = (Render *)render;
    Text_Area *ta    = (Text_Area *)component;
    cursor_t *cursor = &ta->cursor;

    reverse_cursor(component, r);
    move_cursor_down(component);
    draw_cursor(component, r);

    ta->cursor_count = 0;

    return 0;
}

static int __on_key_left_pressed(Component *component, void *render)
{
    Render *r         = (Render *)render;
    Text_Area *ta    = (Text_Area *)component;
    cursor_t *cursor = &ta->cursor;

    /*
     *dbg_str(DBG_DETAIL, "on_key_left_pressed");
     */

    reverse_cursor(component, r);
    move_cursor_left(component);
    draw_cursor(component, r);

    ta->cursor_count = 0;
}

static int __on_key_right_pressed(Component *component, void *render)
{
    Render *r         = (Render *)render;
    Text_Area *ta    = (Text_Area *)component;
    cursor_t *cursor = &ta->cursor;

    /*
     *dbg_str(DBG_DETAIL, "on_key_right_pressed");
     */

    reverse_cursor(component, r);
    move_cursor_right(component);
    draw_cursor(component, r);

    ta->cursor_count = 0;
}

static int __on_key_pageup_pressed(Component *component, void *render)
{
    Render *r               = (Render *)render;
    Text_Area *ta          = (Text_Area *)component;
    Text *text             = ta->text;
    text_line_t *line_info = NULL;
    cursor_t *cursor       = &ta->cursor, cursor_bak;
    int line_count_of_a_screen;
    uint16_t cursor_line;
    Character *character;
    char c;

    dbg_str(DBG_DETAIL, "on_key_pageup_pressed");
    line_count_of_a_screen = ((Subject *)component)->height / ta->char_height;

    if (ta->start_line - line_count_of_a_screen > 0) {
        ta->start_line -= line_count_of_a_screen;
    } else if (ta->start_line > 0) {
        ta->start_line = 0;
    } else {
        dbg_str(DBG_WARNNING, "on_key_pageup_pressed");
        return 0;
    }

    cursor->y      = 0;
    cursor->x      = 0;
    cursor->offset = 0;

    ta->draw(component, render); 
}

static int __on_key_pagedown_pressed(Component *component, void *render)
{
    Render *r               = (Render *)render;
    Text_Area *ta          = (Text_Area *)component;
    Text *text             = ta->text;
    text_line_t *line_info = NULL;
    cursor_t *cursor       = &ta->cursor, cursor_bak;
    int line_count_of_a_screen;
    uint16_t cursor_line;
    Character *character;
    char c;

    dbg_str(DBG_DETAIL, "on_key_pagedown_pressed");
    line_count_of_a_screen = ((Subject *)component)->height / ta->char_height;

    if (ta->start_line + line_count_of_a_screen < text->last_line_num ) {
        ta->start_line += line_count_of_a_screen;
    } else {
        return 0;
    }

    cursor->y      = 0;
    cursor->x      = 0;
    cursor->offset = 0;
    ta->draw(component, render); 
}

static int __on_key_onelineup_pressed(Component *component, void *render)
{
    Render *r         = (Render *)render;
    Text_Area *ta    = (Text_Area *)component;
    cursor_t *cursor = &ta->cursor;

    dbg_str(DBG_DETAIL, "one line up");

    cursor->y        = 0;
    cursor->x        = 0;
    ta->start_line++;
    ta->draw(component, render); 
}

static int __on_key_onelinedown_pressed(Component *component, void *render)
{
    Text_Area *ta    = (Text_Area *)component;
    Render *r         = (Render *)render;
    cursor_t *cursor = &ta->cursor;

    dbg_str(DBG_DETAIL, "one line down");

    cursor->y        = 0;
    cursor->x        = 0;

    if (ta->start_line) {
        ta->start_line--;
        ta->draw(component, render); 
    } else if (ta->start_line == 0) {
        ta->draw(component, render); 
    }

}

static class_info_entry_t text_area_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Component", "component", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "draw", __draw, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "load_resources", __load_resources, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_text_pressed", __on_key_text_pressed, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_backspace_pressed", __on_key_backspace_pressed, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_up_pressed", __on_key_up_pressed, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_down_pressed", __on_key_down_pressed, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_left_pressed", __on_key_left_pressed, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_right_pressed", __on_key_right_pressed, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_pageup_pressed", __on_key_pageup_pressed, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_pagedown_pressed", __on_key_pagedown_pressed, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_onelineup_pressed", __on_key_onelineup_pressed, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_FUNC_POINTER, "", "on_key_onelinedown_pressed", __on_key_onelinedown_pressed, sizeof(void *)}, 
    [17] = {ENTRY_TYPE_STRING, "char", "name", NULL, 0}, 
    [18] = {ENTRY_TYPE_END}, 

};
REGISTER_CLASS("Text_Area", text_area_class_info);

char *gen_text_area_setting_str()
{
    cjson_t *root, *b, *c, *e, *s;
    char *set_str;

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Text_Area", b = cjson_create_object());{
            cjson_add_item_to_object(root, "Component", c = cjson_create_object());{
                cjson_add_item_to_object(root, "Container", e = cjson_create_object());{
                    cjson_add_item_to_object(e, "Subject", s = cjson_create_object());{
                        cjson_add_number_to_object(s, "x", 0);
                        cjson_add_number_to_object(s, "y", 0);
                        cjson_add_number_to_object(s, "width", 600);
                        cjson_add_number_to_object(s, "height", 600);
                    }
                }
                cjson_add_string_to_object(c, "name", "text_area");
            }
        }
    }
    set_str = cjson_print(root);

    return set_str;
}
int text_area()
{
    Window *window;
    Container *container;
    Render *r;
    Subject *subject;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    char buf[2048];

    set_str   = gen_window_setting_str();
    window    = OBJECT_NEW(allocator, Sdl_Window, set_str);
    r         = window->render;
    container = (Container *)window;

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL, "Window dump: %s", buf);

    set_str   = gen_text_area_setting_str();
    subject   = OBJECT_NEW(allocator, Text_Area, set_str);

    object_dump(subject, "Text_Area", buf, 2048);
    dbg_str(DBG_DETAIL, "Text_Area dump: %s", buf);

    container->add_component(container, NULL, subject);
    /*
     *container->search_component(container, "text_area");
     */
    dbg_str(DBG_DETAIL, "window container :%p", container);

    window->load_resources(window);
    window->update_window(window);
    window->event->poll_event(window->event, window);
    window->unload_resources(window);
    object_destroy(window);

    free(set_str);
    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(text_area);
