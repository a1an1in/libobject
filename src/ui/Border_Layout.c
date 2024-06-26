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
#include <libobject/ui/Border_Layout.h>
#include <libobject/ui/Sdl_Window.h>
#include <libobject/ui/Character.h>
#include <libobject/ui/Timer.h>
#include <libobject/ui/Label.h>
#include <libobject/core/utils/config.h>

static void modulate_component_position(Border_Layout *border_layout)
{
    Border_Layout *l       = border_layout;
    Container *container = (Container *)l;
    int i;
    position_t position;
    Component *c;

    if (    l->blocks[BORDER_LAYOUT_NORTH].width != l->blocks[BORDER_LAYOUT_NORTH].width_bak ||
            l->blocks[BORDER_LAYOUT_NORTH].height != l->blocks[BORDER_LAYOUT_NORTH].height_bak)
    {
    }

    if (    l->blocks[BORDER_LAYOUT_WEST].width != l->blocks[BORDER_LAYOUT_WEST].width_bak ||
            l->blocks[BORDER_LAYOUT_WEST].height != l->blocks[BORDER_LAYOUT_WEST].height_bak)
    {
            position.x = 0;
            position.y = l->blocks[BORDER_LAYOUT_NORTH].height - l->blocks[BORDER_LAYOUT_NORTH].height_bak;

            c = l->blocks[BORDER_LAYOUT_WEST].component;
            if (c != NULL)
                container->update_component_position(c, &position);
    }

    if (    l->blocks[BORDER_LAYOUT_CENTER].width != l->blocks[BORDER_LAYOUT_CENTER].width_bak ||
            l->blocks[BORDER_LAYOUT_CENTER].height != l->blocks[BORDER_LAYOUT_CENTER].height_bak)
    {
            position.x = l->blocks[BORDER_LAYOUT_WEST].width - l->blocks[BORDER_LAYOUT_WEST].width_bak;
            position.y = l->blocks[BORDER_LAYOUT_NORTH].height - l->blocks[BORDER_LAYOUT_NORTH].height_bak;

            c = l->blocks[BORDER_LAYOUT_CENTER].component;
            if (c != NULL)
                container->update_component_position(c, &position);
    }

    if (    l->blocks[BORDER_LAYOUT_EAST].width != l->blocks[BORDER_LAYOUT_EAST].width_bak ||
            l->blocks[BORDER_LAYOUT_EAST].height != l->blocks[BORDER_LAYOUT_EAST].height_bak)
    {
            position.x = l->blocks[BORDER_LAYOUT_WEST].width - l->blocks[BORDER_LAYOUT_WEST].width_bak + 
                         l->blocks[BORDER_LAYOUT_CENTER].width - l->blocks[BORDER_LAYOUT_CENTER].width_bak;
            position.y = l->blocks[BORDER_LAYOUT_NORTH].height - l->blocks[BORDER_LAYOUT_NORTH].height_bak;

            c = l->blocks[BORDER_LAYOUT_EAST].component;
            if (c != NULL)
                container->update_component_position(c, &position);
    }

    if (    l->blocks[BORDER_LAYOUT_SOUTH].width != l->blocks[BORDER_LAYOUT_SOUTH].width_bak ||
            l->blocks[BORDER_LAYOUT_SOUTH].height != l->blocks[BORDER_LAYOUT_SOUTH].height_bak)
    {
            position.x = 0;
            position.y = l->blocks[BORDER_LAYOUT_NORTH].height - l->blocks[BORDER_LAYOUT_NORTH].height_bak +
                         l->blocks[BORDER_LAYOUT_CENTER].height - l->blocks[BORDER_LAYOUT_CENTER].height_bak;

            c = l->blocks[BORDER_LAYOUT_SOUTH].component;
            if (c != NULL)
                container->update_component_position(c, &position);
    }
}

