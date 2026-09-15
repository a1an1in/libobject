#ifndef __VPN_COMMAND_H__
#define __VPN_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

/*
 * VPN 命令行（xtools vpn ...）—— 解析选项 -> 组装 vpn_cfg_t -> 调 vpn_run()。
 *
 * 用法示例：
 *   xtools vpn -i vpnA -p vpnB -s 119.4.206.14:12345 \
 *              --ip 10.0.0.1/24 -r 10.10.10.0/24
 *   xtools vpn --help
 */
typedef struct Vpn_Command_s Vpn_Command;

struct Vpn_Command_s {
	Command parent;

	int (*construct)(Vpn_Command *command, char *init_str);
	int (*deconstruct)(Vpn_Command *command);
	int (*set)(Vpn_Command *command, char *attrib, void *value);
	void *(*get)(Vpn_Command *obj, char *attrib);
	char *(*to_json)(Vpn_Command *obj);

	/*virtual methods reimplement*/
	int (*run_command)(Vpn_Command *command);

	/*attribs：命令行解析结果（NULL/空=未设置；由 Option 的 action 回填）*/
	char *stun_id;        /* --id       本端 stun id（必填） */
	char *peer_id;        /* --peer     对端 stun id；空=被叫 */
	char *signal;         /* --signal   信令服 host:port（必填） */
	char *stun;           /* --stun     主 STUN host[:port]；空=用信令服自身 */
	char *stun2;          /* --stun2    第二 STUN host[:port]（对称探测） */
	char *local_service;  /* --local-service 本端会话口；空=随机 */
	char *tun_name;       /* --tun      tun 设备名；空=自动 */
	char *local_ip;       /* --ip       本端 tun 地址 a.b.c.d[/len]（必填；省略 /len 默认 /24） */
	char *remote_cidr;    /* --route    对端网段 CIDR；空=不加路由 */
	int   interval_ms;    /* --interval 打洞/保活周期 ms（默认 200） */
};

#endif
