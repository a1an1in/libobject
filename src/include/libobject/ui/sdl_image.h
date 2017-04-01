#ifndef __IMAGE_SDL_H__
#define __IMAGE_SDL_H__

#include <libobject/obj.h>
#include <libobject/string.h>
#include <libobject/ui/image.h>
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
	int (*load_image)(Image *image);

	/*attribs*/
	SDL_Surface* surface;
	SDL_Texture* texture;
	int width;
	int height;
};

#endif
