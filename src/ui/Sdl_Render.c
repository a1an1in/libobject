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
#include <libobject/ui/sdl_render.h>
#include <libobject/ui/sdl_image.h>
#include <libobject/ui/sdl_text.h>
#include <libobject/ui/sdl_character.h>
#include <libobject/ui/sdl_window.h>

static int __construct(Sdl_Render *sdl_grath, char *init_str)
{
    dbg_str(SDL_INTERFACE_DETAIL, "sdl_grath construct, sdl_grath addr:%p", sdl_grath);

    return 0;
}

static int __deconstrcut(Sdl_Render *sdl_grath)
{
    dbg_str(SDL_INTERFACE_DETAIL, "sdl_grath deconstruct, sdl_grath addr:%p", sdl_grath);

    return 0;
}

static int __set(Sdl_Render *sdl_grath, char *attrib, void *value)
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
    } else if (strcmp(attrib, "draw_image0") == 0) {
        sdl_grath->draw_image0 = value;
    } else if (strcmp(attrib, "create") == 0) {
        sdl_grath->create = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        sdl_grath->destroy = value;
    } else if (strcmp(attrib, "set_color") == 0) {
        sdl_grath->set_color = value;
    } else if (strcmp(attrib, "set_font") == 0) {
        sdl_grath->set_font = value;
    } else if (strcmp(attrib, "clear") == 0) {
        sdl_grath->clear = value;
    } else if (strcmp(attrib, "draw_line") == 0) {
        sdl_grath->draw_line = value;
    } else if (strcmp(attrib, "fill_rect") == 0) {
        sdl_grath->fill_rect = value;
    } else if (strcmp(attrib, "draw_rect") == 0) {
        sdl_grath->draw_rect = value;
    } else if (strcmp(attrib, "draw_image") == 0) {
        sdl_grath->draw_image = value;
    } else if (strcmp(attrib, "draw_texture") == 0) {
        sdl_grath->draw_texture = value;
    } else if (strcmp(attrib, "load_image") == 0) {
        sdl_grath->load_image = value;
    } else if (strcmp(attrib, "load_text") == 0) {
        sdl_grath->load_text = value;
    } else if (strcmp(attrib, "unload_text") == 0) {
        sdl_grath->unload_text = value;
    } else if (strcmp(attrib, "write_text") == 0) {
        sdl_grath->write_text = value;
    } else if (strcmp(attrib, "load_character") == 0) {
        sdl_grath->load_character = value;
    } else if (strcmp(attrib, "unload_character") == 0) {
        sdl_grath->unload_character = value;
    } else if (strcmp(attrib, "write_character") == 0) {
        sdl_grath->write_character = value;
    } else if (strcmp(attrib, "present") == 0) {
        sdl_grath->present = value;
    } else if (strcmp(attrib, "create_yuvtexture") == 0) {
        sdl_grath->create_yuvtexture = value;
    } else if (strcmp(attrib, "destroy_texture") == 0) {
        sdl_grath->destroy_texture = value;
    } 
    else if (strcmp(attrib, "name") == 0) { /**attribs setting start*/
        strncpy(sdl_grath->name, value, strlen(value));
    } else {
        dbg_str(SDL_INTERFACE_DETAIL, "sdl_render set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Sdl_Render *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(DBG_WARNNING, "sdl_grath get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __set_window(Sdl_Render *render, void *window)
{
    Sdl_Window *w = (Sdl_Window *)window;
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render set_window");

    render->window = w->sdl_window;
}

static int __draw_image0(Sdl_Render *render, void *image)
{
    Sdl_Image *i = (Sdl_Image *)image;
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render draw_image");

    SDL_BlitSurface(i->surface, NULL, render->screen_surface, NULL );
}

static int __update_window(Sdl_Render *render)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render update_window");
    SDL_UpdateWindowSurface(render->window);
}

static int __create(Sdl_Render *render)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render create");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1" );
    render->sdl_render = SDL_CreateRenderer( render->window, -1, 0);
    if (render->sdl_render == NULL) {
        dbg_str(SDL_INTERFACE_ERROR, "Sdl_Render create, %s", SDL_GetError());
        exit(-1);
    }
}

static int __destroy(Sdl_Render *render)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render destroy");
    SDL_DestroyRenderer(render->sdl_render);
}

static int __set_color(Sdl_Render *render, 
                              uint8_t r, uint8_t g, 
                              uint8_t b, uint8_t a)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render set_color");
    SDL_SetRenderDrawColor(render->sdl_render, r, g, b, a);
}

static int __set_font(Sdl_Render *render, void *font)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render set_font");
    ((Render *)render)->font = font;

    return 0;
}
static int __clear(Sdl_Render *render)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render clear");
    SDL_RenderClear(render->sdl_render);
}

