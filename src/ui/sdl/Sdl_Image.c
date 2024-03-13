/**
 * @file image.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-12-03
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
#include <stdio.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/Sdl_Image.h>
#include <libobject/ui/Sdl_Window.h>
#include <libobject/ui/Sdl_Render.h>

static int __construct(Image *image, char *init_str)
{
    dbg_str(OBJ_DETAIL, "image construct, image addr:%p", image);
    Sdl_Image *i = (Sdl_Image *)image;

    i->texture = NULL;
    i->surface = NULL;
    image->width = 0;
    image->height = 0;
    return 0;
}

static int __deconstrcut(Image *image)
{
    Sdl_Image *i = (Sdl_Image *)image;
    dbg_str(DBG_DETAIL, "sdl image deconstruct start");

    if (i->surface != NULL) {
        SDL_FreeSurface(i->surface);
    }
    if (i->texture != NULL) {
        SDL_DestroyTexture(i->texture);
    }
    dbg_str(DBG_DETAIL, "sdl image deconstruct end");

    return 0;
}

static int __set(Image *image, char *attrib, void *value)
{
    Sdl_Image *i = (Sdl_Image *)image;

    if (strcmp(attrib, "set") == 0) {
        i->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        i->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        i->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        i->deconstruct = value;
    } else if (strcmp(attrib, "load_image") == 0) {
        i->load_image = value;
    } else if (strcmp(attrib, "draw") == 0) {
        i->draw = value;
    } else if (strcmp(attrib, "set_size") == 0) {
        i->set_size = value;
    } else if (strcmp(attrib, "change_size") == 0) {
        i->change_size = value;
    } else {
        dbg_str(OBJ_WARN, "image set,  \"%s\" setting is not support", attrib);
    }

    return 0;
}

static void * __get(Image *image, char *attrib)
{
    if (strcmp(attrib, "") == 0){ 
    } else {
        dbg_str(OBJ_WARN, "image get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __load_image(Image *image, void *path)
{
    Sdl_Image *i   = (Sdl_Image *)image;

    dbg_str(DBG_SUC, "Sdl_Render load image");

    image->path->assign(image->path, path);
    if (i->surface != NULL) {
        SDL_FreeSurface(i->surface);
    } else {
        i->surface = SDL_LoadBMP(image->path->value);
        if (i->surface == NULL) {
            dbg_str(DBG_ERROR, "SDL_LoadBMP error");
        }
    }

}

static int __draw(Image *image, void *render)
{
    Sdl_Image *si  = (Sdl_Image *)image;
    Sdl_Render *r = (Sdl_Render *)render;

    dbg_str(DBG_SUC, "%s draw", ((Obj *)image)->name);

    if (si->texture == NULL && si->surface != NULL) {
        si->texture = SDL_CreateTextureFromSurface(r->sdl_render, si->surface);
        if (si->texture == NULL) {
            dbg_str(DBG_ERROR, "%s draw, SDL_CreateTextureFromSurface err",
                    ((Obj *)image)->name);
            return -1;
        }

        image->width  = si->surface->w;
        image->height = si->surface->h;
        SDL_FreeSurface(si->surface);
        si->surface = NULL;
        dbg_str(DBG_DETAIL, "convert surface to texture, width=%d, height=%d",
                image->width, image->height);
    }

    if (si->texture != NULL) {
        r->draw_image((Render *)r, 0, 0, image);
    } else {
        si->texture = r->create_yuvtexture(r, image->width, image->height);
    }

    return 0;
}

static int __change_size(Image *image, int width, int height)
{
    Component *c = (Component *)image;
    Render *r = c->render;
    Sdl_Image *si  = (Sdl_Image *)image;
    int ret = 0;

    if (image->width != width || image->height != height) {
        image->width = width;
        image->height = height;

        if (si->texture != NULL) {
            SDL_DestroyTexture(si->texture);
        }
        si->texture = r->create_yuvtexture(r, width, height);
    }

    return ret;
}

static int __set_size(Image *image, int width, int height)
{
    Component *c = (Component *)image;
    Render *r = c->render;
    Sdl_Image *si  = (Sdl_Image *)image;
    int ret = 0;

    if (image->width != width || image->height != height) {
        image->width = width;
        image->height = height;

        if (si->texture != NULL) {
            SDL_DestroyTexture(si->texture);
        }
        si->texture = r->create_yuvtexture(r, width, height);
    }

    return ret;
}

static class_info_entry_t image_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Image", "image", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "load_image", __load_image, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "draw", __draw, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "set_size", NULL, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "change_size", __change_size, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_END}, 

};
REGISTER_CLASS("Sdl_Image", image_class_info);

static int sdl_image()
{
    Window *window;
    Render *r;
    allocator_t *allocator = allocator_get_default_instance();
    char *set_str;
    char buf[2048];
    Image *image;

    dbg_str(DBG_DETAIL, "sdl window draw_background");

    set_str = gen_window_setting_str();

    window = OBJECT_NEW(allocator, Sdl_Window, set_str);
    image  = OBJECT_NEW(allocator, Sdl_Image, "");

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL, "Window dump: %s", buf);

    //???? unload
    image->load_image(image, "../hello_world.bmp");
    image->set_name(image, "image");

    window->add_component((Container *)window, NULL, image);

    window->update_window(window);
    window->event->poll_event(window->event, window);

    object_destroy(window);

    free(set_str);
    return 1;
}

REGISTER_TEST_CMD(sdl_image);


