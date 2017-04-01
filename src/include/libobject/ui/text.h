#ifndef __TEXT_H__
#define __TEXT_H__

#include <libobject/obj.h>
#include <libobject/list.h>
#include <libobject/string.h>
#include <libobject/ui/graph.h>

typedef struct text_s Text;

typedef struct text_line_s{
	int head_offset;
	char *head;
	int tail_offset;
	char *tail;
	int paragraph_num;
	int line_num_in_paragraph;
	int paragraph_line_num_in_text;
	int line_num;
	int line_lenth;
	String *string;
}text_line_t;

struct text_s{
	Obj obj;

	/*normal methods*/
	int (*construct)(Text *text,char *init_str);
	int (*deconstruct)(Text *text);
	int (*set)(Text *text, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    /*
	 *int (*write_text)(Text *text, char *content, void *font);
     */
    int (*write_text)(Text *text, int start_line,char *str, void *font);
    int (*write_char)(Text *text,int line_num,  int offset, int width, char c,void *font);
    int (*delete_char)(Text *text,int line_num,  int offset, int width, void *font);
	void *(*get_text_line_info)(Text *text, int line_num);
	int (*get_line_count)(Text *text);

	/*attribs*/
	char *content;
    List *line_info;
	int width;
	int last_line_num;
};

#endif
