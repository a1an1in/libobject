#ifndef __TEXT_SDL_H__
#define __TEXT_SDL_H__

#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/ui/Text.h>
#include <libobject/ui/Sdl_Font.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct sdl_text_s Sdl_Text;

struct sdl_text_s{
    Text text;

    /*normal methods*/
    int (*construct)(Text *text,char *init_str);
    int (*deconstruct)(Text *text);
    int (*set)(Text *text, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods*/
    int (*load_text)(Text *text,void *font);

    /*attribs*/
    SDL_Surface* surface;
    SDL_Texture* texture;
    int width;
    int height;
};

#endif
