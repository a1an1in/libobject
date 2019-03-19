/**
 * @file girdlayout.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 
 * @date 2017-02-09
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
#include <libobject/ui/grid_layout.h>
#include <libobject/ui/sdl_window.h>
#include <libobject/ui/character.h>
#include <libobject/ui/timer.h>
#include <libobject/ui/label.h>

static int __construct(Grid_Layout *grid_layout, char *init_str)
{
    allocator_t *allocator = ((Obj *)grid_layout)->allocator;
    Grid_Layout *l          = grid_layout;
    uint32_t i;

    dbg_str(DBG_DETAIL, "grid_layout construct");

    l->default_unit_width  = 100;
    l->default_unit_height = 20;
    l->cur_col             = 0;
    l->cur_row             = 0;
    l->layout_height       = 0;
    l->layout_width        = 0;

    if (l->row_max == 0) { l->row_max = 1;}
    if (l->col_max == 0) { l->col_max = 1;}

    dbg_str(DBG_DETAIL, "row_max = %d, col_max=%d", l->row_max, l->col_max);

    /*alloc grid height and width arrays*/
    l->row_height = (uint32_t *) 
                    allocator_mem_alloc(allocator, 
                                        l->row_max * sizeof(uint32_t *));
    l->col_width  = (uint32_t *)
                    allocator_mem_alloc(allocator, 
                                        l->col_max * sizeof(uint32_t *));
    /*init grid row height*/
    for (i = 0; i < l->row_max; i++) {
        l->row_height[i]  = l->default_unit_height;
        l->layout_height += l->row_height[i];
    }

    /*init grid col width*/
    for ( i = 0; i < l->col_max; i++) {
        l->col_width[i]  = l->default_unit_width;
        l->layout_width += l->col_width[i];
    }

    /*alloc space for grid componets*/
    l->grid_components = (Component **) 
                         allocator_mem_alloc(allocator, 
                                             l->row_max * l->col_max * sizeof(Component *));

    memset(l->grid_components, 0, l->row_max * l->col_max * sizeof(Component *));

    return 0;
}

static int __deconstrcut(Grid_Layout *grid_layout)
{
    allocator_t *allocator = ((Obj *)grid_layout)->allocator;

    dbg_str(DBG_SUC, "grid_layout deconstruct");

    allocator_mem_free(allocator, grid_layout->row_height);
    allocator_mem_free(allocator, grid_layout->col_width);
    allocator_mem_free(allocator, grid_layout->grid_components);

    return 0;
}

static int get_x_axis_of_current_grid(Grid_Layout *obj)
{
    Grid_Layout *l = obj;
    int i, x;

    for ( i = 0, x = 0; i < l->cur_col; i++) {
        x += l->col_width[i];
    }

    return x;
}

static int get_y_axis_of_current_grid(Grid_Layout *obj)
{
    Grid_Layout *l = obj;
    int i, y;

    for ( i = 0, y = 0; i < l->cur_row; i++) {
        y += l->row_height[i];
    }

    return y;
}

static int __add_component(Container *obj, void *pos, void *component)
{
    Grid_Layout *l       = (Grid_Layout *)obj;
    Container *container = (Container *)obj;
    Map *map             = container->map;
    Component *c         = (Component *)component;
    Subject *subject     = (Subject *)c;
    uint8_t rearrange_comonents_flag = 0;
    position_t position;

    if (strcmp(c->name, "") == 0) {
        dbg_str(DBG_WARNNING, "component name is NULL, this is vip, add component failed, please check");
        return -1;
    }

    dbg_str(DBG_IMPORTANT, "add component name %s, component addr %p", c->name, c);

    position.x = get_x_axis_of_current_grid(l) + l->hgap;
    position.y = get_y_axis_of_current_grid(l) + l->vgap;

    dbg_str(DBG_SUC, "position x=%d, y=%d", position.x, position.y);
    container->update_component_position(c, &position);

    map->add(map, c->name, c);
    dbg_str(DBG_DETAIL, "grid_components:%p, cur_row:%d, cur_col:%d, c:%p", 
            l->grid_components, l->cur_row, l->cur_col, c);

    *(l->grid_components + l->cur_row * l->col_max + l->cur_col) = c; 

    if (l->row_height[l->cur_row] < subject->height) {
        l->row_height[l->cur_row] = subject->height;
        rearrange_comonents_flag = 1;
    } 

    if (l->col_width[l->cur_col] < subject->width) {
        l->col_width[l->cur_col] = subject->width;
        rearrange_comonents_flag = 1;
    }

    if (rearrange_comonents_flag == 1) {
        dbg_str(DBG_WARNNING, "not support rearrange component in grid now");
        return -1;
    }
    
    l->cur_col++;
    if (l->cur_col > l->col_max - 1) {
        l->cur_col = 0;
        l->cur_row++;
    }

    return 0;
}

static void draw_subcomponent(void *key, void *element, void *arg) 
{
    Component *component;
    uint8_t *addr;
    Render *r = (Render *)arg;

    component = (Component *)element;

    if (component->draw) component->draw(component, r);
}

