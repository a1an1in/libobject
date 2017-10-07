/**
 * @file sdl_grath.c
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
#include <libobject/ui/window.h>
#include <libobject/ui/sdl_graph.h>
#include <libobject/ui/sdl_image.h>
#include <libobject/ui/sdl_text.h>
#include <libobject/ui/sdl_character.h>
#include <libobject/ui/sdl_window.h>

static int __construct(Sdl_Graph *sdl_grath,char *init_str)
{
    dbg_str(SDL_INTERFACE_DETAIL,"sdl_grath construct, sdl_grath addr:%p",sdl_grath);

    return 0;
}

static int __deconstrcut(Sdl_Graph *sdl_grath)
{
    dbg_str(SDL_INTERFACE_DETAIL,"sdl_grath deconstruct,sdl_grath addr:%p",sdl_grath);

    return 0;
}

static int __set(Sdl_Graph *sdl_grath, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        sdl_grath->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        sdl_grath->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        sdl_grath->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        sdl_grath->deconstruct = value;
    }
    else if (strcmp(attrib, "move") == 0) {/**virtual methods setting start*/
        sdl_grath->move = value;
    } else if (strcmp(attrib, "update_window") == 0) {
        sdl_grath->update_window = value;
    } else if (strcmp(attrib, "set_window") == 0) {
        sdl_grath->set_window = value;
    } else if (strcmp(attrib, "draw_image") == 0) {
        sdl_grath->draw_image = value;
    } else if (strcmp(attrib, "render_create") == 0) {
        sdl_grath->render_create = value;
    } else if (strcmp(attrib, "render_destroy") == 0) {
        sdl_grath->render_destroy = value;
    } else if (strcmp(attrib, "render_set_color") == 0) {
        sdl_grath->render_set_color = value;
    } else if (strcmp(attrib, "render_set_font") == 0) {
        sdl_grath->render_set_font = value;
    } else if (strcmp(attrib, "render_clear") == 0) {
        sdl_grath->render_clear = value;
    } else if (strcmp(attrib, "render_draw_line") == 0) {
        sdl_grath->render_draw_line = value;
    } else if (strcmp(attrib, "render_fill_rect") == 0) {
        sdl_grath->render_fill_rect = value;
    } else if (strcmp(attrib, "render_draw_rect") == 0) {
        sdl_grath->render_draw_rect = value;
    } else if (strcmp(attrib, "render_draw_image") == 0) {
        sdl_grath->render_draw_image = value;
    } else if (strcmp(attrib, "render_load_image") == 0) {
        sdl_grath->render_load_image = value;
    } else if (strcmp(attrib, "render_load_text") == 0) {
        sdl_grath->render_load_text = value;
    } else if (strcmp(attrib, "render_unload_text") == 0) {
        sdl_grath->render_unload_text = value;
    } else if (strcmp(attrib, "render_write_text") == 0) {
        sdl_grath->render_write_text = value;
    } else if (strcmp(attrib, "render_load_character") == 0) {
        sdl_grath->render_load_character = value;
    } else if (strcmp(attrib, "render_unload_character") == 0) {
        sdl_grath->render_unload_character = value;
    } else if (strcmp(attrib, "render_write_character") == 0) {
        sdl_grath->render_write_character = value;
    } else if (strcmp(attrib, "render_present") == 0) {
        sdl_grath->render_present = value;
    } 
    else if (strcmp(attrib, "name") == 0) { /**attribs setting start*/
        strncpy(sdl_grath->name,value,strlen(value));
    } else {
        dbg_str(SDL_INTERFACE_DETAIL,"sdl_graph set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Sdl_Graph *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(DBG_WARNNING,"sdl_grath get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __set_window(Sdl_Graph *graph, void *window)
{
    Sdl_Window *w = (Sdl_Window *)window;
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph set_window");

    graph->window = w->sdl_window;
}

static int __draw_image(Sdl_Graph *graph, void *image)
{
    Sdl_Image *i = (Sdl_Image *)image;
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph draw_image");

    SDL_BlitSurface(i->surface, NULL, graph->screen_surface, NULL );
}

static int __update_window(Sdl_Graph *graph)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph update_window");
    SDL_UpdateWindowSurface(graph->window);
}

static int __render_create(Sdl_Graph *graph)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph render_create");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1" );
    graph->render = SDL_CreateRenderer( graph->window, -1, 0);
    if (graph->render == NULL) {
        dbg_str(SDL_INTERFACE_ERROR,"Sdl_Graph render_create, %s", SDL_GetError());
        exit(-1);
    }
}

