/***************************************************************************
  Copyright (C),  2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  File name:      net.h
  Description:    
  Author:         jiang   
  Version:        1.0       
  Date:           2014-02-27
  History:        
                  
    1. Date:
       Author:
       Modification:
    2. ...
***************************************************************************/
#ifndef _NET_H
#define _NET_H
#include"sysinit.h"
#include "user_config.h"


//void connect2dvr(void);



//#define GatewayID  88888888    //���صı��


//#define CLIENT_PORT    5030  //��ͻ���ͨ�Ŷ˿�
////#define NODE_PORT      5018  //��ڵ��豸ͨ�Ŷ˿�
//#define NODE_PORT      5002  //��ڵ��豸ͨ�Ŷ˿�
////#define SERVER_PORT    5031  //�������ͨ�Ŷ˿�
//#define SERVER_PORT    5040  //�������ͨ�Ŷ˿�
//
////#define GATEWAY_IPADDR   "192.168.0.102"
////#define SERVER_IPADDR  "192.168.0.100"
//#define GATEWAY_IPADDR   "192.168.1.137"
//#define SERVER_IPADDR  "192.168.1.111"
//
//#define MAX_CLIENT_NUM 16   //���ͻ���������


//#define NODE_NUM 2
#define NODE_NUM 2
#define NODE_LEN 10
#define IE_LEN   20

//#define TERM_NUM 3
#define TERM_NUM 2
#define TERM_LEN 6

#define  INFOLEN 64

typedef struct{
int client_socket;
uint16 client_alive;
uint16 client_count;
char client_buff[MAXBUF];
uint16 client_buff_len;
}client_status;


/***********************************************************************/
//add yanly141229
//typedef enum connect_gw_SocketStatus
//{
//
//};

/***********************************************************************/  //energy
#define  HeartMsg             0x10     //心跳消息
#define  HeartAckMsg          0x20     //心跳响应消息

#define  ConfigMsg            0x11     //配置下发消息
#define  ConfigAckMsg         0x21     //配置下发响应消息

#define  ConfigQueMsg         0x12     //配置查询消息
#define  ConfigQueAckMsg      0x22     //配置查询响应消息


#define  TermQueMsg           0x13     //  表读数查询消息
#define  TermQueAckMsg        0x23     //  表读数查询响应消息

#define  TermNumReportMsg     0x30    //表读数上报消息
#define  TermNumReportAckMsg  0x31    //表读数上报响应消息

#define  ConfigReportMsg      0x41    //配置上报消息
#define  ConfigReportAckMsg   0x51    //配置上报响应消息

#define  TermDataReportMsg    0x42    //表数据上报消息
#define  TermDataReportAckMsg 0x52    //表数据上报响应消息

#define  HeartReportMsg       0x40    //表数据上报消息
#define  HeartReportAckMsg    0x50    //表数据上报响应消息


/***********************************************************************/  //security yanly
#define	SECURITY_CONFIG_MSG							0x70
#define	SECURITY_CONFIG_CHECK_MSG					0x71
#define	SECURITY_SENSOR_CHECK_MSG					0x72
#define	SECURITY_SWITCH_CHECK_MSG					0x73
#define	SECURITY_SWITCH_CONTROL_MSG					0x74
#define	SECURITY_SWITCH_UPLOAD_MSG					0x75
#define	SECURITY_SENSOR_UPLOAD_MSG					0x76
#define	SECURITY_SET_GLOBAL_OPERATOR_MSG			0x77
#define	SECURITY_SET_DEV_OPERATOR_MSG				0x78
#define	SECURITY_UPLOAD_GLOBAL_OPERATOR_MSG			0x79
#define	SECURITY_UPLOAD_DEV_OPERATOR_MSG			0x7a


