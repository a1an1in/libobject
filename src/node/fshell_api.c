/**
 * @file fsh node api.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-12-03
 */

#include <stdlib.h>
#include <signal.h>
#include <libobject/core/utils/string.h>
#include <libobject/scripts/fshell/FShell.h>
#include <libobject/net/bus/bus.h>
#include "libobject/stub/stub.h"
#include <libobject/node/Node.h>

int fsh_node_test(FShell *shell)
{
    bus_t *bus = shell->opaque;
    Node * node= bus->opaque;

    printf("fshell node test.\n");
    printf("fshell node addr:%p\n", node);
}
