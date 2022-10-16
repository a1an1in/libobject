#ifndef __URL_TEST_H__
#define __URL_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/net/url/Url.h>

typedef struct url_test_s Url_Test;

struct url_test_s{
	Test parent;

	int (*construct)(Test *,char *init_str);
	int (*deconstruct)(Test *);
	int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
	int (*setup)(Test *);
    void *(*teardown)(Test *);

    void (*test_parse_scheme)(Url_Test *test);
    void (*test_parse_user)(Url_Test *test);
    void (*test_parse_password)(Url_Test *test);
    void (*test_parse_host)(Url_Test *test);
    void (*test_parse_port)(Url_Test *test);
    void (*test_parse_path)(Url_Test *test);
    void (*test_parse_query)(Url_Test *test);
    void (*test_parse_fragment)(Url_Test *test);

    Url *url;
};

#endif