static int __draw_line(Sdl_Render *render, int x1, int y1, int x2, int y2)
{
    dbg_str(SDL_INTERFACE_DETAIL, "SDL Render draw_line");
    SDL_RenderDrawLine(render->sdl_render, x1, y1, x2, y2);
}

static int __draw_rect(Sdl_Render *render, int x1, int y1, int wight, int height)
{
    dbg_str(SDL_INTERFACE_DETAIL, "SDL Render draw_rect");
    SDL_Rect rect = {x1, y1, wight, height};
    SDL_RenderDrawRect(render->sdl_render, &rect);
}

static int __fill_rect(Sdl_Render *render, int x1, int y1, int wight, int height)
{
    dbg_str(SDL_INTERFACE_DETAIL, "SDL Render fill_rect");
    SDL_Rect fillRect = {x1, y1, wight, height};
    SDL_RenderFillRect(render->sdl_render, &fillRect );
}

static void *__load_image(Sdl_Render *render, void *path)
{

    Sdl_Image *image;
    allocator_t *allocator = ((Obj *)render)->allocator;

    dbg_str(SDL_INTERFACE_DETAIL, "SDL Render load_image");

    image = OBJECT_NEW(allocator, Sdl_Image, "");
    ((Image *)image)->path->assign(((Image *)image)->path, path);
    if (image->surface == NULL) {
        image->load_image((Image *)image);
    }

    if (image->surface != NULL) {
        image->texture = SDL_CreateTextureFromSurface(render->sdl_render, image->surface);
        image->width   = image->surface->w;
        image->height  = image->surface->h;
        SDL_FreeSurface(image->surface);
        image->surface = NULL;
    }

    return image;
}

static int __unload_image(Sdl_Render *render, void *image)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Text unload image");
    object_destroy(image);
}


static int __draw_image(Sdl_Render *render, int x, int y, void *image)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render draw_image");
    Sdl_Image *i = (Sdl_Image *)image;
    SDL_Rect quad = { x, y, i->width, i->height};
    SDL_RenderCopy(render->sdl_render, i->texture, NULL, &quad );
}

static int __draw_texture(Sdl_Render *render, int x, int y, 
                          int width, int height, void *texture)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render draw_texture");
    SDL_Rect quad = { x, y, width, height};
    SDL_RenderCopy(render->sdl_render, texture, NULL, &quad );
}

#if 0
static void * __load_text(Sdl_Render *render, void *string, void *font, int r, int g, int b, int a)
{
    allocator_t *allocator = ((Obj *)render)->allocator;
    Sdl_Text *text;
    Sdl_Font *f = (Sdl_Font *)font;
    SDL_Surface* surface = NULL;
    SDL_Color textColor = {r, g, b, a };
    String *content;

    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Text load text");
    text = OBJECT_NEW(allocator, Sdl_Text, "");
    content = ((Text *)text)->content;
    content->assign(content, string);

    surface = TTF_RenderText_Solid(f->ttf_font, 
                                   ((Text *)text)->content->value, 
                                   textColor ); 

    if (surface != NULL) {
        text->texture = SDL_CreateTextureFromSurface(render->render, surface);
        text->width = surface->w;
        text->height = surface->h;
        dbg_str(SDL_INTERFACE_DETAIL, "width =%d height=%d", text->width, text->height);
        SDL_FreeSurface(surface);
    }

    return text;
}

static int __unload_text(Sdl_Render *render, void *text)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Text unload text");
    object_destroy(text);
}

static int __write_text(Sdl_Render *render, int x, int y, void *text)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render write_text");
    Sdl_Text *t = (Sdl_Text *)text;
    SDL_Rect quad = { x, y, t->width, t->height};
    SDL_RenderCopy(render->render, t->texture, NULL, &quad );
}
#endif

static void * __load_character(Sdl_Render *render, uint32_t code, void *font, int r, int g, int b, int a)
{
    Character *character;
    allocator_t *allocator    = ((Obj *)render)->allocator;
    Sdl_Font *f               = (Sdl_Font *)font;
    SDL_Surface* surface      = NULL;
    SDL_Color character_color = {r, g, b, a };
    char buf[10]              = {0};

    /*
     *dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Character load character");
     */
    character = OBJECT_NEW(allocator, Sdl_Character, "");
    character->assign(character, code);

    sprintf(buf, "%c", code);

    /*
     *surface = TTF_RenderText_Solid(f->ttf_font, 
     *                               buf, 
     *                               character_color); 
     */
    
    /*
     *surface = TTF_RenderText_Blended(f->ttf_font, 
     *                                 buf, 
     *                                 character_color); 
     */
    /*
     *surface = TTF_RenderGlyph_Blended(f->ttf_font,
     *                                  buf[0],
     *                                  character_color);
     */

    surface = TTF_RenderUTF8_Blended(f->ttf_font,
                                     buf,
                                     character_color);

    if (surface != NULL) {
        ((Sdl_Character *)character)->texture = SDL_CreateTextureFromSurface(render->sdl_render, surface);
        character->width   = surface->w;
        character->height  = surface->h;
        /*
         *dbg_str(SDL_INTERFACE_DETAIL, "width =%d height=%d", character->width, character->height);
         */
        SDL_FreeSurface(surface);
    }

    return character;
}