static int __render_destroy(Sdl_Graph *graph)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph render_destroy");
    SDL_DestroyRenderer(graph->render);
}

static int __render_set_color(Sdl_Graph *graph,
                              uint8_t r, uint8_t g,
                              uint8_t b, uint8_t a)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph set_color");
    SDL_SetRenderDrawColor(graph->render, r, g, b, a);
}

static int __render_set_font(Sdl_Graph *graph, void *font)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph set_font");
    ((Graph *)graph)->font = font;

    return 0;
}
static int __render_clear(Sdl_Graph *graph)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph render_clear");
    SDL_RenderClear(graph->render);
}

static int __render_draw_line(Sdl_Graph *graph,int x1, int y1, int x2, int y2)
{
    dbg_str(SDL_INTERFACE_DETAIL,"SDL Graph render_draw_line");
    SDL_RenderDrawLine(graph->render,x1, y1, x2, y2);
}

static int __render_draw_rect(Sdl_Graph *graph,int x1, int y1,int wight, int height)
{
    dbg_str(SDL_INTERFACE_DETAIL,"SDL Graph render_draw_rect");
    SDL_Rect rect = {x1,y1,wight,height};
    SDL_RenderDrawRect(graph->render, &rect);
}

static int __render_fill_rect(Sdl_Graph *graph,int x1, int y1, int wight, int height)
{
    dbg_str(SDL_INTERFACE_DETAIL,"SDL Graph render_fill_rect");
    SDL_Rect fillRect = {x1,y1,wight,height};
    SDL_RenderFillRect(graph->render, &fillRect );
}

static void *__render_load_image(Sdl_Graph *graph,void *path)
{

    Sdl_Image *image;
    allocator_t *allocator = ((Obj *)graph)->allocator;

    dbg_str(SDL_INTERFACE_DETAIL,"SDL Graph render_load_image");

    image = OBJECT_NEW(allocator, Sdl_Image,"");
    ((Image *)image)->path->assign(((Image *)image)->path,path);
    if (image->surface == NULL) {
        image->load_image((Image *)image);
    }

    if (image->surface != NULL) {
        image->texture = SDL_CreateTextureFromSurface(graph->render, image->surface);
        image->width   = image->surface->w;
        image->height  = image->surface->h;
        SDL_FreeSurface(image->surface);
        image->surface = NULL;
    }

    return image;
}

static int __render_unload_image(Sdl_Graph *graph,void *image)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Text unload image");
    object_destroy(image);
}

static int __render_draw_image(Sdl_Graph *graph,int x, int y, void *image)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph render_draw_image");
    Sdl_Image *i = (Sdl_Image *)image;
    SDL_Rect render_quad = { x, y, i->width, i->height};
    SDL_RenderCopy(graph->render, i->texture, NULL, &render_quad );
}

#if 0
static void * __render_load_text(Sdl_Graph *graph,void *string,void *font,int r, int g, int b, int a)
{
    allocator_t *allocator = ((Obj *)graph)->allocator;
    Sdl_Text *text;
    Sdl_Font *f = (Sdl_Font *)font;
    SDL_Surface* surface = NULL;
    SDL_Color textColor = {r, g, b, a };
    String *content;

    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Text load text");
    text = OBJECT_NEW(allocator, Sdl_Text,"");
    content = ((Text *)text)->content;
    content->assign(content,string);

    surface = TTF_RenderText_Solid(f->ttf_font,
                                   ((Text *)text)->content->value,
                                   textColor ); 

    if (surface != NULL) {
        text->texture = SDL_CreateTextureFromSurface(graph->render, surface);
        text->width = surface->w;
        text->height = surface->h;
        dbg_str(SDL_INTERFACE_DETAIL,"width =%d height=%d",text->width, text->height);
        SDL_FreeSurface(surface);
    }

    return text;
}

static int __render_unload_text(Sdl_Graph *graph, void *text)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Text unload text");
    object_destroy(text);
}

static int __render_write_text(Sdl_Graph *graph,int x, int y, void *text)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph render_write_text");
    Sdl_Text *t = (Sdl_Text *)text;
    SDL_Rect render_quad = { x, y, t->width, t->height};
    SDL_RenderCopy(graph->render, t->texture, NULL, &render_quad );
}
#endif