static int bakeup_old_size(Border_Layout *border_layout)
{

    Border_Layout *l       = border_layout;

    l->blocks[BORDER_LAYOUT_NORTH].width_bak   = l->blocks[BORDER_LAYOUT_NORTH].width;
    l->blocks[BORDER_LAYOUT_NORTH].height_bak  = l->blocks[BORDER_LAYOUT_NORTH].height;
    l->blocks[BORDER_LAYOUT_WEST].width_bak    = l->blocks[BORDER_LAYOUT_WEST].width;
    l->blocks[BORDER_LAYOUT_WEST].height_bak   = l->blocks[BORDER_LAYOUT_WEST].height;
    l->blocks[BORDER_LAYOUT_CENTER].width_bak  = l->blocks[BORDER_LAYOUT_CENTER].width;
    l->blocks[BORDER_LAYOUT_CENTER].height_bak = l->blocks[BORDER_LAYOUT_CENTER].height;
    l->blocks[BORDER_LAYOUT_EAST].width_bak    = l->blocks[BORDER_LAYOUT_EAST].width;
    l->blocks[BORDER_LAYOUT_EAST].height_bak   = l->blocks[BORDER_LAYOUT_EAST].height;
    l->blocks[BORDER_LAYOUT_SOUTH].width_bak   = l->blocks[BORDER_LAYOUT_SOUTH].width;
    l->blocks[BORDER_LAYOUT_SOUTH].height_bak  = l->blocks[BORDER_LAYOUT_SOUTH].height;

    return 0;
}

static int generate_new_size(Border_Layout *border_layout)
{
    Border_Layout *l       = border_layout;
    Subject *s             = (Subject *)border_layout;

    bakeup_old_size(l);

    s->width                               = l->layout_width;
    s->height                              = l->layout_height;

    l->blocks[BORDER_LAYOUT_NORTH].width   = (int)(l->layout_width);
    l->blocks[BORDER_LAYOUT_NORTH].height  = (int)(l->layout_height *
                                             l->height_ratio_of_north_to_layout);

    l->blocks[BORDER_LAYOUT_WEST].width    = (int)(l->layout_width *
                                             l->width_ratio_of_west_to_layout);
    l->blocks[BORDER_LAYOUT_WEST].height   = (int)(l->layout_height *
                                             l->height_ratio_of_center_to_layout);

    l->blocks[BORDER_LAYOUT_CENTER].width  = (int)(l->layout_width *
                                             l->width_ratio_of_center_to_layout);
    l->blocks[BORDER_LAYOUT_CENTER].height = (int)(l->layout_height *
                                             l->height_ratio_of_center_to_layout);

    l->blocks[BORDER_LAYOUT_EAST].width    = (int)(l->layout_width *
                                             l->width_ratio_of_east_to_layout);
    l->blocks[BORDER_LAYOUT_EAST].height   = (int)(l->layout_height * 
                                             l->height_ratio_of_center_to_layout);

    l->blocks[BORDER_LAYOUT_SOUTH].width   = (int)(l->layout_width);
    l->blocks[BORDER_LAYOUT_SOUTH].height  = (int)(l->layout_height * 
                                             l->height_ratio_of_south_to_layout);

    printf( "blocks[BORDER_LAYOUT_NORTH].width=%d, height=%d, \n"
            "blocks[BORDER_LAYOUT_WEST].width=%d, height=%d, \n"
            "blocks[BORDER_LAYOUT_CENTER].width=%d, height=%d, \n"
            "blocks[BORDER_LAYOUT_EAST].width=%d, height=%d, \n"
            "blocks[BORDER_LAYOUT_SOUTH].width=%d, height=%d\n", 
            l->blocks[BORDER_LAYOUT_NORTH].width, 
            l->blocks[BORDER_LAYOUT_NORTH].height, 
            l->blocks[BORDER_LAYOUT_WEST].width, 
            l->blocks[BORDER_LAYOUT_WEST].height, 
            l->blocks[BORDER_LAYOUT_CENTER].width, 
            l->blocks[BORDER_LAYOUT_CENTER].height, 
            l->blocks[BORDER_LAYOUT_EAST].width, 
            l->blocks[BORDER_LAYOUT_EAST].height, 
            l->blocks[BORDER_LAYOUT_SOUTH].width, 
            l->blocks[BORDER_LAYOUT_SOUTH].height);

    printf("bakeup size\n");

    printf( "blocks[BORDER_LAYOUT_NORTH].width_bak=%d, height_bak=%d, \n"
            "blocks[BORDER_LAYOUT_WEST].width_bak=%d, height_bak=%d, \n"
            "blocks[BORDER_LAYOUT_CENTER].width_bak=%d, height_bak=%d, \n"
            "blocks[BORDER_LAYOUT_EAST].width_bak=%d, height_bak=%d, \n"
            "blocks[BORDER_LAYOUT_SOUTH].width_bak=%d, height_bak=%d\n", 
            l->blocks[BORDER_LAYOUT_NORTH].width_bak, 
            l->blocks[BORDER_LAYOUT_NORTH].height_bak, 
            l->blocks[BORDER_LAYOUT_WEST].width_bak, 
            l->blocks[BORDER_LAYOUT_WEST].height_bak, 
            l->blocks[BORDER_LAYOUT_CENTER].width_bak, 
            l->blocks[BORDER_LAYOUT_CENTER].height_bak, 
            l->blocks[BORDER_LAYOUT_EAST].width_bak, 
            l->blocks[BORDER_LAYOUT_EAST].height_bak, 
            l->blocks[BORDER_LAYOUT_SOUTH].width_bak, 
            l->blocks[BORDER_LAYOUT_SOUTH].height_bak);

}

