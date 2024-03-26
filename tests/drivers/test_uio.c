#include <stdio.h>
#include <string.h>
#include <libobject/mockery/mockery.h>
#include <libobject/drivers/uio/Uio.h>

static int test_uio(TEST_ENTRY *entry)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Uio *uio;

    TRY {
        dbg_str(DBG_INFO, "test_uio");
        uio = object_new(allocator, "Uio", NULL);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(uio);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_uio);