/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-02-18
 */

#include "Node_Command.h"

static int __run_command(Node_Command *command)
{
    allocator_t *allocator = command->parent.parent.allocator;
    bus_t *bus = NULL;
    busd_t *busd = NULL;
    int i, ret;

    TRY {
        dbg_str(DBG_VIP, "node command in, command:%p", command);
        THROW_IF(command->host == NULL, -1);
        THROW_IF(command->service == NULL, -1);

        dbg_str(DBG_VIP,"node host:%s, service:%s", command->host, command->service);
        if (command->bus_deamon_flag == 1) {
            busd = busd_create(allocator, command->host,
                               command->service, SERVER_TYPE_INET_TCP);
            THROW_IF(busd == NULL, -1);
            command->busd = busd;

        }

        bus = bus_create(allocator, command->host,
                         command->service, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);
        command->bus = bus;

        
        bus_add_object(bus, &test_object);
        // bus_add_object(bus, &debug_object);

        // pause();
        sleep(10);
        
        // do { sleep(10); } while (command->node_flag != 1);

        dbg_str(DBG_VIP, "node command out");
    } CATCH (ret) {}

    return ret;
}

static int __option_host_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;

    dbg_str(DBG_SUC,"option_host_action_callback:%s", STR2A(option->value));
    c->host = STR2A(option->value);

    return 1;
}

static int __option_service_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;

    dbg_str(DBG_SUC,"option_service_action_callback:%s", STR2A(option->value));
    c->service = STR2A(option->value);

    return 1;
}

static int __option_deamon_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;

    if ((strcmp(STR2A(option->value), "true") == 0) ||
        (strcmp(STR2A(option->value), "True") == 0) ||
        (strcmp(STR2A(option->value), "T") == 0) ||
        (strcmp(STR2A(option->value), "t") == 0)) {
        c->bus_deamon_flag = 1;
    } else {
        c->bus_deamon_flag = 0;
    }
    dbg_str(DBG_SUC, "set bus_deamon_flag:%d, option:%s", c->bus_deamon_flag, STR2A(option->value));
    
    return 1;
}

static int __construct(Node_Command *command, char *init_str)
{
    Command *c = (Command *)command;

    c->set(c, "/Command/name", "node");
    
    c->add_option(c, "--host", "", "", "set node center ip address", __option_host_callback, command);
    c->add_option(c, "--service", "-s", "", "set node center port", __option_service_callback, command);
    c->add_option(c, "--deamon", "-d", "false", "run bus deamon", __option_deamon_callback, command);

    return 0;
}

static int __deconstruct(Node_Command *command)
{
    bus_destroy(command->bus);
    busd_destroy(command->busd);

    return 0;
}

static class_info_entry_t node_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Node_Command, construct, __construct),
    Init_Nfunc_Entry(2, Node_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Node_Command, set, NULL),
    Init_Nfunc_Entry(4, Node_Command, get, NULL),
    Init_Vfunc_Entry(5, Node_Command, run_command, __run_command),
    Init_End___Entry(6, Node_Command),
};
REGISTER_APP_CMD("Node_Command", node_command_class_info);
