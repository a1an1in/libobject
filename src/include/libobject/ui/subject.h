#ifndef __SUBJECT_H__
#define __SUBJECT_H__

#include <libobject/obj.h>
#include <libobject/ui/graph.h>

typedef struct subject_s Subject;
struct subject_s{
	Obj obj;

	/*normal methods*/
	int (*construct)(Subject *subject,char *init_str);
	int (*deconstruct)(Subject *subject);
	int (*set)(Subject *subject, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods*/
	int (*move)(Subject *subject);
	int (*show)(Subject *subject);
	int (*add_x_speed)(Subject *subject, float v);
	int (*add_y_speed)(Subject *subject,float v);
	int (*is_touching)(Subject *me,Subject *subject);

	int x, y, height, width;
	int x_bak, y_bak, height_bak, width_bak;
	float x_speed, y_speed;
	/*
	 *Graph *graph;
	 */
};

#endif
