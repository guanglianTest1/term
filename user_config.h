/*
 * user_config.h
 *
 *  Created on: Jan 9, 2015
 *      Author: yanly
 */

#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

/********************************************************************************/
//version
#define	SPECIAL_PROJECT_VERSION	"SPECIAL_TERM-V01-01"
/********************************************************************************/ //printf debug
#define SMARTGATEWAY_DEBUG
#include <stdio.h>
#ifdef SMARTGATEWAY_DEBUG
#define GDGL_DEBUG(fmt, args...)	printf("%s(%d)[%s]: " fmt, __FILE__, __LINE__, __func__, ## args)
#endif
/********************************************************************************/   //net config
#define CLIENT_PORT    					5030
#define NODE_PORT      					5018
#define SERVER_PORT    					5031
//#define GATEWAY_IPADDR   				"192.168.0.102"
//#define SERVER_IPADDR  				"192.168.0.100"
//#define GATEWAY_IPADDR   				"192.168.1.137"
//#define SERVER_IPADDR  					"192.168.1.111"
#define SERVER_IPADDR_DEBUG				"127.0.0.1"   //local ipaddr
//#define SERVER_IPADDR_DEBUG				"192.168.1.149" // yang add
#define ALL_IDAWY						"127.0.0.1"   //local ipaddr
#define MAX_CLIENT_NUM 					32

#define RECONNECT_SERVER_TIME			1  //1s
#define RECONNECT_GATAWAY5018_TIME		5

/********************************************************************************/   //sqlite config
//#define	DATABASE_PATH					"/mnt/nfs/node.db"  //数据库存放路径
#define	DATABASE_PATH					"/gl/special/node.db"  //数据库存放路径

/********************************************************************************/



/********************************************************************************/
//#define MAC_D03972940D42
#define MAC_D0397293EA28
#ifdef MAC_D03972940D42
#define	WARNING_DEVICE_IEEE				"00137A0000017056"	//报警器ieee
#endif
#ifdef MAC_D0397293EA28
#define	WARNING_DEVICE_IEEE				"00137A0000017046"	//报警器ieee
#endif

#define GLOBAL_OPERATOR_IN_IEEE_NAME	"global operator"
#define WARNING_TIME					"30"    //报警时间
#define WARNING_TYPE					"1"		//报警类型: 1盗窃，2火警，3紧急情况||突发事件,4门铃，5设备故障

/********************************************************************************/ //USE_CONFIG
#define USE_SERVER_THREAD
//#define PUSH_TO_SERVER_NEW_LOGIC


extern char local_addr[1025];  //gateway ipaddr

// yang add
#define TERM_NUMM 10
#define NODE_NUMM 10
#define TERM_LEN 6
//#define NODE_LEN 4
#define TERM_INTER 3       // time1:60
#define TERM_PEIOD 86400    // time2:set by client
#define HEART_TIME 35       // time3:30  //必须小于40s，取35
#define CLIENT_TIME 1       // time4:1


#endif /* USER_CONFIG_H_ */