static void draw_grids(Component *component, void *render) 
{
    Grid_Layout *l = (Grid_Layout *)component;
    Render *r      = (Render *)render;
    int i;
    position_t start, end;

    dbg_str(DBG_SUC, "draw_grids");

    /*draw row lines*/
    r->set_color(r, 0x0, 0x0, 0x0, 0xff);
    start.x = 0;
    start.y = 0;
    end.x   = l->layout_width;
    end.y   = 0;
    r->draw_line(r, start.x, start.y, end.x, end.y);

    for (i = 0; i < l->row_max; i++) {
        start.y += l->row_height[i];
        end.y   += l->row_height[i];
        r->draw_line(r, start.x, start.y, end.x, end.y);
    }

    /*draw column lines*/
    start.x = 0;
    start.y = 0;
    end.x   = 0;
    end.y   = l->layout_height;
    r->draw_line(r, start.x, start.y, end.x, end.y);

    for (i = 0; i < l->row_max; i++) {
        start.x += l->col_width[i];
        end.x   += l->col_width[i];
        r->draw_line(r, start.x, start.y, end.x, end.y);
    }
}

/*reimplement the virtual func draw() int Component class*/
static int __draw(Component *component, void *render)
{
    Container *container = (Container *)component;
    Render *r             = (Render *)render;

    dbg_str(DBG_SUC, "%s draw", ((Obj *)component)->name);

    /*draw grids*/
    draw_grids(component, render);

    /*draw subcomponent*/
    container->for_each_component(container, draw_subcomponent, r);
}

static class_info_entry_t grid_layout_class_info[] = {
    Init_Obj___Entry(0 , Component, component),
    Init_Nfunc_Entry(1 , Grid_Layout, construct, __construct),
    Init_Nfunc_Entry(2 , Grid_Layout, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Grid_Layout, set, NULL),
    Init_Vfunc_Entry(4 , Grid_Layout, get, NULL),
    Init_Vfunc_Entry(5 , Grid_Layout, add_component, __add_component),
    Init_Vfunc_Entry(6 , Grid_Layout, draw, __draw),
    Init_Str___Entry(7 , Grid_Layout, name, NULL),
    Init_U32___Entry(8 , Grid_Layout, hgap, NULL),
    Init_U32___Entry(9 , Grid_Layout, vgap, NULL),
    Init_U32___Entry(10, Grid_Layout, row_max, NULL),
    Init_U32___Entry(11, Grid_Layout, col_max, NULL),
    Init_End___Entry(12),
};
REGISTER_CLASS("Grid_Layout", grid_layout_class_info);

char *gen_grid_layout_setting_str(int x, int y, int width, int height, char *name, void *out)
{
    char *set_str = NULL;

    set_str = "{\
                    \"Subject\": {\
                        \"x\":%d, \
                        \"y\":%d, \
                        \"width\":%d, \
                        \"height\":%d\
                    }, \
                    \"Component\": {\
                        \"name\":\"%s\"\
                    }, \
                    \"Grid_Layout\":{\
                        \"row_max\":%d, \
                        \"col_max\":%d, \
                        \"hgap\":%d, \
                        \"vgap\":%d\
                    }\
                }";

    sprintf(out, set_str, x, y, width, height, name, 4, 4, 5, 2);

    return out;
}

void *new_grid_layout(allocator_t *allocator, int x, int y, int width, int height, char *name)
{
    char *set_str;
    char buf[2048];
    Container *container;

    gen_grid_layout_setting_str(x, y, width, height, name, buf);
    container = OBJECT_NEW(allocator, Grid_Layout, buf);

    object_dump(container, "Grid_Layout", buf, 2048);
    dbg_str(DBG_DETAIL, "Grid_Layout dump: %s", buf);

    return container;
}

void test_ui_grid_layout()
{
    allocator_t *allocator = allocator_get_default_alloc();
    Window *window;
    Grid_Layout *grid;
    Label *l;
    char *set_str;
    char buf[2048];

    set_str = gen_window_setting_str();
    window  = OBJECT_NEW(allocator, Sdl_Window, set_str);

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL, "Window dump: %s", buf);

    grid = new_grid_layout(allocator, 0, 0, 600, 600, "grid");

    l = new_label(allocator, 0, 0, 80, 20, "label00");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator, 0, 0, 80, 20, "label01");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator, 0, 0, 80, 20, "label02");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator, 0, 0, 80, 20, "label03");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator, 0, 0, 80, 20, "label10");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator, 0, 0, 80, 20, "label11");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator, 0, 0, 80, 20, "label12");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator, 0, 0, 80, 20, "label13");
    grid->add_component((Container *)grid, NULL, l);

    window->add_component((Container *)window, NULL, grid);
    window->load_resources(window);
    window->update_window(window);
    window->event->poll_event(window->event, window);

    object_destroy(window);
}
