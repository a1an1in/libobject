#include <libobject/stub/admin.h>

stub_admin_t *stub_admin;

int stub_admin_init_default_instance()
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    int value_type = VALUE_TYPE_ALLOC_POINTER;
    uint8_t trustee_flag = 1;
    List *list;

    TRY {
        stub_admin = allocator_mem_zalloc(allocator, sizeof(stub_admin_t));
        THROW_IF(stub_admin == NULL, -1);

        list = object_new(allocator, "Linked_List", NULL);
        THROW_IF(list == NULL, -1);
        list->set(list, "/List/trustee_flag", &trustee_flag);
        list->set(list, "/List/value_type", &value_type);
        stub_admin->placeholder_list = list;

        list = object_new(allocator, "Linked_List", NULL);
        THROW_IF(list == NULL, -1);
        list->set(list, "/List/trustee_flag", &trustee_flag);
        list->set(list, "/List/value_type", &value_type);
        stub_admin->free_stub_list = list;

        EXEC(stub_admin_add_placeholder(stub_admin, stub_placeholder, stub_placeholder_size));
        dbg_str(DBG_VIP, "xxxxxx stub_placeholder:%p", stub_placeholder);
    } CATCH (ret) {}

    return ret;
}

stub_admin_t *stub_admin_get_default_instance()
{
    return stub_admin;
}

int stub_admin_destroy_default_instance()
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
        object_destroy(stub_admin->free_stub_list);
        object_destroy(stub_admin->placeholder_list);
        allocator_mem_free(allocator, stub_admin->cur);
        allocator_mem_free(allocator, stub_admin);
    } CATCH (ret) {}
    
    return ret;
}

int stub_admin_add_placeholder(stub_admin_t *stub_admin, void *addr, int size)
{
    placeholder_t *holder;
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    List *list;

    TRY {
        holder = allocator_mem_zalloc(allocator, sizeof(placeholder_t));
        THROW_IF(holder == NULL, -1);

        holder->addr = addr;
        holder->size = size;

        list = stub_admin->placeholder_list;
        list->add_back(list, holder);
    } CATCH (ret) {}

    return ret;
}

stub_t *stub_admin_alloc(stub_admin_t *admin)
{
    placeholder_t *cur = admin->cur;
    allocator_t *allocator = allocator_get_default_instance();
    List *list;
    stub_t *stub;
    int ret;

    TRY {
        list = stub_admin->free_stub_list;
        if (list->count(list)) {
            EXEC(list->detach_front(list, &stub));
            EXEC(stub_config_exec_area(stub));
            dbg_str(DBG_DETAIL,"stub_admin_alloc, alloc from free list, stub addr:%p\n", stub);
            return stub;
        }

        if (cur == NULL) {
            list = admin->placeholder_list;
            EXEC(list->detach_front(list, &cur));
        } else if ( (cur->size - cur->index) < sizeof(stub_exec_area_t)) {
            allocator_mem_free(allocator, cur);
            cur = NULL;
            EXEC(list->detach_front(list, &cur));
            THROW_IF(cur == NULL, -1);
        }

        admin->cur = cur;
        stub = (stub_t *)allocator_mem_zalloc(allocator, sizeof(stub_t));
        stub->area = cur->addr + cur->index;
        cur->index += sizeof(stub_exec_area_t);

        dbg_str(DBG_SUC,"stub_admin_alloc, alloc from alloc, stub addr:%p, stub area:%p, addr:%p, index:%d", 
                stub, stub->area, cur->addr, cur->index);
        EXEC(stub_config_exec_area(stub));
        
    } CATCH (ret) {
        stub = NULL;
    }

    return stub;
}

int stub_admin_free(stub_admin_t *admin, stub_t *stub)
{
    List *list = admin->free_stub_list;
    int ret;

    TRY {
        list->add_back(list, stub);
    } CATCH (ret) {}

    return ret;
}