#define	SECURITY_CONFIG_MSG_RES				    	(SECURITY_CONFIG_MSG + 0x10)
#define	SECURITY_CONFIG_CHECK_MSG_RES				(SECURITY_CONFIG_CHECK_MSG + 0x10)
#define	SECURITY_SENSOR_CHECK_MSG_RES				(SECURITY_SENSOR_CHECK_MSG + 0x10)
#define	SECURITY_SWITCH_CHECK_MSG_RES				(SECURITY_SWITCH_CHECK_MSG + 0x10)
#define	SECURITY_SWITCH_CONTROL_MSG_RES				(SECURITY_SWITCH_CONTROL_MSG + 0x10)
#define	SECURITY_SWITCH_UPLOAD_MSG_RES				(SECURITY_SWITCH_UPLOAD_MSG + 0x10)
#define	SECURITY_SENSOR_UPLOAD_MSG_RES				(SECURITY_SENSOR_UPLOAD_MSG + 0x10)
#define	SECURITY_SET_GLOBAL_OPERATOR_MSG_RES		(SECURITY_SET_GLOBAL_OPERATOR_MSG + 0x10)
#define	SECURITY_SET_DEV_OPERATOR_MSG_RES			(SECURITY_SET_DEV_OPERATOR_MSG + 0x10)
#define	SECURITY_UPLOAD_GLOBAL_OPERATOR_MSG_RES		(SECURITY_UPLOAD_GLOBAL_OPERATOR_MSG + 0x10)
#define	SECURITY_UPLOAD_DEV_OPERATOR_MSG_RES		(SECURITY_UPLOAD_DEV_OPERATOR_MSG + 0x10)
/**********************************************************************************************************/
//hearbeat status
#define	HEARTBEAT_NOT_OK							0
#define	HEARTBEAT_OK								1


#if 1

typedef struct {
		 

	 uint8  TermCode[TERM_LEN];      //智能表的编号
	 uint8  TermType;      //智能表的类型
	 uint8  TermInfo[INFOLEN];      //智能表的描述
	 uint16 TermPeriod;    //智能表的上报周期

     uint16    CenturyValue;
     uint16    YearValue;
     uint16    MouthValue;
     uint16    DayValue;
     uint16    HourValue;
     uint16    MinuteValue;
     uint16    SecondValue;
	 uint32     ReportData;
		
	}term_list;


typedef struct {
	 uint8 IEEE[INFOLEN];
     uint8  EnergyNodeID[INFOLEN];
     term_list term_table[TERM_NUM];
	}node_list;

#endif

#if 0
	typedef struct {


		 uint8  *TermCode;      //智能表的编号
		 uint8  TermType;      //智能表的类型
		 uint8  *TermInfo;      //智能表的描述
		 uint16 TermPeriod;    //智能表的上报周期

	     uint16    CenturyValue;
	     uint16    YearValue;
	     uint16    MouthValue;
	     uint16    DayValue;
	     uint16    HourValue;
	     uint16    MinuteValue;
	     uint16    SecondValue;
		 uint32  ReportData;

		}term_list;


	typedef struct {
		 uint8 *EnergyNodeID;
	     //uint8  EnergyNodeState;
	     term_list term_table[TERM_NUM];
		}node_list;
#endif


/*****************************************************************/
//add yanly141230
extern int connect_host[MAX_CLIENT_NUM];
extern char connect_host_online[MAX_CLIENT_NUM]; //add yan0115

//void ConnectClient();
extern int ConnectClient();
extern void get_local_ipaddr();//add yan150112
//extern void *client_msg_thread(void *p);
extern void *client_type_thread(void *p);

extern void *node_msg_thread(void *p) ;
extern void *server_msg_thread(void *p);
extern void node_msg_process();
extern void server_msg_process();
extern int nodeSocket();
extern int ServerSocket();

extern void *handle_connect(void *argv);
extern void *handle_request(void *argv);
extern void *check_client_heartbeat();
extern void send_msg_to_all_client(char *text, int text_size);
extern void send_msg_to_client(char *text, int text_size, int fd);
extern void set_heart_beat_client(int client_fd);

#endif
