/**
 * @file  
 * @author  jackwu
 * @version 
 * @date  
 */
/* Copyright (c) 2015-2020
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/media/FF_Media_Source.h>

static int __construct(FF_Media_Source *self,char *init_str)
{
    allocator_t *allocator = self->parent.obj.allocator;
    configurator_t * c;

    c = cfg_alloc(allocator); 
    cfg_config_num(c, "/List", "value_size", sizeof(key_value_t)) ;  
    char buf[2048];
    dbg_str(DBG_DETAIL,"FF_Media_Source construct, FF_Media_Source addr:%p",self);
    self->m_list      = NULL;
    self->n_playlists = 0;
    self->stream_type = TYPE_OTHERS; 
    self->file_url    = NULL;
    cfg_destroy(c);
    av_register_all();

    return 0;
}

static void __Destroy(void * element)
{   
    key_value_t * ele = element;
    if ( ele != NULL ) {
        free(ele);
        element = NULL;
    }
}

static int __deconstrcut(FF_Media_Source *self)
{   

    if (self->m_list != NULL ) {
        self->m_list->for_each(self->m_list,__Destroy);
        object_destroy(self->m_list);

    }
    if (self->file_url) {
        object_destroy(self->file_url);
        self->file_url = NULL;
    }
    dbg_str(DBG_DETAIL,"FF_Media_Source deconstruct,FF_Media_Source addr:%p",self);

    return 0;
}

static int __set(FF_Media_Source *self, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        self->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        self->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        self->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        self->deconstruct = value;
    } else if (strcmp(attrib, "set_url") == 0) {
        self->set_url = value;
    } else if (strcmp(attrib, "get_url") == 0) {
        self->get_url = value;
    } else if (strcmp(attrib, "get_list_size") == 0) {
        self->get_list_size = value;
    } else if (strcmp(attrib, "get_url_by_bitrate") == 0) {
        self->get_url_by_bitrate = value;
    } else if (strcmp(attrib, "init") == 0) {
        self->init = value;
    } else if (strcmp(attrib, "info") == 0) {
        self->info = value;
    } else if (strcmp(attrib, "isvalid_bitrate") == 0) {
        self->isvalid_bitrate = value;
    } else if (strcmp(attrib, "url_sanitycheck") == 0) {
        self->url_sanitycheck = value;
    } else if (strcmp(attrib, "get_bitrate_array") == 0) {
        self->get_bitrate_array = value;
    } else if (strcmp(attrib, "get_bitrate_number") == 0) {
        self->get_bitrate_number = value;
    } else {
        dbg_str(DBG_DETAIL,"FF_Media_Source set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(FF_Media_Source *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARNNING,"FF_Media_Source get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static void *__set_url(FF_Media_Source *self, char *url)
{  

    allocator_t * allocator =self->parent.obj.allocator;
    if ( url == NULL ) {
        dbg_str(DBG_ERROR,"media stream assets url==nil");
        return NULL;
    }
    self->file_url = OBJECT_NEW(allocator,String,NULL);
    self->file_url = self->file_url->assign( self->file_url,url);

    return self->file_url->get_cstr(self->file_url);
}

static void *__get_url(FF_Media_Source *self)
{
    //default
    Iterator * iter_cur   = NULL;
    key_value_t * p       = NULL;
    char * url            = NULL;

    if ( self->stream_type == HLS_TYPE_MASTER ) {
        iter_cur = self->m_list->begin(self->m_list);    
        p  = iter_cur->get_vpointer(iter_cur);
        url = p->url;
    } else {
        url = self->file_url->get_cstr(self->file_url);
    }

    return url;
}

static void __init(FF_Media_Source *self,char *url) 
{
    int ret;
    int n_variants;
    int i,len;

    if ( url == NULL ) {
        dbg_str(DBG_WARNNING,"FF_Media_Source::__init() failed. second arguments url == NULL ");
        return ;
    }
    self->set_url(self,url);
    self->url_sanitycheck(self,url);

}

static  char * __get_url_by_bitrate(FF_Media_Source *self,int  bitrate/*bitrate*/) 
{
    Iterator * iter_cur   = NULL;
    Iterator * iter_end   = NULL;
    key_value_t * p       = NULL;

    if ( self->m_list == NULL || self->stream_type != HLS_TYPE_MASTER ) {
        return  NULL;
    }

    iter_cur = self->m_list->begin(self->m_list);
    iter_end = self->m_list->end(self->m_list);

    while ( !iter_end->equal(iter_end,iter_cur) ) {
        p  = iter_cur->get_vpointer(iter_cur);

        if ( p->bandwidth == bitrate ) {
            break;
        }
        iter_cur = iter_cur->next(iter_cur);

    }

    if ( iter_end->equal(iter_end,iter_cur) ) {
        return NULL;
    }

    return p->url;
}

