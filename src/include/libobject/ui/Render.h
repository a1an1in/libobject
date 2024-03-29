#ifndef __RENDER_H__
#define __RENDER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/ui/Font.h>
/*
 *#include <libui/window.h>
 */

typedef struct s Render;

struct s{
    Obj obj;

    int (*construct)(Render *render,char *init_str);
    int (*deconstruct)(Render *render);
    int (*set)(Render *render, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*move)(Render *render);
    int (*update_window)(Render *render);
    int (*set_window)(Render *render, void *window);
    int (*draw_image0)(Render *render, void *image);
    int (*create)(Render *render);
    int (*destroy)(Render *render);
    int (*set_color)(Render *render, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    int (*set_font)(Render *render, void *font);
    int (*clear)(Render *render);
    int (*draw_line)(Render *render,int x1, int y1, int x2, int y2);
    int (*draw_rect)(Render *render,int x1, int y1, int x2, int y2);
    int (*fill_rect)(Render *render,int x1, int y1, int x2, int y2);
    void *(*load_image)(Render *render,void *path);
    int (*draw_image)(Render *render,int x, int y, void *image);
    int (*draw_texture)(Render *render, int x, int y, int width, int height, void *texture);
    void *(*load_text)(Render *render,void *string,void *font,int r, int g, int b, int a);
    int (*unload_text)(Render *render,void *text);
    int (*write_text)(Render *render,int x, int y, void *text);
    void *(*load_character)(Render *render,uint32_t code,void *font,int r, int g, int b, int a);
    int (*unload_character)(Render *render,void *character);
    int (*write_character)(Render *render,int x, int y, void *character);
    int (*present)(Render *render);
    int (*draw_component)(Render *render, void *component);
    void* (*create_yuvtexture)(Render *render, int w, int h);
    int (*destroy_texture)(Render *render, void *texture);

#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    Font *font;

};

#endif