static int __unload_character(Sdl_Render *render, void *character)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Text unload character");
    object_destroy(character);
}

static int __write_character(Sdl_Render *render, int x, int y, void *character)
{
    /*
     *dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render write_character");
     */
    Character *c         = (Character *)character;
    SDL_Rect quad = { x, y, c->width, c->height};

    SDL_RenderCopy(render->sdl_render, ((Sdl_Character *)character)->texture, NULL, &quad );
}

static int __present(Sdl_Render *render)
{
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render present");
    SDL_RenderPresent(render->sdl_render);
}

static void *__create_yuvtexture(Sdl_Render *render, int w, int h)
{
    void *texture;
    /*
     *return SDL_CreateTexture(render->sdl_render, SDL_PIXELFORMAT_IYUV,
     *                         SDL_TEXTUREACCESS_STREAMING, w, h);
     */
    texture =  SDL_CreateTexture(render->sdl_render, SDL_PIXELFORMAT_IYUV,
                             SDL_TEXTUREACCESS_STREAMING, w, h);
    /*
     *SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
     */

    return texture;
}

static int __destroy_texture(Sdl_Render *render, void *texture)
{
    SDL_DestroyTexture(texture);
    return 0;
}


static class_info_entry_t sdl_grath_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Render", "render", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set_window", __set_window, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "draw_image0", __draw_image0, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_FUNC_POINTER, "", "update_window", __update_window, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_FUNC_POINTER, "", "create", __create, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_FUNC_POINTER, "", "destroy", __destroy, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_FUNC_POINTER, "", "set_color", __set_color, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_FUNC_POINTER, "", "set_font", __set_font, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_FUNC_POINTER, "", "clear", __clear, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_FUNC_POINTER, "", "draw_line", __draw_line, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_FUNC_POINTER, "", "draw_rect", __draw_rect, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_FUNC_POINTER, "", "fill_rect", __fill_rect, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_FUNC_POINTER, "", "draw_image", __draw_image, sizeof(void *)}, 
    [17] = {ENTRY_TYPE_FUNC_POINTER, "", "draw_texture", __draw_texture, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_FUNC_POINTER, "", "load_image", __load_image, sizeof(void *)}, 
    [19] = {ENTRY_TYPE_FUNC_POINTER, "", "load_character", __load_character, sizeof(void *)}, 
    [20] = {ENTRY_TYPE_FUNC_POINTER, "", "unload_character", __unload_character, sizeof(void *)}, 
    [21] = {ENTRY_TYPE_FUNC_POINTER, "", "write_character", __write_character, sizeof(void *)}, 
    [22] = {ENTRY_TYPE_FUNC_POINTER, "", "present", __present, sizeof(void *)}, 
    [23] = {ENTRY_TYPE_FUNC_POINTER, "", "create_yuvtexture", __create_yuvtexture, sizeof(void *)}, 
    [24] = {ENTRY_TYPE_FUNC_POINTER, "", "destroy_texture", __destroy_texture, sizeof(void *)}, 
    [25] = {ENTRY_TYPE_STRING, "char", "name", NULL, 0}, 
    [26] = {ENTRY_TYPE_END}, 

    /*
     *[19] = {ENTRY_TYPE_FUNC_POINTER, "", "load_text", __load_text, sizeof(void *)}, 
     *[20] = {ENTRY_TYPE_FUNC_POINTER, "", "unload_text", __unload_text, sizeof(void *)}, 
     *[21] = {ENTRY_TYPE_FUNC_POINTER, "", "write_text", __write_text, sizeof(void *)}, 
     */
};
REGISTER_CLASS("Sdl_Render", sdl_grath_class_info);

void test_ui_sdl_grath()
{
    Sdl_Render *sdl_grath;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Sdl_Render", e = cjson_create_object());{
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str = cjson_print(root);

    sdl_grath = OBJECT_NEW(allocator, Sdl_Render, set_str);

    object_dump(sdl_grath, "Sdl_Render", buf, 2048);
    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render dump: %s", buf);

    object_destroy(sdl_grath);

    free(set_str);

}


