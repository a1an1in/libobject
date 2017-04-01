#ifndef __GRIDLAYOUT_H__
#define __GRIDLAYOUT_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/ui/component.h>
#include <libobject/ui/text.h>
#include <libobject/ui/color.h>
#include <libobject/ui/cursor.h>

typedef struct grid_layout_s Grid_Layout;

struct grid_layout_s{
	Component component;

	int (*construct)(Grid_Layout *grid_layout,char *init_str);
	int (*deconstruct)(Grid_Layout *grid_layout);
	int (*set)(Grid_Layout *grid_layout, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add_component)(Container *obj, void *pos, void *component);
	int (*load_resources)(Component *component, void *window);
	int (*draw)(Component *component, void *graph);

#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    void *window;
    uint32_t *col_width, *row_height; 
    uint32_t hgap, vgap;
    uint32_t row_max, col_max, cur_row, cur_col;
    uint32_t default_unit_width, default_unit_height;
    uint32_t layout_width, layout_height;

    Component **grid_components;
};

#endif
