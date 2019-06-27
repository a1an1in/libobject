/**
 * @file Test_Runner.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-18
 */

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test_Runner.h>
#include <libobject/ctest/Test.h>
#include <libobject/ctest/Test_Case_Result.h>
#include <libobject/core/utils/data_structure/map.h>

static int __construct(Test_Runner *runner, char *init_str)
{
    allocator_t *allocator = runner->obj.allocator;

    runner->result = object_new(allocator, "Test_Result", NULL);

    return 0;
}

static int __deconstruct(Test_Runner *runner)
{
    if (runner->result != NULL)
        object_destroy(runner->result);

    return 0;
}

static int __start(Test_Runner *runner)
{
	map_iterator_t it,next,end;
    class_deamon_t *deamon;
    map_t *map;
    char *key, *test_class;

    dbg_str(DBG_DETAIL,"test_runner running");

    debugger_set_all_businesses_level(debugger_gp, 1, 6);

    deamon = class_deamon_get_global_class_deamon();
    if (deamon == NULL) {
        return -1;
    }

    map = deamon->map;
    map_end(map, &end);

    for(	map_begin(map, &it), map_next(&it, &next); 
            !map_equal(&it, &end);
            it = next, map_next(&it, &next))
    {
        key = map_get_key(&it);

        test_class = strstr(key, "_Test");
        if (test_class != NULL) {
            dbg_str(DBG_SUC,"%s test", key);
            runner->run_test(runner, key);
        }
    }

    /*
     *debugger_set_all_businesses_level(debugger_gp, 1, 9);
     */
}

static int __run_test(Test_Runner *runner, char *test_class_name)
{
    Test *test;
    allocator_t *allocator = runner->obj.allocator;
    class_info_entry_t *entry;
    class_deamon_t *deamon;
    void *is_test_method = NULL;
    int (*test_method)(Test *test);
    Vector *failed_cases, *success_cases;
    Test_Case_Result *case_result;
    int i, ret;

    dbg_str(DBG_DETAIL,"%s", test_class_name);

    failed_cases = runner->result->failed_cases;
    success_cases = runner->result->success_cases;

    deamon = class_deamon_get_global_class_deamon();
    if (deamon == NULL) {
        return -1;
    }

    entry = class_deamon_search_class(deamon, (char *)test_class_name);
    if (entry == NULL) {
        return -1;
    }

    test = object_new(allocator, test_class_name, NULL);

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (    entry[i].type != ENTRY_TYPE_FUNC_POINTER && 
                entry[i].type != ENTRY_TYPE_VFUNC_POINTER)
        {
            continue;
        }

        is_test_method = strstr(entry[i].value_name, "test_");
        if (is_test_method != NULL) {
            test_method = (int (*)(Test *test))entry[i].value;
            test->setup(test);
            ret = test_method(test);
            test->teardown(test);

            case_result = object_new(allocator, "Test_Case_Result", NULL);
            case_result->set(case_result, "result", &test->ret);
            case_result->set(case_result, "file", test->file);
            case_result->set(case_result, "line", &test->line);

            if (ret == 0) { ret = test->ret; }
            if (ret == 1) {
                success_cases->add(success_cases, case_result);
                dbg_str(DBG_SUC,"test %s.%s success", test_class_name, entry[i].value_name);
            } else {
                failed_cases->add(failed_cases, case_result);
                dbg_str(DBG_ERROR,"test %s.%s failed",test_class_name,  entry[i].value_name);
            }
        }
        is_test_method = NULL;
    }   
    object_destroy(test);
}

static class_info_entry_t test_runner_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Test_Runner, construct, __construct),
    Init_Nfunc_Entry(2 , Test_Runner, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Test_Runner, get, NULL),
    Init_Vfunc_Entry(4 , Test_Runner, set, NULL),
    Init_Vfunc_Entry(5 , Test_Runner, start, __start),
    Init_Vfunc_Entry(6 , Test_Runner, run_test, __run_test),
    Init_End___Entry(7 , Test_Runner),
};
REGISTER_CLASS("Test_Runner", test_runner_class_info);

