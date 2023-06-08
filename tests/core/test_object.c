/**
 * @file Composite_Obj_Test.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
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
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/core/utils/registry/registry.h>
#include "Composite_Obj.h"

static char *__to_json_new(Obj *obj) 
{
    dbg_str(DBG_SUC, "new to json");
}

static int test_object_marshal(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Composite_Obj *composite;
    Vector *vector;
    int help = 1, ret, num = 5;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    Obj *obj0 = NULL;
    Obj *obj1 = NULL;
    char *expect = "{\"name\":\"test marshal Composite_Obj\",\"help\":1,\"num\":5,\"vector\":[{\"name\":\"simplest obj1\",\"help\":1}, {\"name\":\"simplest obj2\",\"help\":2}]}";
    String *string;

    TRY {
        obj0 = object_new(allocator, "Simplest_Obj", NULL);
        obj1 = object_new(allocator, "Simplest_Obj", NULL);

        help = 1;
        obj0->set(obj0, "/Simplest_Obj/help", &help);
        obj0->set(obj0, "/Simplest_Obj/name", "simplest obj1");

        help = 2;
        obj1->set(obj1, "/Simplest_Obj/help", &help);
        obj1->set(obj1, "/Simplest_Obj/name", "simplest obj2");


        composite = object_new(allocator, "Composite_Obj", NULL);

        help = 1;
        composite->set(composite, "name", "test marshal Composite_Obj");
        composite->set(composite, "help", &help);
        composite->set(composite, "num", &num);
        vector = composite->vector;

        vector->set(vector, "/Vector/value_type", &value_type);
        vector->set(vector, "/Vector/class_name", "Simplest_Obj");
        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);

        vector->add(vector, obj0);
        vector->add(vector, obj1);

        string = object_new(allocator, "String", NULL);
        string->assign(string, composite->to_json(composite));
        string->replace(string, "\t", "", -1);
        string->replace(string, "\r", "", -1);
        string->replace(string, "\n", "", -1);

        SET_CATCH_STR_PARS(string->get_cstr(string), expect);
        ret = strncmp(string->get_cstr(string), expect, strlen(expect));
        THROW_IF(ret != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "int_number error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(string);
        object_destroy(composite);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_object_marshal);

static int test_object_unmarshal(TEST_ENTRY *entry)
{
    Composite_Obj *composite;
    allocator_t *allocator = allocator_get_default_instance();
    int help = 2;
    String *string;
    int ret;
    char *init_data = "{\"name\":\"test unmarshal Composite_Obj\",\"help\":1,\"num\":5,\"vector\":[{\"name\":\"simplest obj1\",\"help\":1},{\"name\":\"simplest obj2\",\"help\":2}]}";
    char *expect    = "{\"name\":\"test unmarshal Composite_Obj\",\"help\":1,\"num\":5,\"vector\":[{\"name\":\"simplest obj1\",\"help\":1},{\"name\":\"simplest obj2\",\"help\":2}]}";

    TRY {
        composite = object_new(allocator, "Composite_Obj", init_data);

        string = object_new(allocator, "String", composite->to_json(composite));
        //string->assign(string, composite->to_json(composite));
        string->replace(string, "\t", "" , -1);
        string->replace(string, "\r", "" , -1);
        string->replace(string, "\n", "" , -1);
        string->replace(string, ", ", ",", -1);

        SET_CATCH_STR_PARS(string->get_cstr(string), expect);
        ret = strncmp(string->get_cstr(string), expect, strlen(expect));
        THROW_IF(ret != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_object_unmarshal error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(string);
        object_destroy(composite);
    }
}
REGISTER_TEST_FUNC(test_object_unmarshal);

static int test_object_override_virtual_funcs(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Composite_Obj *composite;
    int ret;
    Obj *o;
    void *to_json;

    TRY {
        composite = object_new(allocator, "Composite_Obj", NULL);

        o = (Obj *)composite;
        object_override(o, "to_json", __to_json_new);

        THROW_IF(o->to_json != __to_json_new, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(composite);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_object_override_virtual_funcs);