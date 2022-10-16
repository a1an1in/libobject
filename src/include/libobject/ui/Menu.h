#ifndef __MENU_H__
#define __MENU_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/Component.h>
#include <libobject/ui/Label.h>

typedef struct menu_s Menu;

struct menu_s{
    Component component;

    int (*construct)(Menu *menu,char *init_str);
    int (*deconstruct)(Menu *menu);
    int (*set)(Menu *menu, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*draw)(Component *component, void *graph);
};
#endif
