/*
 * user_config.h
 *
 *  Created on: Jan 9, 2015
 *      Author: yanly
 */

#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_


/********************************************************************************/   //net config
#define CLIENT_PORT    					5030
#define NODE_PORT      					5018
#define SERVER_PORT    					5040
//#define GATEWAY_IPADDR   				"192.168.0.102"
//#define SERVER_IPADDR  				"192.168.0.100"
#define GATEWAY_IPADDR   				"192.168.1.137"
#define SERVER_IPADDR  					"192.168.1.111"
#define MAX_CLIENT_NUM 					32

/********************************************************************************/   //sqlite config
#define	DATABASE_PATH					"/mnt/nfs/node.db"  //数据库存放路径

/********************************************************************************/
//respond_status:-5>>json key error,-6>>json value error
/********************************************************************************/
#define	WARNING_DEVICE_IEEE				"00137A0000017056"	//报警器ieee
#define GLOBAL_OPERATOR_IN_IEEE_NAME	"global operator"
#define WARNING_TIME					"30"    //报警时间
#define WARNING_TYPE					"1"		//报警类型: 1盗窃，2火警，3紧急情况||突发事件,4门铃，5设备故障


extern char local_addr[1025];  //gateway ipaddr

#endif /* USER_CONFIG_H_ */