static void * __render_load_character(Sdl_Graph *graph,uint32_t code,void *font,int r, int g, int b, int a)
{
    Character *character;
    allocator_t *allocator    = ((Obj *)graph)->allocator;
    Sdl_Font *f               = (Sdl_Font *)font;
    SDL_Surface* surface      = NULL;
    SDL_Color character_color = {r, g, b, a };
    char buf[10]              = {0};

    /*
     *dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Character load character");
     */
    character = OBJECT_NEW(allocator, Sdl_Character,"");
    character->assign(character,code);

    sprintf(buf,"%c",code);

    /*
     *surface = TTF_RenderText_Solid(f->ttf_font,
     *                               buf,
     *                               character_color); 
     */
    
    surface = TTF_RenderText_Blended(f->ttf_font,
                                     buf,
                                     character_color); 

    if (surface != NULL) {
        ((Sdl_Character *)character)->texture = SDL_CreateTextureFromSurface(graph->render, surface);
        character->width   = surface->w;
        character->height  = surface->h;
        /*
         *dbg_str(SDL_INTERFACE_DETAIL,"width =%d height=%d",character->width, character->height);
         */
        SDL_FreeSurface(surface);
    }

    return character;
}

static int __render_unload_character(Sdl_Graph *graph, void *character)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Text unload character");
    object_destroy(character);
}

static int __render_write_character(Sdl_Graph *graph,int x, int y, void *character)
{
    /*
     *dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph render_write_character");
     */
    Character *c         = (Character *)character;
    SDL_Rect render_quad = { x, y, c->width, c->height};

    SDL_RenderCopy(graph->render, ((Sdl_Character *)character)->texture, NULL, &render_quad );
}

static int __render_present(Sdl_Graph *graph)
{
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph render_present");
    SDL_RenderPresent(graph->render);
}

static class_info_entry_t sdl_grath_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Graph","graph",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_window",__set_window,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","draw_image",__draw_image,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","update_window",__update_window,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","render_create",__render_create,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","render_destroy",__render_destroy,sizeof(void *)},
    [10] = {ENTRY_TYPE_FUNC_POINTER,"","render_set_color",__render_set_color,sizeof(void *)},
    [11] = {ENTRY_TYPE_FUNC_POINTER,"","render_set_font",__render_set_font,sizeof(void *)},
    [12] = {ENTRY_TYPE_FUNC_POINTER,"","render_clear",__render_clear,sizeof(void *)},
    [13] = {ENTRY_TYPE_FUNC_POINTER,"","render_draw_line",__render_draw_line,sizeof(void *)},
    [14] = {ENTRY_TYPE_FUNC_POINTER,"","render_draw_rect",__render_draw_rect,sizeof(void *)},
    [15] = {ENTRY_TYPE_FUNC_POINTER,"","render_fill_rect",__render_fill_rect,sizeof(void *)},
    [16] = {ENTRY_TYPE_FUNC_POINTER,"","render_draw_image",__render_draw_image,sizeof(void *)},
    [17] = {ENTRY_TYPE_FUNC_POINTER,"","render_load_image",__render_load_image,sizeof(void *)},
    [18] = {ENTRY_TYPE_FUNC_POINTER,"","render_load_character",__render_load_character,sizeof(void *)},
    [19] = {ENTRY_TYPE_FUNC_POINTER,"","render_unload_character",__render_unload_character,sizeof(void *)},
    [20] = {ENTRY_TYPE_FUNC_POINTER,"","render_write_character",__render_write_character,sizeof(void *)},
    [21] = {ENTRY_TYPE_FUNC_POINTER,"","render_present",__render_present,sizeof(void *)},
    [22] = {ENTRY_TYPE_STRING,"char","name",NULL,0},
    [23] = {ENTRY_TYPE_END},

    /*
     *[19] = {ENTRY_TYPE_FUNC_POINTER,"","render_load_text",__render_load_text,sizeof(void *)},
     *[20] = {ENTRY_TYPE_FUNC_POINTER,"","render_unload_text",__render_unload_text,sizeof(void *)},
     *[21] = {ENTRY_TYPE_FUNC_POINTER,"","render_write_text",__render_write_text,sizeof(void *)},
     */
};
REGISTER_CLASS("Sdl_Graph",sdl_grath_class_info);

void test_ui_sdl_grath()
{
    Sdl_Graph *sdl_grath;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Sdl_Graph", e = cjson_create_object());{
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str = cjson_print(root);

    sdl_grath = OBJECT_NEW(allocator, Sdl_Graph,set_str);

    object_dump(sdl_grath, "Sdl_Graph", buf, 2048);
    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph dump: %s",buf);

    object_destroy(sdl_grath);

    free(set_str);

}


