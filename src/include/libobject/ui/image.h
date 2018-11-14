#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <libobject/core/obj.h>
#include <libobject/core/string.h>

typedef struct image_s Image;
struct image_s{
	Obj obj;

	/*normal methods*/
	int (*construct)(Image *image,char *init_str);
	int (*deconstruct)(Image *image);
	int (*set)(Image *image, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods*/
	int (*load_image)(Image *image);

	/*attribs*/
	String *path;
};

#endif
