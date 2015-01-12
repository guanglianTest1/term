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
#define GLOBAL_OPERATOR_IN_IEEE_NAME	"global operator"
/********************************************************************************/
//respond_status:-5>>json key error,-6>>json value error
/********************************************************************************/    //报警器ieee
#define	WARNING_DEVICE_IEEE				"00137A0000017056"




extern char local_addr[1025];  //gateway ipaddr

#endif /* USER_CONFIG_H_ */
