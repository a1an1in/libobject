/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-02-18
 */

#include <libobject/core/Vector.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/core/utils/string.h>
#include <libobject/concurrent/event_api.h>
#include <libobject/node/Node_Cli_Command.h>

static int __call_fsh_command_action(Node_Cli_Command *command, char *arg1, char *arg2)
{
    Node *node = command->node;
    return TRY_EXEC(node->call_fsh(node, arg1));
}

static int __call_bus_command_action(Node_Cli_Command *command, char *arg1, char *arg2)
{
    Node *node = command->node;
    return TRY_EXEC(node->call_bus(node, arg1, NULL, 0));
}

static int __copy_command_action(Node_Cli_Command *command, char *arg1, char *arg2)
{
    Node *node = command->node;
    return TRY_EXEC(node->fcopy(node, arg1, arg2));
}

static int __list_command_action(Node_Cli_Command *command, char *arg1, char *arg2)
{
    Node *node = command->node;
    allocator_t *allocator = node->parent.allocator;
    char *node_id, *path;
    int value_type = VALUE_TYPE_STRUCT_POINTER;
    uint8_t trustee_flag = 1;
    Vector *list;
    int ret;

    TRY {
        if ((path = strchr(arg1, '@')) != NULL) {
            node_id = arg1;
            *path = '\0';
            path = path + 1;
        }
        list = object_new(allocator, "Vector", NULL);
        EXEC(node->flist(node, node_id, path, list));

        list->for_each(list, fs_file_info_struct_custom_print);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}

static int __lookup_command_action(Node_Cli_Command *command, char *arg1, char *arg2)
{
    Vector *list;
    Node *node = command->node;
    allocator_t *allocator = node->parent.allocator;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "lookup_command_action, arg1:%s", arg1);
        list = object_new(allocator, "Vector", NULL);
        EXEC(node->lookup(node, arg1, list));
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}

static int __call_cmd_command_action(Node_Cli_Command *command, char *arg1, char *arg2)
{
    Node *node = command->node;
    String *str = node->str;
    struct event_base *event_base = event_base_get_default_instance();
    bus_t *bus = node->bus;
    Map *map = bus->req_map;
    bus_req_t *req = NULL;
    char cmd[1024] = {0};
    int ret, cnt;

    TRY {
        dbg_str(DBG_VIP, "call_cmd_command_action, arg1:%s", arg1);
        EXEC(str->reset(str));
        str->assign(str, arg1);
        cnt = str->split(str, "[@{}]", -1);
        THROW_IF(cnt < 2 || cnt > 4, -1);

        EXEC(node->call_cmd(node, arg1));
        while (node->node_exit_flag != 1) usleep(100);

        if (node->node_wait_flag == 1) { //Event Base已经收到退出指令，但是需要node cli先走退出流程.
            dbg_str(DBG_VIP, "call_cmd_command abort_cmd");
            snprintf(cmd, 1024, "%s@abort_cmd()", node->call_cmd_node_id);
            node->call_bus(node, cmd, NULL, 0);

            //需要清楚call cmd req。
            sprintf(cmd, "%s@%s", node->call_cmd_node_id, "call_cmd");
            ret = map->search(map, cmd, (void **)&req);
            EXEC(map->del(map, cmd));
            allocator_mem_free(bus->allocator, req);

            event_base->eb->break_flag = 1;
        }
    } CATCH (ret) {}

    return ret;
}

static int __call_obj_command_action(Node_Cli_Command *command, char *arg1, char *arg2)
{
    Node *node = command->node;
    return TRY_EXEC(node->call_fsh_object_method(node, arg1));
}

static int __mset_command_action(Node_Cli_Command *command, char *arg1, char *arg2)
{
    Node *node = command->node;
    String *str = node->str;
    char *func_name, *arg, *node_id, *addr_name;
    void *par[20] = {0}, *addr;
    int ret, len, cnt, i, offset, capacity;

    TRY {
        dbg_str(DBG_DETAIL, "mset_command_action, arg1:%s", arg1);
        dbg_str(DBG_DETAIL, "mset_command_action, arg2:%s", arg2);
        EXEC(str->reset(str));
        str->assign(str, arg1);
        cnt = str->split(str, "[@{}-]", -1);
        THROW_IF(cnt < 2 || cnt > 4, -1);
        node_id = str->get_splited_cstr(str, 0);
        addr_name = str->get_splited_cstr(str, 1);
        if (cnt == 4) {
            offset = atoi(str->get_splited_cstr(str, 2));
            capacity = atoi(str->get_splited_cstr(str, 3));;
        } else {
            offset = 0, capacity = sizeof(void *);
        }
        dbg_str(DBG_DETAIL, "mset_command_action, cnt:%d, addr:%s, offset:%d, capacity:%d", cnt, addr_name, offset, capacity);
        EXEC(node->mget_addr(node, node_id, addr_name, &addr));
        EXEC(node->mset(node, node_id, addr, offset, capacity, arg2, strlen(arg2)));
    } CATCH (ret) {}

    return ret;
}