static int __construct(Border_Layout *border_layout, char *init_str)
{
    allocator_t *allocator = ((Obj *)border_layout)->allocator;
    Border_Layout *l       = border_layout;
    Subject *s             = (Subject *)border_layout;
    uint32_t i;

    dbg_str(DBG_INFO, "border_layout construct");

    memset(l->blocks, 0, sizeof(layout_block_t) * BORDER_LAYOUT_MAX); 

    if (s->width == 0) {
        l->layout_width = 30;
        s->width        = l->layout_width;
    } else {
        l->layout_width = s->width;
    }
    if (s->height == 0) {
        l->layout_height = 10;
        s->height        = l->layout_height;
    } else {
        l->layout_height = s->height;
    }
    l->width_ratio_of_west_to_layout    = (float) (3.0 / 10);
    l->width_ratio_of_east_to_layout    = (float) (3.0 / 10);
    l->width_ratio_of_center_to_layout  = (float) (1.0 - l->width_ratio_of_west_to_layout -
                                          l->width_ratio_of_east_to_layout);
    l->height_ratio_of_north_to_layout  = (float) (3.0 / 10);
    l->height_ratio_of_south_to_layout  = (float) (3.0 / 10);
    l->height_ratio_of_center_to_layout = (float) (1.0 - l->height_ratio_of_north_to_layout -
                                          l->height_ratio_of_south_to_layout);

    dbg_str(DBG_DETAIL, "r1 =%f, r2=%f , r3=%f", 
            l->width_ratio_of_west_to_layout, 
            l->width_ratio_of_east_to_layout, 
            l->width_ratio_of_center_to_layout);

    generate_new_size(border_layout);


    dbg_str(DBG_DETAIL, "lgap=%d vgap =%d", l->hgap, l->vgap);
    dbg_str(DBG_DETAIL, "border_layout construct end");

    return 0;
}

static int __deconstrcut(Border_Layout *border_layout)
{
    allocator_t *allocator = ((Obj *)border_layout)->allocator;

    dbg_str(DBG_INFO, "border_layout deconstruct");

    return 0;
}

