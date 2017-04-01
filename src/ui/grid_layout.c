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
#include <miscellany/buffer.h>

static int __construct(Grid_Layout *grid_layout,char *init_str)
{
    allocator_t *allocator = ((Obj *)grid_layout)->allocator;
    Grid_Layout *l          = grid_layout;
    uint32_t i;

    dbg_str(DBG_DETAIL,"grid_layout construct");

    l->default_unit_width  = 100;
    l->default_unit_height = 20;
    l->cur_col             = 0;
    l->cur_row             = 0;
    l->layout_height       = 0;
    l->layout_width        = 0;

    if (l->row_max == 0) { l->row_max = 1;}
    if (l->col_max == 0) { l->col_max = 1;}

    dbg_str(DBG_DETAIL,"row_max = %d, col_max=%d", l->row_max, l->col_max);

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

    dbg_str(DBG_SUC,"grid_layout deconstruct");

    allocator_mem_free(allocator, grid_layout->row_height);
    allocator_mem_free(allocator, grid_layout->col_width);
    allocator_mem_free(allocator, grid_layout->grid_components);

    return 0;
}

static int __set(Grid_Layout *grid_layout, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        grid_layout->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        grid_layout->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        grid_layout->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        grid_layout->deconstruct = value;
    }
    /*vitual methods*/
    else if (strcmp(attrib, "add_component") == 0) {
        grid_layout->add_component = value;
    } else if (strcmp(attrib, "draw") == 0) {
        grid_layout->draw = value;
    } else if (strcmp(attrib, "load_resources") == 0) {
        grid_layout->load_resources = value;
    }
    /*attribs*/
    else if (strcmp(attrib, "name") == 0) {
        dbg_str(DBG_SUC,"set grid_layout name");
        strncpy(grid_layout->name,value,strlen(value));
    } else if (strcmp(attrib, "row_max") == 0) {
        grid_layout->row_max = *((uint32_t *)value);
    } else if (strcmp(attrib, "col_max") == 0) {
        grid_layout->col_max = *((uint32_t *)value);
    } else if (strcmp(attrib, "hgap") == 0) {
        grid_layout->hgap = *((uint32_t *)value);
    } else if (strcmp(attrib, "vgap") == 0) {
        grid_layout->vgap = *((uint32_t *)value);
    } else {
        dbg_str(DBG_DETAIL,"grid_layout set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Grid_Layout *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else if (strcmp(attrib, "row_max") == 0) {
        return &obj->row_max;
    } else if (strcmp(attrib, "col_max") == 0) {
        return &obj->col_max;
    } else if (strcmp(attrib, "hgap") == 0) {
        return &obj->hgap;
    } else if (strcmp(attrib, "vgap") == 0) {
        return &obj->vgap;
    } else {
        dbg_str(DBG_WARNNING,"grid_layout get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
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
    char buffer[8]       = {0};
    Component *c         = (Component *)component;
    Subject *subject     = (Subject *)c;
    uint8_t rearrange_comonents_flag = 0;
    position_t position;

    if (strcmp(c->name,"") == 0) {
        dbg_str(DBG_WARNNING,"component name is NULL, this is vip, add component failed, please check");
        return -1;
    }

    dbg_str(DBG_IMPORTANT, "add component name %s, component addr %p", c->name,c);

    addr_to_buffer(c,(uint8_t *)buffer);

    position.x = get_x_axis_of_current_grid(l) + l->hgap;
    position.y = get_y_axis_of_current_grid(l) + l->vgap;

    dbg_str(DBG_SUC,"position x=%d, y=%d", position.x, position.y);
    container->update_component_position(c, &position);

    map->insert(map, c->name, buffer);
    dbg_str(DBG_DETAIL,"grid_components:%p, cur_row:%d, cur_col:%d, c:%p",
            l->grid_components, l->cur_row, l->cur_col,c);

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
        dbg_str(DBG_WARNNING,"not support rearrange component in grid now");
        return -1;
    }
    
    l->cur_col++;
    if (l->cur_col > l->col_max - 1) {
        l->cur_col = 0;
        l->cur_row++;
    }

    return 0;
}

static void draw_subcomponent(Iterator *iter, void *arg) 
{
    Component *component;
    uint8_t *addr;
    Graph *g = (Graph *)arg;

    addr      = (uint8_t *)iter->get_vpointer(iter);
    component = (Component *)buffer_to_addr(addr);

    if (component->draw) component->draw(component, g);
}

static void draw_grids(Component *component, void *graph) 
{
    Grid_Layout *l = (Grid_Layout *)component;
    Graph *g      = (Graph *)graph;
    int i;
    position_t start, end;

    dbg_str(DBG_SUC,"draw_grids");

    /*draw row lines*/
    g->render_set_color(g,0x0,0x0,0x0,0xff);
    start.x = 0;
    start.y = 0;
    end.x   = l->layout_width;
    end.y   = 0;
    g->render_draw_line(g,start.x, start.y, end.x, end.y);

    for (i = 0; i < l->row_max; i++) {
        start.y += l->row_height[i];
        end.y   += l->row_height[i];
        g->render_draw_line(g,start.x, start.y, end.x, end.y);
    }

    /*draw column lines*/
    start.x = 0;
    start.y = 0;
    end.x   = 0;
    end.y   = l->layout_height;
    g->render_draw_line(g,start.x, start.y, end.x, end.y);

    for (i = 0; i < l->row_max; i++) {
        start.x += l->col_width[i];
        end.x   += l->col_width[i];
        g->render_draw_line(g,start.x, start.y, end.x, end.y);
    }
}

/*reimplement the virtual func draw() int Component class*/
static int __draw(Component *component, void *graph)
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;

    dbg_str(DBG_SUC,"%s draw", ((Obj *)component)->name);

    /*draw grids*/
    draw_grids(component, graph);

    /*draw subcomponent*/
    container->for_each_component(container, draw_subcomponent, g);
}

