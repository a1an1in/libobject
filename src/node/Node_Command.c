/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-02-18
 */
#include <libobject/node/Node.h>
#include <libobject/core/io/File.h>
#include <libobject/argument/Application.h>
#include "Node_Command.h"

Node_Command *global_node;

static int __read_configs(Node_Command *command)
{
    Command *c = (Command *)command;
    Node *n = command->node;
    allocator_t *allocator = c->parent.allocator;
    Application *app;
    File *file = NULL;
    Vector *options = c->options;
    char path[128] = {0}, content[4096] = {0};
    cjson_t *root = NULL, *item;
    Option *o;
    int ret, i, option_count, set_flag = 1;

    TRY {
        // THROW_IF(n->run_bus_deamon_flag == 1, 0); //deamon 不需要node配置文件
        app = get_global_application();

        snprintf(path, 128, "%s/%s", STR2A(app->root), "node");
        fs_mkdir(path, 0777);
        strcat(path, "/node.json");
        THROW_IF(fs_is_exist(path) == 0, 0);
        dbg_str(DBG_VIP, "node config file:%s", path);

        file = object_new(allocator, "File", NULL);
        file->open(file, path, "r+");
        ret = file->read(file, content, sizeof(content));
        root = cjson_parse(content);

        option_count = options->count(options);
        for (i = 0; i < option_count; i++) {
            options->peek_at(options, i, (void **)&o);
            /* if configed default value and set_flag == 1, then need run option action */
            if (o != NULL && o->set_flag != 1) {
                item = cjson_get_object_item(root, STR2A(o->name) + 2);
                if (item == NULL) continue;
                o->set(o, "value", item->valuestring);
                o->set(o, "set_flag", &set_flag);
                int (*option_action)(void *, void *) = o->action;
                option_action(o, o->opaque);
            }
        }

        item = cjson_get_object_item(root, "node_id");
        THROW_IF(item == NULL, 0);
        strcpy(n->node_id, item->valuestring);
    } CATCH (ret) {} FINALLY {
        object_destroy(file);
        if (root) cjson_delete(root);
    }

    return ret;
}

static int __save_configs(Node_Command *command)
{
    Command *c = (Command *)command;
    Node *n = command->node;
    allocator_t *allocator = c->parent.allocator;
    Application *app;
    File *file = NULL;
    Vector *options = c->options;
    char path[128] = {0}, content[4096] = {0};
    cjson_t *root = NULL, *item;
    Option *o;
    char *out = NULL;
    int ret, i, option_count, set_flag = 1;

    TRY {
        THROW_IF(n->run_bus_deamon_flag == 1, 0);
        app = get_global_application();

        snprintf(path, 128, "%s/%s", STR2A(app->root), "node");
        fs_mkdir(path, 0777);
        strcat(path, "/node.json");
        dbg_str(DBG_DETAIL, "path:%s", path);
        EXEC(fs_mkfile(path, 0777));

        file = object_new(allocator, "File", NULL);
        file->open(file, path, "r+");
        root = cjson_create_object();

        option_count = options->count(options);
        for (i = 0; i < option_count; i++) {
            options->peek_at(options, i, (void **)&o);
            /* if configed default value and set_flag == 1, then need run option action */
            if (o == NULL || o->set_flag != 1) {
                continue;
            }
            cjson_add_string_to_object(root, STR2A(o->name) + 2, STR2A(o->value));
        }

        cjson_add_string_to_object(root, "node_id", n->node_id);
        out = cjson_print(root);
        file->write(file, out, strlen(out));
    } CATCH (ret) {} FINALLY {
        object_destroy(file);
        if (root) cjson_delete(root);
        if (out) free(out);
    }

    return ret;
}

static int __run_command(Node_Command *command)
{
    Node *node = command->node;
    int ret;

    TRY {
        EXEC(__read_configs(command));
        EXEC(node->init(node));
        EXEC(__save_configs(command));
        EXEC(node->loop(node));
    } CATCH (ret) {}

    return ret; 
}

static int __option_host_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_INFO,"option_host_action_callback:%s", STR2A(option->value));
    n->host = STR2A(option->value);

    return 1;
}

static int __option_service_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_INFO,"option_service_action_callback:%s", STR2A(option->value));
    n->service = STR2A(option->value);

    return 1;
}

static int __option_deamon_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;
    Node *n = c->node;

    if ((strcmp(STR2A(option->value), "true") == 0) ||
        (strcmp(STR2A(option->value), "True") == 0) ||
        (strcmp(STR2A(option->value), "T") == 0) ||
        (strcmp(STR2A(option->value), "t") == 0)) {
        n->run_bus_deamon_flag = 1;
    } else {
        n->run_bus_deamon_flag = 0;
    }
    dbg_str(DBG_INFO, "set run_bus_deamon_flag:%d, option:%s", n->run_bus_deamon_flag, STR2A(option->value));
    
    return 1;
}

static int __construct(Node_Command *command, char *init_str)
{
    Command *c = (Command *)command;
    allocator_t *allocator = c->parent.allocator;
    int ret;

    TRY {
        command->node = object_new(allocator, "Node", NULL);

        c->set(c, "/Command/name", "node");
        c->add_option(c, "--host", "", "", "set node center ip address", __option_host_callback, command);
        c->add_option(c, "--service", "-s", "", "set node center port", __option_service_callback, command);
        c->add_option(c, "--deamon", "-d", "false", "run bus deamon", __option_deamon_callback, command);
        command->node->opaque = command;
        global_node = command;
    } CATCH (ret) {}

    return ret;
}

static int __deconstruct(Node_Command *command)
{
    object_destroy(command->node);

    return 0;
}

DEFINE_COMMAND(Node_Command,
    CLASS_OBJ___ENTRY(Command, parent),
    CLASS_NFUNC_ENTRY(construct, __construct),
    CLASS_NFUNC_ENTRY(deconstruct, __deconstruct),
    CLASS_VFUNC_ENTRY(run_command, __run_command)
);

int node_command_get_global_addr(Node_Command **addr)
{
    *addr = global_node;
    dbg_str(DBG_VIP, "node command addr:%p", global_node);

    return 1;
}

int node_command_config(Node_Command *command, char *config)
{
    Command *c = (Command *)command;
    Vector *options = c->options;
    cjson_t *root, *item;
    Option *o;
    int ret, i, option_count, set_flag = 1;

    TRY {
        dbg_str(DBG_VIP, "node_command_config, node command addr:%p, config:%s", command, config);
        root = cjson_parse(config);
        option_count = options->count(options);

        for (i = 0; i < option_count; i++) {
            options->peek_at(options, i, (void **)&o);
            /* if configed default value and set_flag == 1, then need run option action */
            if (o == NULL) continue;
            item = cjson_get_object_item(root, STR2A(o->name) + 2);
            if (item == NULL) continue;
            o->set(o, "value", item->valuestring);
            o->set(o, "set_flag", &set_flag);
            int (*option_action)(void *, void *) = o->action;
            option_action(o, o->opaque);
        }
        EXEC(__save_configs(command));
    } CATCH (ret) {} FINALLY {
        cjson_delete(root);
    }

    return ret;
}

int node_command_upgrade(Node_Command *addr)
{
    int ret;

    TRY {
        dbg_str(DBG_VIP, "node_command_upgrade, node command addr:%p", addr);
    } CATCH (ret) {}

    return ret;
}