static int __add_component(Container *obj, void *pos, void *component)
{
    Border_Layout *l                 = (Border_Layout *)obj;
    Container *container             = (Container *)obj;
    Map *map                         = container->map;
    Component *c                     = (Component *)component;
    Subject *s                       = (Subject *)c;
    uint8_t rearrange_comonents_flag = 0;
    position_t position;
    int index = -1;
    int height, width, h, w;
    int size_change_flag = 0;

    if (pos == NULL) {
        dbg_str(DBG_DETAIL, "border layout add component error, please dessignate pos");
        return -1;
    }
    if (component == NULL) {
        dbg_str(DBG_DETAIL, "border_layout add null component at %s", pos);
    } else {
        dbg_str(DBG_DETAIL, "border_layout add component:%s at %s", c->name, pos);
    }

    if (strncmp(pos, "North", 5) == 0) {
        index = BORDER_LAYOUT_NORTH;
    } else if (strncmp(pos, "West", 4) == 0 ) {
        index = BORDER_LAYOUT_WEST;
    } else if (strncmp(pos, "Center", 6) == 0) {
        index = BORDER_LAYOUT_CENTER;
    } else if (strncmp(pos, "East", 4) == 0) {
        index = BORDER_LAYOUT_EAST;
    } else if (strncmp(pos, "South", 5) == 0) {
        index = BORDER_LAYOUT_SOUTH;
    } else {
        dbg_str(DBG_WARN, "borderlayout add component, pos par err");
        return -1;
    }

    if (component == NULL) {
        l->blocks[index].component  = NULL;

        switch (index) {
            case BORDER_LAYOUT_NORTH:
                l->height_ratio_of_north_to_layout        = 0;
                l->blocks[BORDER_LAYOUT_NORTH].height_bak = 0;
                l->blocks[BORDER_LAYOUT_NORTH].height     = 0;
                l->height_ratio_of_center_to_layout       = (float) (1.0 - l->height_ratio_of_north_to_layout -
                                                            l->height_ratio_of_south_to_layout);

                break;
            case BORDER_LAYOUT_WEST:
                l->width_ratio_of_west_to_layout          = 0;
                l->blocks[BORDER_LAYOUT_WEST].width_bak   = 0;
                l->blocks[BORDER_LAYOUT_WEST].width       = 0;
                l->width_ratio_of_center_to_layout        = (float) (1.0 - l->width_ratio_of_west_to_layout -
                                                            l->width_ratio_of_east_to_layout);
                break;
            case BORDER_LAYOUT_CENTER:
                l->width_ratio_of_center_to_layout        = 0;
                l->blocks[BORDER_LAYOUT_CENTER].width_bak = 0;
                l->blocks[BORDER_LAYOUT_CENTER].width     = 0;
                break;
            case BORDER_LAYOUT_EAST:
                l->width_ratio_of_east_to_layout          = 0;
                l->blocks[BORDER_LAYOUT_EAST].width_bak   = 0;
                l->blocks[BORDER_LAYOUT_EAST].width       = 0;
                l->width_ratio_of_center_to_layout        = (float) (1.0 - l->width_ratio_of_west_to_layout -
                                                            l->width_ratio_of_east_to_layout);
                break;
            case BORDER_LAYOUT_SOUTH:
                l->height_ratio_of_south_to_layout        = 0;
                l->blocks[BORDER_LAYOUT_SOUTH].height_bak = 0;
                l->blocks[BORDER_LAYOUT_SOUTH].height     = 0;
                l->height_ratio_of_center_to_layout       = (float) (1.0 - l->height_ratio_of_north_to_layout -
                                                            l->height_ratio_of_south_to_layout);
                dbg_str(DBG_SUC, "south set 0");
                break;
            default:
                return -1;
                break;
        }
        generate_new_size(l);
        return 0;
    }

    l->blocks[index].component  = component;

    if (l->blocks[index].width < s->width + 2 * l->hgap) {
        l->blocks[index].width  = s->width + 2 * l->hgap;
    }

    if (l->blocks[index].height < s->height + 2 * l->vgap) {
        l->blocks[index].height = s->height + 2 * l->vgap;
    }

    switch (index) {
        case BORDER_LAYOUT_NORTH:
            {
                height = (int)(l->blocks[index].height / l->height_ratio_of_north_to_layout);

                if (l->layout_width < l->blocks[index].width) {
                    l->layout_width = l->blocks[index].width;
                    size_change_flag = 1;
                }

                if (l->layout_height < height) {
                    l->layout_height        = height;
                    h                       = (int)(l->layout_height * l->height_ratio_of_north_to_layout);
                    l->blocks[index].height = h;
                    size_change_flag        = 1;
                }
            }
            break;
        case BORDER_LAYOUT_WEST:
            {
                width  = (int)(l->blocks[index].width / l->width_ratio_of_west_to_layout);
                height = (int)(l->blocks[index].height / l->height_ratio_of_center_to_layout);

                if (l->layout_width < width) {
                    l->layout_width  = width;
                    size_change_flag = 1;
                }

                if (l->layout_height < height) {
                    l->layout_height        = height;
                    h                       = (int)(l->layout_height * l->height_ratio_of_center_to_layout);
                    l->blocks[index].height = h;
                    size_change_flag        = 1;
                }
            }
            break;
        case BORDER_LAYOUT_CENTER:
            {
                width  = (int)(l->blocks[index].width / l->width_ratio_of_center_to_layout);
                height = (int)(l->blocks[index].height / l->height_ratio_of_center_to_layout);

                if (l->layout_width < width) {
                    l->layout_width  = width;
                    size_change_flag = 1;
                }

                if (l->layout_height < height) {
                    l->layout_height        = height;
                    h                       = (int)(l->layout_height * l->height_ratio_of_center_to_layout);
                    w                       = (int)(l->layout_width * l->width_ratio_of_center_to_layout);
                    l->blocks[index].height = h;
                    l->blocks[index].width  = w;
                    size_change_flag        = 1;
                }
            }
            break;
        case BORDER_LAYOUT_EAST:
            {
                width  = (int)(l->blocks[index].width / l->width_ratio_of_east_to_layout);
                height = (int)(l->blocks[index].height / l->height_ratio_of_center_to_layout);

                if (l->layout_width < width) {
                    l->layout_width  = width;
                    size_change_flag = 1;
                }

                if (l->layout_height < height) {
                    l->layout_height        = height;
                    h                       = (int)(l->layout_height * l->height_ratio_of_center_to_layout);
                    l->blocks[index].height = h;
                    size_change_flag        = 1;
                }
            }
            break;
        case BORDER_LAYOUT_SOUTH:
            {
                height = (int)(l->blocks[index].height / l->height_ratio_of_south_to_layout);

                if (l->layout_width < l->blocks[index].width) {
                    l->layout_width  = l->blocks[index].width;
                    size_change_flag = 1;
                }

                if (l->layout_height < height) {
                    l->layout_height        = height;
                    h                       = (int)(l->layout_height * l->height_ratio_of_south_to_layout);
                    l->blocks[index].height = h;
                    size_change_flag        = 1;
                }
            }
            break;
        default:
            return -1;
            break;
    }

    if (size_change_flag == 1) {
        dbg_str(DBG_WARN, "border layout size changed, index=%d", index);
        generate_new_size(l);
    }

    switch (index) {
        case BORDER_LAYOUT_NORTH:
            {
                position.x = l->hgap;
                position.y = l->vgap;
            }
            break;
        case BORDER_LAYOUT_WEST:
            {
                position.x = l->hgap;
                position.y = l->blocks[BORDER_LAYOUT_NORTH].height + l->vgap;
            }
            break;
        case BORDER_LAYOUT_CENTER:
            {
                position.x = l->blocks[BORDER_LAYOUT_WEST].width + l->hgap;
                position.y = l->blocks[BORDER_LAYOUT_NORTH].height + l->vgap;
                dbg_str(DBG_INFO, "x =%d, y=%d", position.x, position.y);
            }
            break;
        case BORDER_LAYOUT_EAST:
            {
                position.x = l->blocks[BORDER_LAYOUT_WEST].width +
                             l->blocks[BORDER_LAYOUT_CENTER].width + l->hgap;
                position.y = l->blocks[BORDER_LAYOUT_NORTH].height + l->vgap;
            
            }
            break;
        case BORDER_LAYOUT_SOUTH:
            {
                position.x = l->hgap;
                position.y = l->blocks[BORDER_LAYOUT_NORTH].height +
                             l->blocks[BORDER_LAYOUT_WEST].height + l->vgap;
            }
            break;
        default:
            return -1;
            break;
    }

    container->update_component_position(c, &position);

    map->add(map, c->name, c);

    if (size_change_flag == 1) {
        modulate_component_position(l);
        size_change_flag = 0;
    }

    return 0;
}