static class_info_entry_t grid_layout_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Component","component",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","add_component",__add_component,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","draw",__draw,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_STRING,"char","name",NULL,0},
    [8 ] = {ENTRY_TYPE_INT32_T,"int","row_max",NULL,sizeof(int)},
    [9 ] = {ENTRY_TYPE_INT32_T,"int","col_max",NULL,sizeof(int)},
    [10] = {ENTRY_TYPE_INT32_T,"int","hgap",NULL,sizeof(int)},
    [11] = {ENTRY_TYPE_INT32_T,"int","vgap",NULL,sizeof(int)},
    [12] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Grid_Layout",grid_layout_class_info);

char *gen_grid_layout_setting_str(int x, int y, int width, int height, char *name, void *out)
{
    char *set_str = NULL;

    set_str = "{\
                    \"Subject\": {\
                        \"x\":%d,\
                        \"y\":%d,\
                        \"width\":%d,\
                        \"height\":%d\
                    },\
                    \"Component\": {\
                        \"name\":\"%s\"\
                    },\
                    \"Grid_Layout\":{\
                        \"row_max\":%d,\
                        \"col_max\":%d,\
                        \"hgap\":%d,\
                        \"vgap\":%d\
                    }\
                }";

    sprintf(out, set_str, x, y, width, height, name, 4,4, 5, 2);

    return out;
}

void *new_grid_layout(allocator_t *allocator, int x, int y, int width, int height, char *name)
{
    char *set_str;
    char buf[2048];
    Container *container;

    gen_grid_layout_setting_str(x, y, width, height, name, buf);
    container = OBJECT_NEW(allocator, Grid_Layout,buf);

    object_dump(container, "Grid_Layout", buf, 2048);
    dbg_str(DBG_DETAIL,"Grid_Layout dump: %s",buf);

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
    window  = OBJECT_NEW(allocator, Sdl_Window,set_str);

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL,"Window dump: %s",buf);

    grid = new_grid_layout(allocator, 0, 0, 600, 600, "grid");

    l = new_label(allocator,0, 0, 80, 20, "label00");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator,0, 0, 80, 20, "label01");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator,0, 0, 80, 20, "label02");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator,0, 0, 80, 20, "label03");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator,0, 0, 80, 20, "label10");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator,0, 0, 80, 20, "label11");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator,0, 0, 80, 20, "label12");
    grid->add_component((Container *)grid, NULL, l);
    l = new_label(allocator,0, 0, 80, 20, "label13");
    grid->add_component((Container *)grid, NULL, l);

    window->add_component((Container *)window,NULL, grid);
    window->load_resources(window);
    window->update_window(window);
    window->event->poll_event(window->event, window);

    object_destroy(window);
}