static  int __get_list_size(FF_Media_Source *self) 
{
    return self->n_playlists;
}

static void __info(FF_Media_Source *self) 
{
    Iterator * iter_cur   = NULL;
    Iterator * iter_end   = NULL;
    key_value_t * p       = NULL;
    int   pos             = 0;
    int   len;
    char __buffer[MAX_URL_SIZE];
    char __temp[MAX_URL_SIZE];

    memset(__buffer,0,MAX_URL_SIZE);
    memset(__temp,0,MAX_URL_SIZE);

    if ( self->m_list == NULL || self->stream_type == TYPE_OTHERS) {
        dbg_str(DBG_DETAIL,"stream media file url:%s",self->file_url->get_cstr(self->file_url));
        return  NULL;
    }

    iter_cur = self->m_list->begin(self->m_list);
    iter_end = self->m_list->end(self->m_list);

    while ( !iter_end->equal(iter_end,iter_cur) ) {
        p  = iter_cur->get_vpointer(iter_cur);
        sprintf(__temp,"[%d : %s]\n",p->bandwidth,p->url);
        iter_cur = iter_cur->next(iter_cur);
        len  = strlen(__temp);
        sprintf(__buffer+pos,"%s",__temp);
        pos+=len;
        memset(__temp,0,MAX_URL_SIZE);
    } 

    dbg_str(DBG_DETAIL,"%s",__buffer);
}

static int __get_bitrate_array(FF_Media_Source *self,int * array )
{
    Iterator * iter_cur   = NULL;
    Iterator * iter_end   = NULL;
    key_value_t * p       = NULL;
    int  pos = 0;
    int  i = 0;

    if ( self->m_list == NULL || self->stream_type != HLS_TYPE_MASTER || array == NULL ) {
        dbg_str(DBG_DETAIL,"stream media file is not master m3u8 url:%s",self->file_url->get_cstr(self->file_url));
        return -1;
    }

    iter_cur = self->m_list->begin(self->m_list);
    iter_end = self->m_list->end(self->m_list);

    while ( !iter_end->equal(iter_end,iter_cur) ) {
        p  = iter_cur->get_vpointer(iter_cur);
        *(array+i) = p->bandwidth;
        iter_cur = iter_cur->next(iter_cur);
        i++;
    }

    return 0;
}

static int __isvalid_bitrate(FF_Media_Source *self,int bitrate)
{
    Iterator * iter_cur   = NULL;
    Iterator * iter_end   = NULL;
    key_value_t * p       = NULL;
    int flag              = 0;

    if ( self->m_list == NULL ) {
        dbg_str(DBG_DETAIL,"unvalid bitrate %d failed!",bitrate);
        return  -1;
    }

    iter_cur = self->m_list->begin(self->m_list);
    iter_end = self->m_list->end(self->m_list);

    while ( !iter_end->equal(iter_end,iter_cur) ) {
        p  = iter_cur->get_vpointer(iter_cur);

        if ( p->bandwidth == bitrate ) {
            flag = 1;
            break;
        }
        iter_cur = iter_cur->next(iter_cur);

    }

    return flag;
}

static int __get_bitrate_number(FF_Media_Source *self)
{
    return self->n_playlists;
}

