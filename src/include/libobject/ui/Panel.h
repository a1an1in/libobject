#ifndef __PANEL_H__
#define __PANEL_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/Component.h>
#include <libobject/ui/Image.h>
#include <libobject/ui/Font.h>
#include <libobject/ui/Event.h>

typedef struct panel_s Panel;

struct panel_s{
    Component component;

    int (*construct)(Panel *panel,char *init_str);
    int (*deconstruct)(Panel *panel);
    int (*set)(Panel *panel, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*update_panel)(Panel *panel);

    /*virtual methods reimplement*/
    int (*draw)(Component *component, void *graph);

    /*inherit methods*/

    /*attribs*/
};

#endif
