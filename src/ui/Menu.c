/**
 * @file menu.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 
 * @date 2017-03-29
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

#include <libobject/ui/Menu.h>

static int __construct(Menu *menu, char *init_str)
{
	return 0;
}

static int __deconstrcut(Menu *menu)
{
	dbg_str(DBG_INFO, "menu deconstruct, menu addr:%p", menu);

	return 0;
}

static int __set(Menu *menu, char *attrib, void *value)
{
	if (strcmp(attrib, "set") == 0) {
		menu->set = value;
    } else if (strcmp(attrib, "get") == 0) {
		menu->get = value;
	} else if (strcmp(attrib, "construct") == 0) {
		menu->construct = value;
	} else if (strcmp(attrib, "deconstruct") == 0) {
		menu->deconstruct = value;
	} 
    else if (strcmp(attrib, "draw") == 0) {
        menu->draw = value;
    } 
    else {
		dbg_str(DBG_DETAIL, "menu set, not support %s setting", attrib);
	}

	return 0;
}

static void *__get(Menu *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN, "menu get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t menu_class_info[] = {
    Init_Obj___Entry(0 , Component, component),
    Init_Nfunc_Entry(1 , Menu, construct, __construct),
    Init_Nfunc_Entry(2 , Menu, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Menu, set, NULL),
    Init_Vfunc_Entry(4 , Menu, get, NULL),
    Init_Vfunc_Entry(5 , Menu, draw, NULL),
    Init_End___Entry(6 , Menu),
};
REGISTER_CLASS(Menu, menu_class_info);