static void draw_subcomponent_foreach_cb(void *key, void *element, void *arg) 
{
    Component *component;
    uint8_t *addr;
    Render *r = (Render *)arg;

    component = (Component *)element;

    if (component->draw) component->draw(component, r);
}

static void draw_border(Component *component, void *render) 
{
    Border_Layout *l = (Border_Layout *)component;
    Render *r         = (Render *)render;
    Subject *s       = (Subject *)component;
    int i;
    position_t start, end;

    dbg_str(DBG_DETAIL, "draw_border");

    r->set_color(r, 0, 0, 0, 0xff);

    r->draw_rect(r, s->x, s->y, l->layout_width, l->layout_height);

	r->draw_line(r, 
                        s->x, 
                        s->y + l->blocks[BORDER_LAYOUT_NORTH].height, 
                        s->x + l->layout_width, 
                        s->y + l->blocks[BORDER_LAYOUT_NORTH].height);

	r->draw_line(r, 
                        s->x, 
                        s->y + l->blocks[BORDER_LAYOUT_NORTH].height + l->blocks[BORDER_LAYOUT_CENTER].height, 
                        s->x + l->layout_width, 
                        s->y + l->blocks[BORDER_LAYOUT_NORTH].height + l->blocks[BORDER_LAYOUT_CENTER].height);
    /*
     *dbg_str(DBG_SUC, "width=%d, height=%d", l->layout_width, l->layout_height);
     */

    if (l->blocks[BORDER_LAYOUT_WEST].width != 0)
        r->draw_line(r, 
                            s->x + l->blocks[BORDER_LAYOUT_WEST].width, 
                            s->y + l->blocks[BORDER_LAYOUT_NORTH].height, 
                            s->x + l->blocks[BORDER_LAYOUT_WEST].width, 
                            s->y + l->blocks[BORDER_LAYOUT_NORTH].height + l->blocks[BORDER_LAYOUT_CENTER].height);

    if (l->blocks[BORDER_LAYOUT_EAST].width != 0)
        r->draw_line(r, 
                            s->x + l->blocks[BORDER_LAYOUT_WEST].width + l->blocks[BORDER_LAYOUT_CENTER].width, 
                            s->y + l->blocks[BORDER_LAYOUT_NORTH].height, 
                            s->x + l->blocks[BORDER_LAYOUT_WEST].width + l->blocks[BORDER_LAYOUT_CENTER].width, 
                            s->y + l->blocks[BORDER_LAYOUT_NORTH].height + l->blocks[BORDER_LAYOUT_CENTER].height);

    /*
     *dbg_str(DBG_DETAIL, 
     *        "blocks[BORDER_LAYOUT_NORTH].width=%d, height=%d, "
     *        "blocks[BORDER_LAYOUT_WEST].width=%d, height=%d, "
     *        "blocks[BORDER_LAYOUT_CENTER].width=%d, height=%d, "
     *        "blocks[BORDER_LAYOUT_EAST].width=%d, height=%d, "
     *        "blocks[BORDER_LAYOUT_SOUTH].width=%d, height=%d", 
     *        l->blocks[BORDER_LAYOUT_NORTH].width, 
     *        l->blocks[BORDER_LAYOUT_NORTH].height, 
     *        l->blocks[BORDER_LAYOUT_WEST].width, 
     *        l->blocks[BORDER_LAYOUT_WEST].height, 
     *        l->blocks[BORDER_LAYOUT_CENTER].width, 
     *        l->blocks[BORDER_LAYOUT_CENTER].height, 
     *        l->blocks[BORDER_LAYOUT_EAST].width, 
     *        l->blocks[BORDER_LAYOUT_EAST].height, 
     *        l->blocks[BORDER_LAYOUT_SOUTH].width, 
     *        l->blocks[BORDER_LAYOUT_SOUTH].height);
     */
}