static int __mget_command_action(Node_Cli_Command *command, char *arg1, char *arg2)
{
    Node *node = command->node;
    String *str = node->str;
    char *node_id, *addr_name, fmt, unit, unit_size = 1;
    void *addr;
    int ret, i, len, cnt, offset = 0, capacity;
    char buffer[BLOB_BUFFER_MAX_SIZE] = {0};

    TRY {
        dbg_str(DBG_VIP, "mget_command_action, arg1:%s", arg1);
        dbg_str(DBG_VIP, "mget_command_action, arg2:%s", arg2);

        EXEC(str->reset(str));
        str->assign(str, arg1);
        cnt = str->split(str, "[@]", -1);
        THROW_IF(cnt != 2, -1);
        node_id = str->get_splited_cstr(str, 0);
        addr_name = str->get_splited_cstr(str, 1);

        len = strlen(arg2);
        unit = arg2[len - 1];
        THROW_IF(arg2[0] != '/', -1);
        sscanf(arg2, "/%d%c", &cnt, &fmt);    

        if (fmt == 's' || fmt == unit || unit == 'b') { unit_size = 1;}
        else if ((unit == 'h')) {unit_size = 2;}
        else if ((unit == 'w')) {unit_size = 4;}
        else if ((unit == 'g')) {unit_size = 8;}
        else { THROW(-1); }
        len = cnt * unit_size;

        if (addr_name[0] == '#') {
            EXEC(node->mget_addr(node, node_id, addr_name, &addr));
        } else if (addr_name[0] == '0' || addr_name[1] == 'x'|| addr_name[1] == 'X') {
            THROW(-1);
        }

        dbg_str(DBG_VIP, "mget_command_action, len:%d, fmt:%c, unit:%c, unit_size:%d, cnt:%d", len, fmt, unit, unit_size, cnt);
        EXEC(node->mget(node, node_id, addr, offset, len, buffer, &len));

        if (unit == 's') {
            printf("%s\n", buffer);
            THROW(1);
        }
        for (i = 0; i < cnt; i++) {
            addr = buffer + i * unit_size;
            if ((unit == 'b')) {
                printf("0x%02x ", *((char *)addr));
                if (i % 16 == 15) printf("\n");
                if (i == cnt -1 && i % 16 != 15) printf("\n");
            } else if ((unit == 'h')) {
                printf("0x%04x ", *((short *)addr));
                if (i % 16 == 15) printf("\n");
                if (i == cnt -1 && i % 16 != 15) printf("\n");
            } else if ((unit == 'w')) {
                printf("0x%08x ", *((int *)addr));
                if (i % 8 == 7) printf("\n");
                if (i == cnt -1 && i % 8 != 7) printf("\n");
            } else if ((unit == 'g')) {
                printf("0x%016llx ", *((long long *)addr));
                if (i % 4 == 3) printf("\n");
                if (i == cnt -1 && i % 4 != 3) printf("\n");
            } 
        }
    } CATCH (ret) {}

    return ret;
}

struct node_command_s {
    int type;
    char *command_name;
    int (*action)(Node_Cli_Command *command, char *arg1, char *arg2);
} node_cli_command_table[COMMAND_TYPE_MAX] = {
    [COMMAND_TYPE_BUS_CALL] = {COMMAND_TYPE_BUS_CALL, "call_bus", __call_bus_command_action},
    [COMMAND_TYPE_FSH_CALL] = {COMMAND_TYPE_FSH_CALL, "call_fsh", __call_fsh_command_action},
    [COMMAND_TYPE_CMD_CALL] = {COMMAND_TYPE_CMD_CALL, "call_cmd", __call_cmd_command_action},
    [COMMAND_TYPE_OBJ_CALL] = {COMMAND_TYPE_OBJ_CALL, "call_obj", __call_obj_command_action},
    [COMMAND_TYPE_LOOKUP] = {COMMAND_TYPE_LOOKUP, "lookup", __lookup_command_action},
    [COMMAND_TYPE_COPY] = {COMMAND_TYPE_COPY, "fcopy", __copy_command_action},
    [COMMAND_TYPE_LIST] = {COMMAND_TYPE_LIST, "flist", __list_command_action},
    [COMMAND_TYPE_MSET] = {COMMAND_TYPE_MSET, "mset", __mset_command_action},
    [COMMAND_TYPE_MGET] = {COMMAND_TYPE_MGET, "mget", __mget_command_action},
    [COMMAND_TYPE_EXIT] = {COMMAND_TYPE_EXIT, "exit", NULL},
};

