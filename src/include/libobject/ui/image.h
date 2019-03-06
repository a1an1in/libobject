#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <libobject/core/obj.h>
#include <libobject/core/string.h>
#include <libobject/ui/component.h>

typedef struct image_s Image;
struct image_s{
	Component component;

	/*normal methods*/
	int (*construct)(Image *image,char *init_str);
	int (*deconstruct)(Image *image);
	int (*set)(Image *image, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods*/
	int (*load_image)(Image *image, void *path);
	int (*draw)(Image *image, void *render);
    void (*set_name)(Image *image,void *name);

	/*attribs*/
	String *path;
};

#endif
