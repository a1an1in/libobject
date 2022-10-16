#ifndef __IMAGE_SDL_H__
#define __IMAGE_SDL_H__

#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/ui/Image.h>
#include <SDL2/SDL.h>

typedef struct sdl_image_s Sdl_Image;

struct sdl_image_s{
    Image image;

    /*normal methods*/
    int (*construct)(Image *image,char *init_str);
    int (*deconstruct)(Image *image);
    int (*set)(Image *image, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods*/
    int (*load_image)(Image *image, void *path);
    int (*draw)(Image *image, void *render);
    void (*set_size)(Image *image, int width, int height);
    int (*change_size)(Image *image, int width, int height);

    /*attribs*/
    SDL_Surface* surface;
    SDL_Texture* texture;
};

#endif
