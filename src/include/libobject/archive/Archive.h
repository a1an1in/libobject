#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/core/io/File.h>
#include <libobject/core/Vector.h>

typedef struct Archive_s Archive;

typedef struct file_info_s {
    char *file_name;
    uint16_t compression_method;
} file_info_t;

typedef enum wildchard_type_enum {
    SET_INCLUSIVE_WILDCHARD_TYPE = 0,
    CLEAR_INCLUSIVE_WILDCHARD_TYPE,
    SET_EXCLUSIVE_WILDCHARD_TYPE,
    CLEAR_EXCLUSIVE_WILDCHARD_TYPE,
    MAX_WILDCHARD_TYPE,
} wildchard_type_e;

struct Archive_s {
    Obj parent;

    int (*construct)(Archive *, char *);
    int (*deconstruct)(Archive *);
    int (*open)(Archive *a, char *archive_name, char *mode);
	int (*close)(Archive *a);

    /*virtual methods reimplement*/
    int (*set)(Archive *archive, char *attrib, void *value);
    void *(*get)(Archive *, char *attrib);
    char *(*to_json)(Archive *); 

    int (*extract_file)(Archive *a, char *file_name);
    /* extract designated files */
    int (*extract_files)(Archive *a, Vector *files);
    int (*extract)(Archive *a);
    int (*add_file)(Archive *a, char *file_name);
    int (*add_files)(Archive *a, Vector *files);
    int (*add)(Archive *a);
    int (*set_extracting_path)(Archive *a, char *path);
    int (*set_adding_path)(Archive *a, char *path);
    int (*set_wildchard)(Archive *archive, wildchard_type_e, char *wildchard);
    int (*is_filtered_out)(Archive *archive, char *file);
    int (*get_file_infos)(Archive *a, Vector **infos);
    int (*print_file_infos)(Archive *a);

    /* compress and uncompress are used by tar, which may need copress
     * archive firstly, zip don't need those interfaces
     */
    int (*compress)(Archive *c, char *file_in, char *file_out);
    int (*uncompress)(Archive *c, char *file_in, char *file_out);

    /* adding and extracting use different path, then we don't have to swith the path */
    String *extracting_path;
    String *adding_path;
    /* wildchard */
    Vector *inclusive_wildchards;
    Vector *exclusive_wildchards; 
    /* archive file object */
    File *file; 
    /* stored extracting file infos */       
    Vector *extracting_file_infos;
    /* we can specify compression_method for each file by this configure */
    Vector *adding_file_configs;
    /* this is used by string operation */
    String *tmp;
};

#endif
