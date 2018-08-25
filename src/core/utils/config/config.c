#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/json/cjson.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/dbg/debug.h>

static int str_split(char *str, char *delim, char **out, int *cnt) 
{
    int index = 0;
    char *ptr = NULL;

    while((*(out + index) = strtok_r(str, delim, &ptr)) != NULL) {  
        str = NULL;  
        index++;
    }  

    return *cnt = index;
}

static int compute_slash_count(char *path)
{
    int i, len = strlen(path), cnt = 0;

    for (i = 0; i < len; i++) {
        if (path[i] == '/') {
            cnt++;
        }
    }

    return cnt;
}

configurator_t *cfg_alloc(allocator_t *allocator)
{
    configurator_t *c;

    c = (configurator_t *)allocator_mem_alloc(allocator,sizeof(configurator_t));
    if (c == NULL) {
        dbg_str(DBG_ERROR,"configurator_alloc");
        return NULL;
    }

    c->allocator = allocator;
    c->buf_len   = DEFAULT_CONFIG_BUF_LEN;
    c->buf       = allocator_mem_alloc(allocator, c->buf_len);
    memset(c->buf, 0, c->buf_len);

    return c;
}

int cfg_destroy(configurator_t * c)
{
    allocator_mem_free(c->allocator, c->buf);
    allocator_mem_free(c->allocator, c);

    return 0;
}

int cfg_config(configurator_t * c, const char *path, int type, const char *name, void *value) 
{
    allocator_t *allocator = allocator_get_default_alloc();
    cjson_t *root, *object, *item;
    char *buf;  
    char **out;  
    char *p;
    int cnt, j, ret = 0;

    buf = (char *)allocator_mem_alloc(allocator, strlen(path));
    if (buf == NULL) {
        dbg_str(OBJ_WARNNING, "oss set alloc err");
        return -1;
    }
    strcpy(buf, path);

    cnt = compute_slash_count((char *)path);
    out = (char **)allocator_mem_alloc(allocator, sizeof(char *) * cnt);
    if (out == NULL) {
        dbg_str(OBJ_WARNNING, "oss set alloc err");
        allocator_mem_free(allocator, buf);
        return -1;
    }

    str_split(buf, "/", out, &cnt);

    if (strlen(c->buf) != 0) {
        object = cjson_parse(c->buf);
    } else {
        object = cjson_create_object();
    }

    root = object;

    for (j = 0; j < cnt; j++)  {     
        item = cjson_get_object_item(object, out[j]);
        if (item != NULL) {
            object = item;
        } else {
            item = cjson_create_object();
            cjson_add_item_to_object(object, out[j],item);
            object = item;
        }
    } 

    switch(type) {
        case CJSON_FALSE:
            break;
        case CJSON_NUMBER:
            {
                int val = atoi(value);
                cjson_add_number_to_object(item, name, val);
                break;
            }
        case CJSON_STRING:
            {
                cjson_add_string_to_object(item, name, (char *)value);
                break;
            }
        default:
            break;
    }

    p = cjson_print(root);

    if (strlen(p) > c->buf_len) {
        dbg_str(OBJ_WARNNING,"config buffer is too small");
        ret = -1;
        goto err;
    } else {
        strcpy(c->buf, p);
    }

    goto end;

err:
end:
    allocator_mem_free(allocator, buf);
    allocator_mem_free(allocator, out);

    free(p);
    cjson_delete(root);

    return ret;
}

int cfg_config_num(configurator_t * c, const char *path, const char *name, int value) 
{
    allocator_t *allocator = allocator_get_default_alloc();
    cjson_t *root, *object, *item;
    char *buf;  
    char **out;  
    char *p;
    int cnt, j, ret = 0;

    buf = (char *)allocator_mem_alloc(allocator, strlen(path));
    if (buf == NULL) {
        dbg_str(OBJ_WARNNING, "oss set alloc err");
        return -1;
    }
    strcpy(buf, path);

    cnt = compute_slash_count((char *)path);
    out = (char **)allocator_mem_alloc(allocator, sizeof(char *) * cnt);
    if (out == NULL) {
        dbg_str(OBJ_WARNNING, "oss set alloc err");
        allocator_mem_free(allocator, buf);
        return -1;
    }

    str_split(buf, "/", out, &cnt);

    if (strlen(c->buf) != 0) {
        object = cjson_parse(c->buf);
    } else {
        object = cjson_create_object();
    }

    root = object;

    /*find item*/
    for (j = 0; j < cnt; j++)  {     
        item = cjson_get_object_item(object, out[j]);
        if (item != NULL) {
            object = item;
        } else {
            item = cjson_create_object();
            cjson_add_item_to_object(object, out[j],item);
            object = item;
        }
    } 

    /*insert new number item*/
    if (item != NULL){
        cjson_add_number_to_object(item, name, value);
    } else {
        goto end;
    }

    p = cjson_print(root);

    if (strlen(p) > c->buf_len) {
        dbg_str(OBJ_WARNNING,"config buffer is too small");
        ret = -1;
        goto err;
    } else {
        strcpy(c->buf, p);
    }

    goto end;

err:
end:
    allocator_mem_free(allocator, buf);
    allocator_mem_free(allocator, out);

    free(p);
    cjson_delete(root);

    return ret;
}

int cfg_config_str(configurator_t * c, const char *path, const char *name, void *value) 
{
    allocator_t *allocator = allocator_get_default_alloc();
    cjson_t *root, *object, *item;
    char *buf;  
    char **out;  
    char *p;
    int cnt, j, ret = 0;

    buf = (char *)allocator_mem_alloc(allocator, strlen(path));
    if (buf == NULL) {
        dbg_str(OBJ_WARNNING, "oss set alloc err");
        return -1;
    }
    strcpy(buf, path);

    cnt = compute_slash_count((char *)path);
    out = (char **)allocator_mem_alloc(allocator, sizeof(char *) * cnt);
    if (out == NULL) {
        dbg_str(OBJ_WARNNING, "oss set alloc err");
        allocator_mem_free(allocator, buf);
        return -1;
    }

    str_split(buf, "/", out, &cnt);

    if (strlen(c->buf) != 0) {
        object = cjson_parse(c->buf);
    } else {
        object = cjson_create_object();
    }

    root = object;

    for (j = 0; j < cnt; j++)  {     
        item = cjson_get_object_item(object, out[j]);
        if (item != NULL) {
            object = item;
        } else {
            item = cjson_create_object();
            cjson_add_item_to_object(object, out[j],item);
            object = item;
        }
    } 

    if (item != NULL) {
        cjson_add_string_to_object(item, name, (char *)value);
    } else {
        goto end;
    }

    p = cjson_print(root);

    if (strlen(p) > c->buf_len) {
        dbg_str(OBJ_WARNNING,"config buffer is too small");
        ret = -1;
        goto err;
    } else {
        strcpy(c->buf, p);
    }

    goto end;

err:
end:
    allocator_mem_free(allocator, buf);
    allocator_mem_free(allocator, out);

    free(p);
    cjson_delete(root);

    return ret;
}
int test_configurator(void)
{
    configurator_t * c;
    allocator_t *allocator = allocator_get_default_alloc(); 

    c = cfg_alloc(allocator);
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_size", "10") ;  
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "value_size", "25") ;  
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "bucket_size", "15") ;     

    dbg_str(DBG_DETAIL,"config:%s",c->buf);
    cfg_destroy(c);
}