static int __run_command(Node_Cli_Command *command)
{
    Node *node = command->node;
    int (*action)(Node_Cli_Command *command, char *arg1, char *arg2);
    int ret;

    TRY {
        EXEC(node->init(node));
        action = node_cli_command_table[command->command_type].action;
        THROW_IF(action == NULL, -1);
        EXEC(action(command, command->arg1, command->arg2));
    } CATCH (ret) {}

    return ret; 
}

static int __option_host_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_INFO,"option_host_action_callback:%s", STR2A(option->value));
    n->host = STR2A(option->value);

    return 1;
}

static int __option_service_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_INFO, "option_service_action_callback:%s", STR2A(option->value));
    n->service = STR2A(option->value);

    return 1;
}

static int __option_disable_node_service_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_INFO, "disable_node_service: %s", STR2A(option->value));
    if ((strcmp(STR2A(option->value), "true") == 0) ||
        (strcmp(STR2A(option->value), "True") == 0) ||
        (strcmp(STR2A(option->value), "T") == 0) ||
        (strcmp(STR2A(option->value), "t") == 0)) {
        n->disable_node_service_flag = 1;
    } else {
        n->disable_node_service_flag = 0;
    }

    return 1;
}

static int __argument_arg0_action_callback(Argument *arg, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    int i, ret;

    dbg_str(DBG_INFO, "argument arg0:%s", STR2A(arg->value));

    for (i = 0; i < COMMAND_TYPE_MAX; i++) {
        if (node_cli_command_table[i].command_name == NULL) continue;
        if (strcmp(STR2A(arg->value), node_cli_command_table[i].command_name) == 0) {
            c->command_type = node_cli_command_table[i].type;
            break;
        }
    }

    return 0;
}

static int __argument_arg1_action_callback(Argument *arg, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;

    dbg_str(DBG_INFO,"argument arg1:%s", STR2A(arg->value));
    c->arg1 = STR2A(arg->value);

    return 0;
}

static int __argument_arg2_action_callback(Argument *arg, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;

    dbg_str(DBG_INFO,"argument arg2:%s", STR2A(arg->value));
    c->arg2 = STR2A(arg->value);

    return 0;
}

static int __construct(Node_Cli_Command *command, char *init_str)
{
    Command *c = (Command *)command;
    allocator_t *allocator = c->parent.allocator;

    command->node = object_new(allocator, "Node", NULL);

    c->set(c, "/Command/name", "node_cli");
    c->add_option(c, "--host", "", "", "set node center ip address.", __option_host_callback, command);
    c->add_option(c, "--service", "-s", "", "set node center port.", __option_service_callback, command);
    c->add_option(c, "--disable-node-service", "", "true", "disable node service for node cli.", 
                  __option_disable_node_service_callback, command);
    c->add_argument(c, "",
                                                      "command type, it's optional if you want to call bus or fshell, \n"
                    "                                ""we can excute cmd by option. now we support call_bus, call_fsh \n"
                    "                                ""and copy commands.",
                    __argument_arg0_action_callback, command);
    c->add_argument(c, "",
                                                      "if arg0 is call_bus or call_fsh, it should set call method here, \n"
                    "                                ""like nodeid@set_loglevel(1,2,3).\n"
                    "                                ""if arg0 is copy command, it should set the source file to copy.\n"
                    "                                ""if arg0 is upgrade command, it should set install package URL.",
                    __argument_arg1_action_callback, command);
    c->add_argument(c, "", "only copy command need arg2, which represent where the file to copy to.\n",
                    __argument_arg2_action_callback, command);
    c->set(c, "/Command/description", "node client command.");

    return 0;
}

static int __deconstruct(Node_Cli_Command *command)
{
    object_destroy(command->node);

    return 0;
}

DEFINE_COMMAND(Node_Cli_Command,
    CLASS_OBJ___ENTRY(Command, parent),
    CLASS_NFUNC_ENTRY(construct, __construct),
    CLASS_NFUNC_ENTRY(deconstruct, __deconstruct),
    CLASS_VFUNC_ENTRY(set, NULL),
    CLASS_VFUNC_ENTRY(get, NULL),
    CLASS_VFUNC_ENTRY(run_command, __run_command)
);
