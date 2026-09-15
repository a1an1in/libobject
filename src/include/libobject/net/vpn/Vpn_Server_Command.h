#ifndef __VPN_SERVER_COMMAND_H__
#define __VPN_SERVER_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

/*
 * VPN 服务端命令行（xtools vpnserver ...）—— vpn 各节点要连的「信令 + STUN」服务器。
 *
 * 说明：vpn 没有独立服务端，它复用 p2p 的中心服务器（同一进程兼任
 * 信令地址簿/撮合 + STUN 回显，预留 TURN），即 p2p_server_run()。
 * 本命令是它面向运维的正式入口（test_p2p_server 只是测试命令）。
 *
 * 用法：
 *   xtools vpnserver -l 0.0.0.0:12345
 */
typedef struct Vpn_Server_Command_s Vpn_Server_Command;

struct Vpn_Server_Command_s {
	Command parent;

	int (*construct)(Vpn_Server_Command *command, char *init_str);
	int (*deconstruct)(Vpn_Server_Command *command);
	int (*set)(Vpn_Server_Command *command, char *attrib, void *value);
	void *(*get)(Vpn_Server_Command *obj, char *attrib);
	char *(*to_json)(Vpn_Server_Command *obj);

	/*virtual methods reimplement*/
	int (*run_command)(Vpn_Server_Command *command);

	/*attribs*/
	char *listen;   /* --listen host:port（默认 0.0.0.0:12345） */
};

#endif
