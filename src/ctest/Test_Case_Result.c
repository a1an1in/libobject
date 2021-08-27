/**
 * @file Test_Case_Result.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-18
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this test of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this test of conditions and the following disclaimer in the
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
#include <libobject/ctest/Test_Case_Result.h>

static int __construct(Test_Case_Result *result, char *init_str)
{
    allocator_t *allocator = result->obj.allocator;

    result->file = object_new(allocator, "String", NULL);

    return 0;
}

static int __deconstruct(Test_Case_Result *result)
{
    if (result->file != NULL)
        object_destroy(result->file);

    return 0;
}
static class_info_entry_t test_case_result_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Test_Case_Result, construct, __construct),
    Init_Nfunc_Entry(2 , Test_Case_Result, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Test_Case_Result, get, NULL),
    Init_Vfunc_Entry(4 , Test_Case_Result, set, NULL),
    Init_Str___Entry(5 , Test_Case_Result, file, NULL),
    Init_U32___Entry(6 , Test_Case_Result, line, 0),
    Init_S32___Entry(7 , Test_Case_Result, result, 0),
    Init_End___Entry(8 , Test_Case_Result),
};
REGISTER_CLASS("Test_Case_Result", test_case_result_class_info);
