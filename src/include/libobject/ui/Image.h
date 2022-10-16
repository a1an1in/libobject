#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/ui/Component.h>

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
    void (*set_size)(Image *image, int width, int height);
    int (*change_size)(Image *image, int width, int height);

    /*attribs*/
    String *path;
    int width;
    int height;
};

#endif