static int __url_sanitycheck(FF_Media_Source *self,char *url)
{   
    int pos ,len;
    int ret,i;
    My_HLSContext         *hls_ctx;
    AVFormatContext       *avfmt_ctx;
    int n_variants;

    if (url == NULL) {
        return -1;    
    }

    allocator_t * allocator = self->parent.obj.allocator;
    String * temp_url = OBJECT_NEW(allocator,String,NULL);

    temp_url->assign(temp_url,url);
    temp_url->trim(temp_url);
    len = temp_url->get_len(temp_url);
    pos = temp_url->find(temp_url, ".m3u8", len-5, 1);


    if ( pos > 0 ) {
        avfmt_ctx = avformat_alloc_context();
        ret = avformat_open_input(&avfmt_ctx,url,NULL,NULL);

        if ( ret < 0 || avformat_find_stream_info(avfmt_ctx,NULL) < 0 ) {
            dbg_str(DBG_WARNNING,"FF_Media_Source::__init() failed. streamfile assets lose ");
            goto end;
        }

        hls_ctx = (struct My_HLSContext *)avfmt_ctx->priv_data;
        if (hls_ctx == NULL ) {
            dbg_str(DBG_WARNNING,"FF_Media_Source::__init() failed. parse playlist error. ");
            goto end;
        }

        n_variants = hls_ctx->n_variants;
        if ( n_variants == 0 ) {
            dbg_str(DBG_WARNNING,"FF_Media_Source::__init() failed. playlist format error. ");
            goto end;
        }
        self->stream_type = n_variants > 1 ? HLS_TYPE_MASTER:HLS_TYPE_NORMAL;

        if ( self->stream_type == HLS_TYPE_NORMAL ) {
            goto end;
        }

        self->m_list = OBJECT_NEW(allocator,Linked_List,NULL);

        for ( i = 0 ; i < n_variants ; i++ ) {
            key_value_t *kv;
            kv = (key_value_t * ) malloc(sizeof(key_value_t));
            memset(kv->url,0,sizeof(kv->url)) ;
            strcpy(kv->url,hls_ctx->playlists[i]->url);
            kv->bandwidth = hls_ctx->variants[i]->bandwidth;

            self->m_list->add_back(self->m_list,kv);
        }

        avformat_close_input(&avfmt_ctx);
    } else {
        self->stream_type = TYPE_OTHERS;
    }

end:
    self->n_playlists = n_variants;
    object_destroy(temp_url);

    return 0;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Media_Source","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_url",__set_url,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_url",__get_url,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_url_by_bitrate",__get_url_by_bitrate,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","init",__init,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_list_size",__get_list_size,sizeof(void *)},
    [10] = {ENTRY_TYPE_FUNC_POINTER,"","info",__info,sizeof(void *)},
    [11] = {ENTRY_TYPE_FUNC_POINTER,"","isvalid_bitrate",__isvalid_bitrate,sizeof(void *)},
    [12] = {ENTRY_TYPE_FUNC_POINTER,"","url_sanitycheck",__url_sanitycheck,sizeof(void *)},
    [13] = {ENTRY_TYPE_FUNC_POINTER,"","get_bitrate_array",__get_bitrate_array,sizeof(void *)},
    [14] = {ENTRY_TYPE_FUNC_POINTER,"","get_bitrate_number",__get_bitrate_number,sizeof(void *)},
    [15] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("FF_Media_Source",concurent_class_info);

static int hls_parse(void *base,int argc,char **argv) 
{
    FF_Media_Source *self = NULL ;
    int bitrate_number = 0;
    int i;
    allocator_t *allocator = allocator_get_default_alloc();

    self = OBJECT_NEW(allocator,FF_Media_Source,NULL);

    self->init(self,"http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8");

    //self->info(self);
    bitrate_number = self->get_bitrate_number(self);
    dbg_str(DBG_DETAIL," current bitnumbers:%d",bitrate_number);

    int * parray = (int *)allocator_mem_alloc(allocator,bitrate_number);
    self->get_bitrate_array(self,parray);
    for (i = 0 ;i < bitrate_number ;i++) {
        dbg_str(DBG_DETAIL," i=%d bitrate :%d \n",i,*(parray+i));    
    }

    allocator_mem_free(allocator,parray);
#if 0
    dbg_str(DBG_DETAIL," xxx :%s \n",self->get_url_by_bitrate(self,200000));
    dbg_str(DBG_DETAIL," xxx :%s \n",self->get_url_by_bitrate(self,311111));
    dbg_str(DBG_DETAIL," xxx :%s \n",self->get_url_by_bitrate(self,484444));
    dbg_str(DBG_DETAIL," xxx :%s \n",self->get_url_by_bitrate(self,737777));
#endif 
    object_destroy(self);
    return 1;

}
REGISTER_TEST_FUNC(hls_parse);