/*reimplement the virtual func draw() int Component class*/
static int __draw(Component *component, void *render)
{
    Container *container = (Container *)component;
    Render *r             = (Render *)render;

    dbg_str(DBG_DETAIL, "%s draw", ((Obj *)component)->name);

    /*draw layouts*/
    draw_border(component, render);

    /*draw subcomponent*/
    container->for_each_component(container, draw_subcomponent_foreach_cb, r);
}

static class_info_entry_t border_layout_class_info[] = {
    Init_Obj___Entry(0 , Component, component),
    Init_Nfunc_Entry(1 , Border_Layout, construct, __construct),
    Init_Nfunc_Entry(2 , Border_Layout, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Border_Layout, set, NULL),
    Init_Vfunc_Entry(4 , Border_Layout, get, NULL),
    Init_Vfunc_Entry(5 , Border_Layout, add_component, __add_component),
    Init_Vfunc_Entry(6 , Border_Layout, draw, __draw),
    Init_Str___Entry(7 , Border_Layout, name, NULL),
    Init_U32___Entry(8 , Border_Layout, hgap, NULL),
    Init_U32___Entry(9 , Border_Layout, vgap, NULL),
    Init_End___Entry(10, Border_Layout),
};
REGISTER_CLASS(Border_Layout, border_layout_class_info);


