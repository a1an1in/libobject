#ifndef __FileTransfer_H__
#define __FileTransfer_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct FileTransfer_s FileTransfer;

struct FileTransfer_s {
    Obj parent;

    int (*construct)(FileTransfer *,char *);
    int (*deconstruct)(FileTransfer *);

    /*virtual methods reimplement*/
    int (*set)(FileTransfer *FileTransfer, char *attrib, void *value);
    void *(*get)(FileTransfer *, char *attrib);
    char *(*to_json)(FileTransfer *); 
};

#endif