#if 0
char *gen_border_layout_setting_str(int x, int y, int width, int height, char *name, void *out)
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
                    \"Border_Layout\":{\
                        \"hgap\":%d, \
                        \"vgap\":%d\
                    }\
                }";

    sprintf(out, set_str, x, y, width, height, name, 2, 2);

    return out;
}

void *new_border_layout(allocator_t *allocator, int x, int y, int width, int height, char *name)
{
    char *set_str;
    char buf[2048];
    Container *container;

    gen_border_layout_setting_str(x, y, width, height, name, buf);
    container = OBJECT_NEW(allocator, Border_Layout, buf);

    /*
     *object_dump(container, "Border_Layout", buf, 2048);
     *dbg_str(DBG_DETAIL, "Border_Layout dump: %s", buf);
     */

    return container;
}
#else
void *new_border_layout(allocator_t *allocator, int x, int y, int width, int height, char *name)
{
#define MAX_BUFFER_LEN 1024
    char *set_str;
    Container *container;
    configurator_t * c;
    char buf[MAX_BUFFER_LEN] = {0};
    int vgap = 2, hgap = 2;

    c = cfg_alloc(allocator); 
    cfg_config_num(c, "/Subject", "x", x);
    cfg_config_num(c, "/Subject", "y", y);
    cfg_config_num(c, "/Subject", "width", width);
    cfg_config_num(c, "/Subject", "height", height);
    cfg_config_str(c, "/Component/name", "name", name) ;
    cfg_config_num(c, "/Border_Layout", "hgap", hgap);
    cfg_config_num(c, "/Border_Layout", "vgap", vgap);

    dbg_str(DBG_DETAIL, "border init str:%s", c->buf);
    container = OBJECT_NEW(allocator, Border_Layout, c->buf);

    return container;
#undef MAX_BUFFER_LEN
}
#endif


void test_ui_border_layout()
{
    allocator_t *allocator = allocator_get_default_instance();
    Window *window;
    Border_Layout *layout;
    Label *l;
    char *set_str;
    char buf[2048];

    set_str = gen_window_setting_str();
    window  = OBJECT_NEW(allocator, Sdl_Window, set_str);

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL, "Window dump: %s", buf);

    layout = new_border_layout(allocator, 0, 0, 0, 0, "border_layout");

#if 0
    l = new_label(allocator, 0, 0, 80, 18, "label00");
    layout->add_component((Container *)layout, "North", l);
    l = new_label(allocator, 0, 0, 80, 18, "label01");
    layout->add_component((Container *)layout, "West", l);
    l = new_label(allocator, 0, 0, 80, 20, "label02");
    layout->add_component((Container *)layout, "Center", l);
    l = new_label(allocator, 0, 0, 80, 28, "label03");
    layout->add_component((Container *)layout, "East", l);
    l = new_label(allocator, 0, 0, 80, 68, "label10");
    layout->add_component((Container *)layout, "South", l);
#else
    l = new_label(allocator, 0, 0, 80, 18, "label00");
    layout->add_component((Container *)layout, "North", l);
    layout->add_component((Container *)layout, "West", NULL);
    l = new_label(allocator, 0, 0, 80, 20, "label02");
    layout->add_component((Container *)layout, "Center", l);
    layout->add_component((Container *)layout, "East", NULL);
    l = new_label(allocator, 0, 0, 80, 20, "label10");
    layout->add_component((Container *)layout, "South", l);
#endif

    window->add_component((Container *)window, NULL, layout);
    window->load_resources(window);
    window->update_window(window);
    window->event->poll_event(window->event, window);
    window->unload_resources(window);

    object_destroy(window);
}
