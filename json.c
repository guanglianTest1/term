/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName:      json.cpp
  Author:        jiang
  Version :      1.0
  Date:          2014-03-19
  Description:   实现封装和解析json相关接口
  History:
      <author>  <time>   <version >   <desc>
***************************************************************************/
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netdb.h>
#include <sqlite3.h>  //add yan150104
#include <time.h>
#include <errno.h>

#include"pthread.h"
#include"net.h"
#include"json.h"
#include"cJSON.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"
#include "appSqlite.h" //add yanly150105
#include"user_config.h"
//#include"public.h"
//extern uint16 client_num;
extern int MsgtermTxId; //���ͳ������ݵ���Ϣ����
//extern int MsgtermRxId; //���ܳ������ݵ���Ϣ����
extern int MsgserverTxId;//���ͷ��������ݵ���Ϣ����

//extern node_list node_table[NODE_NUM];
//extern client_status client_list[MAX_CLIENT_NUM];
//extern uint8 Term_Num[NODE_NUM];
extern uint8 Client_Sn;
extern int Client_Socket;
extern uint8 msgfromflg;

//extern uint8 ConfigReportMsg_flag;
//extern uint8 TermDataReportMsg_flag;

extern int Term_rcvPeriod;
uint8 ConfigReportMsg_sn=0;

#define JSON_PARSE_FAILED	 -1
uint8 Rx_count1=0;
uint8 Rx_count2=0;

char get_systime[INFOLEN]={0};
/*全局布撤防状态,1-全局布防，2-全局撤防
 * */
char global_operator = 2;
//extern uint8 Term_Num[];
//char *GatewayID="8888888";
//extern uint8 server_sn;

//add yanly141229
typedef int (* functionP_t) (cJSON *root, int fd);
//app fun for handle client msg for security
static int msghandle_security_config(cJSON *root, int fd);
static int msghandle_security_config_check(cJSON *root, int fd);
static int msghandle_sensor_state_check(cJSON *root, int fd);
static int msghandle_switch_state_check(cJSON *root, int fd);
static int msghandle_switch_state_ctrl(cJSON *root, int fd);
static int msghandle_set_global_opt(cJSON *root, int fd);
static int msghandle_set_dev_opt(cJSON *root, int fd);
static int msghandle_error(cJSON *root, int fd);
static int msghandle_error(cJSON *root, int fd){return 0;}
//
//send respond to node
static void upload_sensor_change_respond(char *addr, int num, int text_len);



const functionP_t normalTransaction[]=
{
	msghandle_security_config, //0x70
	msghandle_security_config_check, //71
	msghandle_sensor_state_check, //72
	msghandle_switch_state_check, //73
	msghandle_switch_state_ctrl, //74
	msghandle_error, //0x75
	msghandle_error, //76
	msghandle_set_global_opt, //77
	msghandle_set_dev_opt, //78
	msghandle_error, //79
	msghandle_error //7a
};
static int msghandle_set_global_opt(cJSON *root, int fd)
{
//	cJSON *origin = root;
//	cJSON *respond;
//	char *out;
//	int g_operator;
//	char sql[128];
//
//	printf("receive client msg: set global operator\n");
//	////////////////////////////////////////////////////////////////////////////////预取json异常处理
//	if(cJSON_GetObjectItem(origin, "OperatorType") == NULL)
//		return JSON_KEY_ERROR;
//	////////////////////////////////////////////////////////////////////////////////
//	//数据解包
//	g_operator = cJSON_GetObjectItem(origin, "OperatorType")->valueint;
//	if((g_operator >= OPERATER_MAX)||(g_operator == 0))
//		return JSON_VALUE_ERROR;
//
//	//数据插入数据库
//	sprintf(sql,"UPDATE stable SET operator = %d WHERE ieee = '%s'",g_operator,GLOBAL_OPERATOR_IN_IEEE_NAME);
//	sqlite_updata_msg(sql);
//
//	//响应
//	respond  = cJSON_CreateObject();
//	cJSON_AddNumberToObject(respond, "MsgType", SECURITY_SET_GLOBAL_OPERATOR_MSG_RES);
//	cJSON_AddNumberToObject(respond, "Sn", 10);
//	//cJSON_AddNumberToObject(respond, "respond_status", JSON_OK);
//	out = cJSON_PrintUnformatted(respond);
//	send_msg_to_client(out, strlen(out),fd);
//	free(out);
//	cJSON_Delete(respond);
//
//	//转发给其他客户端
//	cJSON_ReplaceItemInObject(origin, "MsgType", cJSON_CreateNumber(SECURITY_UPLOAD_GLOBAL_OPERATOR_MSG));
//	out = cJSON_PrintUnformatted(origin);
//	send_msg_to_all_client(out, strlen(out));
//
//	//转发给服务器
//	//...wait to do in the feature!
//
//	//release
//	free(out);
	//printf("over!\n");
	return	JSON_OK;
}
static int msghandle_set_dev_opt(cJSON *root, int fd)
{
	cJSON *origin = root;
	cJSON *respond;
	char *out;
	int operator;
	char *ieee;
	int num;

	char sql[128];

//	GDGL_DEBUG("set device operator\n");
	////////////////////////////////////////////////////////////////////////////////预取json异常处理
	if((cJSON_GetObjectItem(origin, "OperatorType") == NULL)||
		(cJSON_GetObjectItem(origin, "SensorNum") == NULL)||
		(cJSON_GetObjectItem(origin, "SecurityNodeID") == NULL)
			)
		return JSON_KEY_ERROR;
	//数据解包
	operator = cJSON_GetObjectItem(origin, "OperatorType")->valueint;
	num = cJSON_GetObjectItem(origin, "SensorNum")->valueint;
	ieee = cJSON_GetObjectItem(origin, "SecurityNodeID")->valuestring;
	if((operator >= OPERATER_MAX)||
			(operator ==0)||
			(num>=SENSOR_DEV_MAX_IN_ONE_NODE))
		return JSON_VALUE_ERROR;

	//数据插入数据库
	sprintf(sql,"UPDATE stable SET operator = %d WHERE ieee='%s' and num=%d and type=%d",operator,ieee,num,SECURITY_SENSOR_TYPE);
	if(sqlite_updata_msg(sql) ==0)
		return JSON_INSERT_DATABASE_ERROR;

	//响应
	respond  = cJSON_CreateObject();
	cJSON_AddNumberToObject(respond, "MsgType", SECURITY_SET_DEV_OPERATOR_MSG_RES);
	cJSON_AddNumberToObject(respond, "Sn", 10);
	//cJSON_AddNumberToObject(respond, "respond_status", JSON_OK);
	out = cJSON_PrintUnformatted(respond);
	send_msg_to_client(out, strlen(out),fd);
	free(out);
	cJSON_Delete(respond);

	//转发给其他客户端
	cJSON_ReplaceItemInObject(origin, "MsgType", cJSON_CreateNumber(SECURITY_UPLOAD_DEV_OPERATOR_MSG));
	out = cJSON_PrintUnformatted(origin);
	send_msg_to_all_client(out, strlen(out));

	//转发给服务器
	//...wait to do in the feature!
	#ifdef USE_SERVER_THREAD
	cJSON_ReplaceItemInObject(origin, "MsgType", cJSON_CreateNumber(SERVER_SECURITY_UPLOAD_DEV_OPERATOR_MSG));
	out = cJSON_PrintUnformatted(origin);
	json_msgsnd(MsgserverTxId, SERVER_SECURITY_UPLOAD_DEV_OPERATOR_MSG, out, strlen(out));
	#endif

	//release
	free(out);
	//printf("over!\n");
	return JSON_OK;
}
#if 1


static int msghandle_security_config(cJSON *root, int fd)
{
	cJSON *_root = root;
	cJSON *NodeList_array, *NodeList_item;	int node_cnt;
	cJSON *SubNode_array, *SubNode_item;	int subnode_cnt;
	char *SecurityNodeID;
	char *Nwkaddr;
//	char opt;
	int i,j;
	//
	subsecurityConfig_t *subnod;

//	GDGL_DEBUG("set security config\n");
	////////////////////////////////////////////////////////////////////////////////预取json异常处理
	{
//		if((cJSON_GetObjectItem(_root, "GlobalOpt")) == NULL)
//		{
//			return JSON_KEY_ERROR;
//		}
		if( (NodeList_array = cJSON_GetObjectItem(_root, "NodeList")) == NULL)
		{
			return JSON_KEY_ERROR;
		}
		node_cnt = cJSON_GetArraySize (NodeList_array ); //获取数组的大小
		for(i=0;i<node_cnt;i++)
		{
			if((NodeList_item = cJSON_GetArrayItem(NodeList_array,i)) == NULL)
			{
				return JSON_KEY_ERROR;
			}
			if((cJSON_GetObjectItem(NodeList_item,"SecurityNodeID") == NULL)||
					(cJSON_GetObjectItem(NodeList_item,"Nwkaddr")==NULL) ||
					((SubNode_array = cJSON_GetObjectItem (NodeList_item, "SubNode"))==NULL)
					)
			{
				return JSON_KEY_ERROR;
			}
			subnode_cnt = cJSON_GetArraySize (SubNode_array); //获取数组的大小
			for(j=0;j<subnode_cnt;j++)
			{
				if((SubNode_item = cJSON_GetArrayItem(SubNode_array,j)) == NULL)
				{
					return JSON_KEY_ERROR;
				}
				if((cJSON_GetObjectItem(SubNode_item, "Type")==NULL)||
						(cJSON_GetObjectItem(SubNode_item, "SubType")==NULL)||
						(cJSON_GetObjectItem(SubNode_item, "Num")==NULL)||
						(cJSON_GetObjectItem(SubNode_item,"Info")==NULL)||
						(cJSON_GetObjectItem(SubNode_item, "OperatorType")==NULL)
						)
				{
					return JSON_KEY_ERROR;
				}
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	//opt = cJSON_GetObjectItem(_root, "GlobalOpt")->valueint;
	//转发给其他客户端
	char *out;
	cJSON_ReplaceItemInObject(_root, "MsgType", cJSON_CreateNumber(SECURITY_CONFIG_MSG_RES));
	out = cJSON_PrintUnformatted(_root);
	send_msg_to_all_client(out, strlen(out));
	//转发给服务器
	#ifdef USE_SERVER_THREAD
	cJSON_ReplaceItemInObject(_root, "MsgType", cJSON_CreateNumber(SERVER_SECURITY_CONFIG_MSG));
	out = cJSON_PrintUnformatted(_root);
	json_msgsnd(MsgserverTxId, SERVER_SECURITY_CONFIG_MSG, out, strlen(out));
	#endif

	free(out);
	//配置更新到数据库
	//opt = cJSON_GetObjectItem(_root, "GlobalOpt")->valueint;
	NodeList_array = cJSON_GetObjectItem (_root, "NodeList");
	//sqlite_updata_global_operator(opt);//配置不更新global operator
	if(NodeList_array)
	{
		node_cnt = cJSON_GetArraySize (NodeList_array ); //获取数组的大小
		for(i=0;i<node_cnt;i++)
		{
			NodeList_item = cJSON_GetArrayItem(NodeList_array,i);
		    if(NodeList_item)
		    {
		    	SecurityNodeID = cJSON_GetObjectItem(NodeList_item,"SecurityNodeID")->valuestring;
		    	Nwkaddr = cJSON_GetObjectItem(NodeList_item,"Nwkaddr")->valuestring;
		    	//下发任意透传消息给透传节点触发透传节点能上传透传消息
		    	send_data_to_dev_security(Nwkaddr, "wake up", 8);//add yanly150114
		    	//printf("ieee:%s,addr:%s\n",SecurityNodeID,Nwkaddr);
		    	SubNode_array = cJSON_GetObjectItem (NodeList_item, "SubNode");
		    	subnode_cnt = cJSON_GetArraySize (SubNode_array ); //获取数组的大小
		    	subnod = (subsecurityConfig_t *)malloc(subnode_cnt*sizeof(subsecurityConfig_t));//malloc //
		    	if(subnod ==NULL)
		    	{
		    		GDGL_DEBUG("malloc fail!\n");
		    		return MALLOC_ERROR;
		    	}
		    	for(j=0;j<subnode_cnt;j++)
		    	{
		    		SubNode_item = cJSON_GetArrayItem(SubNode_array,j);
		    		if(SubNode_item)
		    		{
		    			subnod[j].type = cJSON_GetObjectItem(SubNode_item, "Type")->valueint;
		    			subnod[j].subtype = cJSON_GetObjectItem(SubNode_item, "SubType")->valueint;
		    			subnod[j].num = cJSON_GetObjectItem(SubNode_item, "Num")->valueint;
		    			subnod[j].info = cJSON_GetObjectItem(SubNode_item,"Info")->valuestring;
		    			subnod[j].operator = OPERATER_CLOSE;//cJSON_GetObjectItem(SubNode_item, "OperatorType")->valueint;
//		    			printf("type:%d,subtype:%d,num:%d,info:%s,operator:%d\n",subnod[j].type,
//		    					subnod[j].subtype,subnod[j].num,subnod[j].info,subnod[j].operator);

		    		}
		    	}
		    	//handle insert config to database
		    	sqlite_insert_security_config_table(SecurityNodeID, Nwkaddr, subnod, subnode_cnt);
		    	//向节点设备请求开关当前状态
		    	for(j=0;j<subnode_cnt;j++)
		    	{
//		    		printf("request switch status for ieee %s,num",SecurityNodeID);

		    		if(subnod[j].type == SECURITY_SWITCH_TYPE)
		    		{
		    			char buff[12] ={0x01,0x01,0x53,0x74,0x72,0x69,0x6E,0x67,0x00,0x00,0x00,0x01};
		    			buff[10] = subnod[j].num;
		    			buff[11] = subnod[j].operator;
		    			send_data_to_dev_security(Nwkaddr, buff, 12);
//		    			printf("%d,",j);
		    		}
		    	}
		    	//printf(">>over!\n");
		    	//free
		    	free(subnod);
		    }
		}
	}
	return JSON_OK;
}
#else
static void msghandle_security_config(cJSON *root, int fd)
{
	cJSON *_root = root;
	cJSON *NodeList_array, *NodeList_item;	int node_cnt;
	cJSON *SubNode_array, *SubNode_item;	int subnode_cnt;
	char *SecurityNodeID;
	char *Nwkaddr;
	char opt;
	int i,j;
	//
	subsecurityConfig_t *subnod;
	printf("msghandle_security_config\n");
	opt = cJSON_GetObjectItem(_root, "GlobalOpt")->valueint;
	NodeList_array = cJSON_GetObjectItem (_root, "NodeList");
	if(NodeList_array)
	{
		//handle database
		node_cnt = cJSON_GetArraySize (NodeList_array ); //获取数组的大小
		for(i=0;i<node_cnt;i++)
		{
			NodeList_item = cJSON_GetArrayItem(NodeList_array,i);
		    if(NodeList_item)
		    {
		    	SecurityNodeID = cJSON_GetObjectItem(NodeList_item,"SecurityNodeID")->valuestring;
		    	Nwkaddr = cJSON_GetObjectItem(NodeList_item,"Nwkaddr")->valuestring;
		    	//printf("ieee:%s,addr:%s\n",SecurityNodeID,Nwkaddr);
		    	SubNode_array = cJSON_GetObjectItem (NodeList_item, "SubNode");
		    	subnode_cnt = cJSON_GetArraySize (SubNode_array ); //获取数组的大小
		    	subnod = (subsecurityConfig_t *)malloc(subnode_cnt*sizeof(subsecurityConfig_t));//malloc //
		    	if(subnod ==NULL)
		    	{
		    		printf("malloc fail!\n");
		    		return;
		    	}
		    	for(j=0;j<subnode_cnt;j++)
		    	{
		    		SubNode_item = cJSON_GetArrayItem(SubNode_array,j);
		    		if(SubNode_item)
		    		{
		    			subnod[j].type = cJSON_GetObjectItem(SubNode_item, "Type")->valueint;
		    			subnod[j].subtype = cJSON_GetObjectItem(SubNode_item, "SubType")->valueint;
		    			subnod[j].num = cJSON_GetObjectItem(SubNode_item, "Num")->valueint;
		    			subnod[j].info = cJSON_GetObjectItem(SubNode_item,"Info")->valuestring;
		    			subnod[j].operator = cJSON_GetObjectItem(SubNode_item, "OperatorType")->valueint;
		    			printf("type:%d,subtype:%d,num:%d,info:%s,operator:%d\n",subnod[j].type,
		    					subnod[j].subtype,subnod[j].num,subnod[j].info,subnod[j].operator);
		    		}
		    	}
		    	//handle sqlite3
		    	sqlite_insert_security_config_table(SecurityNodeID, Nwkaddr, subnod, subnode_cnt);
		    	sqlite_updata_global_operator(opt);
		    	//freee
		    	free(subnod);
		    }
		}
		//转发给其他客户端
		char *out;
		cJSON_ReplaceItemInObject(_root, "MsgType", cJSON_CreateNumber(SECURITY_CONFIG_MSG_RES));
		out = cJSON_PrintUnformatted(_root);
		send_msg_to_all_client(out, strlen(out));
		//转发给服务器
		//...wait to do in the feature!

		//handle request switch status to dev

		free(out);
	}
}
#endif
#if 0
static void msghandle_security_config_check(cJSON *root, int fd)
{
	char **q_data=NULL;
	int q_row,q_col;int i;
	int index;

	cJSON *sroot;
	cJSON *NodeList_array, *NodeList_item;
    int opt;

    char *out=NULL;
//check database:
    q_data = sqlite_query_security_config(q_data, &q_row, &q_col);
//	if(q_data = (sqlite_query_security_config(q_data, &q_row, &q_col))<0)
//	{
//		return;
//	}
	//printf("data:%s,row:%d,col:%d",q_data,q_row,q_col);

	sroot = cJSON_CreateObject();

	cJSON_AddNumberToObject(sroot, "MsgType", 129);
	cJSON_AddNumberToObject(sroot, "Sn", 10);
	opt = sqlite_query_global_operator();
	cJSON_AddNumberToObject(sroot, "GlobalOpt", opt);
	cJSON_AddItemToObject(sroot, "NodeList", NodeList_item =cJSON_CreateArray());
	index = q_col;
//    for(j=0;j<q_col;j++)
//    {
//        printf(" %s",q_data[index++]);
//
//    }
	for(i=0;i<q_row;i++)
	{
		cJSON_AddItemToArray(NodeList_item, NodeList_array=cJSON_CreateObject());
		cJSON_AddStringToObject(NodeList_array, "SecurityNodeID", q_data[index++]);
		cJSON_AddStringToObject(NodeList_array, "Nwkaddr", q_data[index++]);
		cJSON_AddNumberToObject(NodeList_array, "Type", atoi(q_data[index++]));
		cJSON_AddNumberToObject(NodeList_array, "SubType", atoi(q_data[index++]));
		cJSON_AddNumberToObject(NodeList_array, "Num", atoi(q_data[index++]));
		cJSON_AddStringToObject(NodeList_array, "Info", q_data[index++]);
		cJSON_AddNumberToObject(NodeList_array, "OperatorType", atoi(q_data[index++]));
	}
	out = cJSON_PrintUnformatted(sroot);
	//printf("out=%s\n",out);
//send msg to client:
	send_msg_to_client(out, strlen(out), fd);
//free:
	cJSON_Delete(sroot);
	free(out);
	sqlite_free_query_result(q_data);
}
#else
static int msghandle_security_config_check(cJSON *root, int fd)
{
	char **q_data=NULL;
	char **ieee=NULL;int ieee_row,ieee_col;
	int q_row,q_col;int i,j;
	int index_ieee,index_q;
	char *sql_req_config = "select ieee,nwkaddr,type,subtype,num,info,operator from stable where nwkaddr not null";
	char *sql_req_ieee = "SELECT  DISTINCT ieee,nwkaddr FROM stable where nwkaddr not null";

	cJSON *sroot;
	cJSON *NodeList_array, *NodeList_item;
	cJSON *SubNodeItem, *SubNodeArray;
//    int opt ;
    char *out=NULL;
//    GDGL_DEBUG("security config check\n");
//query database:
    q_data = sqlite_query_msg(&q_row, &q_col, sql_req_config);
    if(q_data ==NULL)
    {
//    	GDGL_DEBUG("query db error\n");
    	sqlite_free_query_result(q_data);
    	return JSON_VALUE_ERROR;
    }
    ieee = sqlite_query_msg(&ieee_row, &ieee_col,sql_req_ieee);
    if(ieee == NULL)
    {
//    	printf("query db error\n");
    	sqlite_free_query_result(ieee);
    	return JSON_VALUE_ERROR;
    }
//    opt = sqlite_query_global_operator();
//    if(opt ==-1)
//    {
//    	printf("query db error\n");
//    	return JSON_VALUE_ERROR;
//    }
//organize json package
    sroot = cJSON_CreateObject();
	cJSON_AddNumberToObject(sroot, "MsgType", 129);
	cJSON_AddNumberToObject(sroot, "Sn", 10);
//	cJSON_AddNumberToObject(sroot, "GlobalOpt", opt);
	cJSON_AddItemToObject(sroot, "NodeList", NodeList_item =cJSON_CreateArray());
	for(i=0;i<ieee_row;i++)
	{
		index_ieee = i*ieee_col+ieee_col;//ieee data的每行头
		cJSON_AddItemToArray(NodeList_item, NodeList_array=cJSON_CreateObject());
		cJSON_AddStringToObject(NodeList_array, "SecurityNodeID", ieee[index_ieee]);
		cJSON_AddStringToObject(NodeList_array, "Nwkaddr", ieee[index_ieee+1]);
		cJSON_AddItemToObject(NodeList_array, "SubNode", SubNodeItem =cJSON_CreateArray());
		for(j=0;j<q_row;j++)
		{
			index_q = j*q_col + q_col;//data的每行头
			if(strcmp(q_data[index_q], ieee[index_ieee])==0)
			{
				cJSON_AddItemToArray(SubNodeItem, SubNodeArray=cJSON_CreateObject());
				cJSON_AddNumberToObject(SubNodeArray, "Type", atoi(q_data[index_q+2]));
				cJSON_AddNumberToObject(SubNodeArray, "SubType", atoi(q_data[index_q+3]));
				cJSON_AddNumberToObject(SubNodeArray, "Num", atoi(q_data[index_q+4]));
				cJSON_AddStringToObject(SubNodeArray, "Info", q_data[index_q+5]);
				cJSON_AddNumberToObject(SubNodeArray, "OperatorType", atoi(q_data[index_q+6]));
			}
			else
			{}
		}
	}
//
	out = cJSON_PrintUnformatted(sroot);
	//printf("out=%s\n",out);
//send msg to client:
	//printf("send msg to all client>>");
	send_msg_to_client(out, strlen(out), fd);
//free:
	cJSON_Delete(sroot);
	free(out);
	sqlite_free_query_result(q_data);
	sqlite_free_query_result(ieee);
	//printf("over!\n");
	return JSON_OK;
}
#endif
static int msghandle_sensor_state_check(cJSON *root, int fd){
	return 0;
}
static int msghandle_switch_state_check(cJSON *root, int fd){
	return 0;
}
static int msghandle_switch_state_ctrl(cJSON *root, int fd)
{

	cJSON *_root = root;
	char *SecurityNodeID;
	char *Nwkaddr;
	int SwitchNum;
	int SwitchStatus;
	char buff[30] ={0x01,0x01,0x53,0x74,0x72,0x69,0x6E,0x67,0x00,0x00,0x00,0x01};
	int b_len;

//	GDGL_DEBUG("set control switch\n");
	//Sn = cJSON_GetObjectItem(_root, "Sn")->valueint;
	////////////////////////////////////////////////////////////////////////////////预取json异常处理
	if((cJSON_GetObjectItem(_root,"SecurityNodeID") == NULL)||
			(cJSON_GetObjectItem(_root,"Nwkaddr") == NULL) ||
			(cJSON_GetObjectItem(_root, "SwitchNum") == NULL) ||
			(cJSON_GetObjectItem(_root, "SwitchStatus") == NULL)
			)
	{
		return JSON_KEY_ERROR;
	}
	SwitchNum = cJSON_GetObjectItem(_root, "SwitchNum")->valueint;
	SwitchStatus = cJSON_GetObjectItem(_root, "SwitchStatus")->valueint;
	if((SwitchStatus >= SWITCH_STATUS_MAX)||
			(SwitchStatus ==0)||
			(SwitchNum>=SWITCH_DEV_MAX_IN_ONE_NODE))
		return JSON_VALUE_ERROR;
	////////////////////////////////////////////////////////////////////////////////
	//响应客户端
	//响应
	cJSON *respond;
	char *out;
	respond  = cJSON_CreateObject();
	cJSON_AddNumberToObject(respond, "MsgType", SECURITY_SET_GLOBAL_OPERATOR_MSG_RES);
	cJSON_AddNumberToObject(respond, "Sn", 10);
	//cJSON_AddNumberToObject(respond, "respond_status", JSON_OK);
	out = cJSON_PrintUnformatted(respond);
	send_msg_to_client(out, strlen(out),fd);
	free(out);
	cJSON_Delete(respond);

	SecurityNodeID = cJSON_GetObjectItem(_root,"SecurityNodeID")->valuestring;
	Nwkaddr = cJSON_GetObjectItem(_root,"Nwkaddr")->valuestring;
//	SwitchNum = cJSON_GetObjectItem(_root, "SwitchNum")->valueint;
//	SwitchStatus = cJSON_GetObjectItem(_root, "SwitchStatus")->valueint;
	buff[10] = SwitchNum;
	buff[11] = SwitchStatus;
	b_len = 12;
	send_data_to_dev_security(Nwkaddr, buff, b_len);
	//printf("respond>>over!\n");
	return JSON_OK;
}
//////////////
static void upload_sensor_change_respond(char *addr, int num, int text_len)
{
	char buff[11] ={0x00,0x02,0x53,0x74,0x72,0x69,0x6E,0x67,0x00,0x00,0x00};
	int b_len = 11;
	buff[10] = num;
//	printf("sensor upload respond to dev...\n");
	send_data_to_dev_security(addr, buff, b_len);
}


int detach_interface_msg_client(char *text,int textlen)
{
	int status;
	cJSON *root;
	int msgtype;

//	if((text[0]>=0x30)&&(text[0]<=0x39))
//	{
//		return DETACH_PRASE_ERROR;
//	}
	root=cJSON_Parse(text);
	if (!root)
	{
		GDGL_DEBUG("****Parse client JSON Error before: %s\n****",cJSON_GetErrorPtr());
		return DETACH_PRASE_ERROR;//need to return immediately 空指针需立即返回
	}
	if(cJSON_GetObjectItem(root, "MsgType") == NULL)
	{
		GDGL_DEBUG("msgtype error!\n");
		cJSON_Delete(root);
		return DETACH_PRASE_ERROR;
	}
	msgtype = cJSON_GetObjectItem(root, "MsgType")->valueint;
	GDGL_DEBUG("recv client msgtype: %d\n", msgtype);
	if ((msgtype >= 0x10) && (msgtype < 0x70))
	{
		if(msgtype ==0x10)    //this is debug add yan141231
		{
			status = DETACH_BELONG_IN_COMMON;
		}
		else
			status = DETACH_BELONG_ENERGY;
	}
	else if((msgtype>= 0x70)&&(msgtype<0x90)) //modify yan
	{
		status = DETACH_BELONG_SECURITY;
	}
	else
	{
		status = DETACH_MSGTYPE_ERROR;
	}
	cJSON_Delete(root);
	return status;
}
int client_msg_handle_security(char *buff, int size, int fd)
{
	int MsgType;
	cJSON *root;
	int cfd = fd;
	cJSON *sroot;
	char *sout;
	int ret=0;

	root=cJSON_Parse(buff);
	if(root)
	{
		MsgType = cJSON_GetObjectItem(root, "MsgType")->valueint;
		ret = normalTransaction[MsgType-112](root, cfd);  //handle message normal
		//respond: when normal set,not request
		if((ret != JSON_OK))
		{
			sroot=cJSON_CreateObject();
			cJSON_AddNumberToObject(sroot,"MsgType",		MsgType+0x10);
			cJSON_AddNumberToObject(sroot,"Sn",				ret);
			//cJSON_AddNumberToObject(sroot,"respond_status",				ret);
			sout=cJSON_PrintUnformatted(sroot);
			send_msg_to_client(sout,strlen(sout),cfd);  //tcp send respond
			cJSON_Delete(sroot);
			free(sout);
		}
	}
    cJSON_Delete(root);
    return ret;
}
int client_msg_handle_in_common(char *buff, int size, int fd)
{
	int MsgType;
	int sn;
	cJSON *root;
	int cfd = fd;
	cJSON *sroot;
	char *sout=NULL;
	time_t timestamp;

	struct sockaddr_in peerAddr;
	int peerLen;
	char ipAddr[INET_ADDRSTRLEN];//保存点分十进制的地址

	root=cJSON_Parse(buff);
	if(root)
	{
		MsgType = cJSON_GetObjectItem(root, "MsgType")->valueint;
		sn=cJSON_GetObjectItem(root, "Sn")->valueint;
		sroot=cJSON_CreateObject();
		switch (MsgType)
		{
			case HeartMsg:
				cJSON_AddNumberToObject(sroot,"MsgType",		MsgType+0x10);
				cJSON_AddNumberToObject(sroot,"Sn",				sn);
				timestamp=time(NULL);
				getpeername(cfd, (struct sockaddr *)&peerAddr, &peerLen);
				GDGL_DEBUG("recv client heartbeat, SOCKET[%d],ADDR[%s],PORT[%d],TIME[%ld]\n",cfd, inet_ntop(AF_INET, &peerAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(peerAddr.sin_port), timestamp);
				sout=cJSON_PrintUnformatted(sroot);
				set_heart_beat_client(fd);//add yan 150115
				send_msg_to_client(sout,strlen(sout),cfd);  //tcp send respond
				free(sout);
				break;
			default:break;
		}
		cJSON_Delete(sroot);
		cJSON_Delete(root);
	}
    return 0;
}
void client_msg_handle_in_msgtype_error(char *buff, int size, int fd)
{
	int MsgType;
	cJSON *root;
	cJSON *sroot;
	char *sout;

	root=cJSON_Parse(buff);
	MsgType = cJSON_GetObjectItem(root, "MsgType")->valueint;

	sroot=cJSON_CreateObject();
	cJSON_AddNumberToObject(sroot,"MsgType",		MsgType);//msgtype值错误回复原来msgtype
	cJSON_AddNumberToObject(sroot,"Sn",				JSON_MSGTYPE_ERROR);
	sout=cJSON_PrintUnformatted(sroot);
	send_msg_to_client(sout,strlen(sout),fd);  //tcp send respond
	cJSON_Delete(sroot);
	free(sout);
	cJSON_Delete(root);
}




/****************************************************************************************/
int detach_5002_message22(char *text, int textlen)
{
	cJSON *root=NULL;
	int msgtype;
    char *ieee=NULL;

	char **data=NULL;
	int col,row;
	char sql[512];//char *sql;not init

	int ret=0;

    root=cJSON_Parse(text);
	if (!root)
    {
	 	return DETACH_PRASE_ERROR;
    }
	msgtype=cJSON_GetObjectItem(root,"msgtype")->valueint;
	GDGL_DEBUG("receive 5018 [size=%d], mestype=%d\n", textlen, msgtype);
	if(msgtype != TERM_MSGTYPE)
	{
		if((msgtype == ANNCE_CALLBACK)||(msgtype == GLOBAL_ARM))
		{
			ret = DETACH_BELONG_IN_COMMON;
		}
		else
		{
			ret = DETACH_MSGTYPE_ERROR;
		}
		cJSON_Delete(root);
		return ret;
	}
	//printf("recevie 5002 callback message 22 >>");
	ieee =cJSON_GetObjectItem(root, "IEEE")->valuestring;
	//printf("ret is %d  %s\n",ret,ieee);
	sprintf(sql,"SELECT ieee FROM stable where ieee ='%s'",ieee);
	//printf("ret is %d  %s\n",ret,sql);
	data = sqlite_query_msg(&row, &col, sql);
	//printf("ret is %d\n",ret);
	if(data == NULL)
	{
		sprintf(sql,"SELECT ieee FROM etable where ieee ='%s'",ieee);
		data = sqlite_query_msg(&row, &col, sql);
		if(data == NULL)
		{
			ret =  DETACH_IEEE_NOT_FOUND_ERROR;
		}
		else
		{
			ret = DETACH_BELONG_ENERGY;
		}
	}
	else
	{
		ret = DETACH_BELONG_SECURITY;
	}
	cJSON_Delete(root);
	sqlite_free_query_result(data);
	return ret;
}
/*
 * security node callback handle
 * */
int parse_json_node_security(char *text,int textlen)
{
	cJSON *root ;
	cJSON *sroot;
	char *sout;

	int msgtype;
    char *IEEE=NULL;
	char *Nwkaddr=NULL;
    char *data =NULL;
	int databuf_len;
	char switchnum,switchstatus,sensornum,sensorstatus;
	int *databuf=NULL;
	int s_len;
	int i;

	char command_num;
	char  cmd_respond_status;
	char sql[128]; //ample size

    root=cJSON_Parse(text);
	if (!root)
    {
	 	return JSON_PARSE_FAILED;
    }
	{
		msgtype=cJSON_GetObjectItem(root,"msgtype")->valueint;
//		printf("node_msgtype=%d\n",msgtype);
		switch(msgtype)
        {
			case TERM_MSGTYPE:
				IEEE =cJSON_GetObjectItem(root, "IEEE")->valuestring;
				//printf("IEEE=%s\n",IEEE);
				Nwkaddr=cJSON_GetObjectItem(root, "Nwkaddr")->valuestring;
				//printf("Nwkaddr=%s\n",Nwkaddr);
				data=cJSON_GetObjectItem(root, "data")->valuestring;
				//printf("data=%s\n",data);
				databuf_len=strlen(data)/2;
				databuf=(int *)malloc(sizeof(int)*40);//databuf=(uint8 *)malloc(sizeof(uint8)*databuf_len);
				//之前没有修改前一直会出现malloc(): memory corruption (fast)的问题；具体触发原因不详，猜测databuf_len传入有问题？！
				if(databuf ==NULL)
				{
					GDGL_DEBUG("malloc fail!\n");
				}
				//printf("databuf= ");
				for(i=0;i<databuf_len;i++)
				{
					sscanf(data+i*2,"%02X",databuf+i);
					//printf("%0X ",databuf[i]);
				}
				//printf("databuf_len=%d \n",databuf_len);
				//串口数据检验
				sroot=cJSON_CreateObject();
				command_num = databuf[0];
				switch (command_num)
				{
					case 0x00://传感器上传的
						GDGL_DEBUG("callback: upload sensor change\n");
						sensornum = databuf[databuf_len-2];
						sensorstatus = databuf[databuf_len-1];
						//响应给节点
						upload_sensor_change_respond(Nwkaddr, sensornum, databuf_len);
						//从数据库中查询布撤防状态，看是否需要上传传感器变化状态 //add yanly150107
						char q_sql[128];int qrow,qcol;char **qopr;int opt;//char qcnt;
//						for(qcnt=0;qcnt<2;qcnt++)
//						{
//							if(qcnt==0) {
//								if(global_operator ==1)
//									qcnt=1;
//							}
//							if(qcnt==1)
//								sprintf(q_sql, "select operator from stable where ieee = '%s' "
//										"and type=%d and num=%d",IEEE,SECURITY_SENSOR_TYPE,sensornum);
//							qopr = sqlite_query_msg(&qrow, &qcol, q_sql);
//							if(qopr!=NULL)
//							{
//								opt = atoi(qopr[qcol]);
//								if(opt==OPERATER_OPEN)
//								{
//								}
//								else
//								{
//									printf("ieee operator is not set \n");
//									qopr = NULL;
//									break;
//								}
//							}
//							else
//							{
//								printf("ieee  operator is null\n");
//								break;
//							}
//						}
						/**********/
						if(global_operator ==1) {

							sprintf(q_sql, "select operator from stable where ieee = '%s' "
									"and type=%d and num=%d",IEEE,SECURITY_SENSOR_TYPE,sensornum);
							qopr = sqlite_query_msg(&qrow, &qcol, q_sql);
							if(qopr!=NULL)
							{
								opt = atoi(qopr[qcol]);
								if(opt==OPERATER_OPEN)
								{
									if(sensorstatus == SENSOR_STATUS_ALARM)
									{//控制报警器
										//http_ctrl_iasWarningDeviceOperation(WARNING_DEVICE_IEEE);
										http_ctrl_start_alarm();
										GDGL_DEBUG("alarm upload\n");
										cJSON_AddNumberToObject(sroot,"MsgType",				118);
										cJSON_AddNumberToObject(sroot,"Sn",						10);
										cJSON_AddStringToObject(sroot,"SecurityNodeID",		IEEE);
										cJSON_AddStringToObject(sroot,"Nwkaddr",				Nwkaddr);
										cJSON_AddNumberToObject(sroot,"SensorNum",				sensornum);
										cJSON_AddNumberToObject(sroot,"SensorStatus",			sensorstatus);
										sout = cJSON_PrintUnformatted(sroot);
										s_len = strlen(sout);
										//发送给所有在线客户端
										send_msg_to_all_client(sout, s_len);
										//发送给云代理服务器  141230
										#ifdef USE_SERVER_THREAD
										cJSON_ReplaceItemInObject(sroot, "MsgType", cJSON_CreateNumber(SERVER_SECURITY_SENSOR_UPLOAD_MSG));
										sout = cJSON_PrintUnformatted(sroot);
										json_msgsnd(MsgserverTxId, SERVER_SECURITY_SENSOR_UPLOAD_MSG, sout, strlen(sout));
										#endif
										free(sout);
									}
								}
							}
							sqlite_free_query_result(qopr);
						}
						/**********/
//						if(qopr!=NULL)
//						{
//							if(sensorstatus == SENSOR_STATUS_ALARM)
//							{//控制报警器
//								http_ctrl_iasWarningDeviceOperation(WARNING_DEVICE_IEEE);
//
//								cJSON_AddNumberToObject(sroot,"MsgType",				118);
//								cJSON_AddNumberToObject(sroot,"Sn",						10);
//								cJSON_AddStringToObject(sroot,"SecurityNodeID",		IEEE);
//								cJSON_AddStringToObject(sroot,"Nwkaddr",				Nwkaddr);
//								cJSON_AddNumberToObject(sroot,"SensorNum",				sensornum);
//								cJSON_AddNumberToObject(sroot,"SensorStatus",			sensorstatus);
//								sout = cJSON_PrintUnformatted(sroot);
//								s_len = strlen(sout);
//								//发送给所有在线客户端
//								send_msg_to_all_client(sout, s_len);
//								//发送给云代理服务器  141230
//								#ifdef USE_SERVER_THREAD
//								cJSON_ReplaceItemInObject(sroot, "MsgType", cJSON_CreateNumber(SERVER_SECURITY_SENSOR_UPLOAD_MSG));
//								sout = cJSON_PrintUnformatted(sroot);
//								json_msgsnd(MsgserverTxId, SERVER_SECURITY_SENSOR_UPLOAD_MSG, sout, strlen(sout));
//								#endif
//								free(sout);
//							}
//							else
//							{
//								printf("not need upload sensor,sensor status is normal\n");//传感器变成正常no need send
//							}
//
//						}
//						sqlite_free_query_result(qopr);
						break;
					case 0x01://控制智能插座的响应
						GDGL_DEBUG("callback: switch control respond\n");
						cmd_respond_status = databuf[9];
						if(cmd_respond_status !=SECURITY_SERIAL_RESPOND_STATUS_SUCCESS)
						{
							GDGL_DEBUG("control switch respond status error\n");
						}
						else
						{
							//更新到数据库中
							switchnum = databuf[databuf_len-2];
							switchstatus = databuf[databuf_len-1];
							sprintf(sql,"UPDATE stable SET operator=%d WHERE ieee ='%s' and type=%d and num=%d"
									,switchstatus,IEEE,SECURITY_SWITCH_TYPE,switchnum);
							//printf("updata sql:%s",sql);
							if(sqlite_updata_msg(sql) == 0)
							{}
							else{
								GDGL_DEBUG("switch change upload\n");
								cJSON_AddNumberToObject(sroot,"MsgType",				SECURITY_SWITCH_UPLOAD_MSG);
								cJSON_AddNumberToObject(sroot,"Sn",						10);
								cJSON_AddStringToObject(sroot,"SecurityNodeID",		IEEE);
								cJSON_AddStringToObject(sroot,"Nwkaddr",				Nwkaddr);
								cJSON_AddNumberToObject(sroot,"SwitchNum",				switchnum);
								cJSON_AddNumberToObject(sroot,"SwitchStatus",			switchstatus);
								sout = cJSON_PrintUnformatted(sroot);
								s_len = strlen(sout);
								//发送给所有在线客户端
								send_msg_to_all_client(sout, s_len);
								//发送给云代理服务器  141230
								#ifdef USE_SERVER_THREAD
								cJSON_ReplaceItemInObject(sroot, "MsgType", cJSON_CreateNumber(SERVER_SECURITY_SWITCH_UPLOAD_MSG));
								sout = cJSON_PrintUnformatted(sroot);
								json_msgsnd(MsgserverTxId, SERVER_SECURITY_SWITCH_UPLOAD_MSG, sout, strlen(sout));
								#endif

								free(sout);
							}
						}
						break;

					case 0x02:
						GDGL_DEBUG("callback: request switch status\n");
						cmd_respond_status = databuf[9];
						if(cmd_respond_status !=0)
						{
							GDGL_DEBUG("serial data format error\n");
						}
						else
						{
							switchnum = databuf[databuf_len-2];
							switchstatus = databuf[databuf_len-1];
							cJSON_AddNumberToObject(sroot,"MsgType",				SECURITY_SWITCH_UPLOAD_MSG);
							cJSON_AddNumberToObject(sroot,"Sn",						10);
							cJSON_AddStringToObject(sroot,"SecurityNodeID",		IEEE);
							cJSON_AddStringToObject(sroot,"Nwkaddr",				Nwkaddr);
							cJSON_AddNumberToObject(sroot,"SwitchNum",				switchnum);
							cJSON_AddNumberToObject(sroot,"SwitchStatus",			switchstatus);
							sout = cJSON_PrintUnformatted(sroot);
							//printf("%s\n",sout);
							s_len = strlen(sout);
							//printf("send message to all client and server!\n");
							//发送给所有在线客户端
							send_msg_to_all_client(sout, s_len);
							//发送给云代理服务器  141230
							//....no do
							//更新到数据库中
							sprintf(sql,"UPDATE stable SET operator=%d WHERE ieee ='%s' and type=%d and num=%d"
									,switchstatus,IEEE,SECURITY_SWITCH_TYPE,switchnum);
							//printf("updata sql:%s",sql);
							sqlite_updata_msg(sql);
							free(sout);
						}
						break;
					default:
						break;
				}
				//printf("over!\n");
				cJSON_Delete(sroot);
				free(databuf);
				break;
			default: break;
        }
		//release
		cJSON_Delete(root);
	}
	return 1;
}
/*
 * 设备节点重新上电产生的callback处理,如果在配置表内唤醒该节点;
 * 全局布防的callback处理,取消全局布撤防状态写进数据库，取消发送给服务器 modify by yanly150512;
 *
 * */
void origin_callback_handle(char *text,int textlen)
{
	cJSON *root ;
	char *ieee;
	char sql[126];
	int msgtype;
	root=cJSON_Parse(text);
	if (!root)
		return;
	if(cJSON_GetObjectItem(root,"msgtype") == NULL)
		return;
	msgtype=cJSON_GetObjectItem(root,"msgtype")->valueint;
	switch(msgtype)
	{
		case ANNCE_CALLBACK:

			if(cJSON_GetObjectItem(root, "IEEE") == NULL)
				return;
			ieee = cJSON_GetObjectItem(root, "IEEE")->valuestring;
			//查询ieee的nwkaddr

			char **data;
			int row,col;
			sprintf(sql,"select distinct nwkaddr from stable where ieee = '%s'",ieee);
			data = sqlite_query_msg(&row, &col, sql);
			if(data!= NULL)
			{
				send_data_to_dev_security(data[col], "wake up",8);
//				printf("nwkaddr:%s\n",data[col]);

			}
			sqlite_free_query_result(data);

			break;

		case GLOBAL_ARM:

			if(cJSON_GetObjectItem(root, "status") == NULL)
				return;
			char *status;
			char *or_s[2] ={"ArmAllZone","DisArm"};
			status = cJSON_GetObjectItem(root, "status")->valuestring;
			if(strcmp(status, or_s[0]) == 0){
				set_global_opertor_status(1);
//				//插入数据库
//				printf("set ArmAllZone\n");
//				sprintf(sql,"UPDATE stable SET operator = %d WHERE ieee = '%s'",1,GLOBAL_OPERATOR_IN_IEEE_NAME);
//				if(sqlite_updata_msg(sql) ==0){
//					exit(1);
//				}
//				i =1;
			}
			else if(strcmp(status, or_s[1]) == 0){
				set_global_opertor_status(2);
//				printf("set disarm\n");
//				sprintf(sql,"UPDATE stable SET operator = %d WHERE ieee = '%s'",2,GLOBAL_OPERATOR_IN_IEEE_NAME);
//				if(sqlite_updata_msg(sql) ==0){
//					exit(1);
//				}
//				i =2;
			}
			else{
				cJSON_Delete(root);
				GDGL_DEBUG("GLOBAL_ARM string error\n");
				return;
			}
			//发送全局布撤防命令到服务器
//#ifdef USE_SERVER_THREAD
//			cJSON *sroot;
//			char *sout;
//			sroot=cJSON_CreateObject();
//			cJSON_AddNumberToObject(sroot,"MsgType",				SERVER_SECURITY_UPLOAD_GLOBAL_OPERATOR_MSG);
//			cJSON_AddNumberToObject(sroot,"Sn",						10);
//			cJSON_AddNumberToObject(sroot,"OperatorType",			i);
//			sout = cJSON_PrintUnformatted(sroot);
//			json_msgsnd(MsgserverTxId, SERVER_SECURITY_SENSOR_UPLOAD_MSG, sout, strlen(sout));
//			cJSON_Delete(sroot);
//			free(sout);
//#endif
			break;
	}
	cJSON_Delete(root);
}
/*
 * 解析服务器回应的消息，打印消息类型
 * add by yan150114
 * */
int parse_received_server_msg(char *text)
{
	cJSON *root;
	int msgtype;
    root=cJSON_Parse(text);
	if (!root)
    {
	 	return JSON_PARSE_FAILED;
    }
	if(cJSON_GetObjectItem(root,"MsgType") ==NULL)
		return JSON_KEY_ERROR;
	msgtype=cJSON_GetObjectItem(root,"MsgType")->valueint;
	GDGL_DEBUG("received server MsgType=%d, [size=%d]\n", msgtype, strlen(text));
	switch(msgtype)
	{
	 case ConfigReportAckMsg:
		 //ConfigReportMsg_flag=1;
		 GDGL_DEBUG("receive ConfigReportAckMsg success \r\n");
		 break;
	 case TermDataReportAckMsg:
		// TermDataReportMsg_flag=1;
		 GDGL_DEBUG("receive TermDataReportAckMsg success \r\n");
		 break;
	 case HeartReportAckMsg:
	 		// TermDataReportMsg_flag=1;
		 GDGL_DEBUG("receive HeartReportAckMsg success \r\n");
	    break;
	 default:break;
	}


	cJSON_Delete(root);
	return 1;
}
/*
 * 解析上传服务器的数据，加上uploadtime字段再上传
 *
 * */
int add_time_field_in_upload_server_msg(char *newbuf, const char *oldbuf)
{
	cJSON *root;
    char *out;
    int nwrite;
	time_t timestamp;

	timestamp = time(NULL);
	root=cJSON_Parse(oldbuf);
	if(!root)
	{
		printf("parse failed\n");
		cJSON_Delete(root);
		return -1;
	}
	cJSON_AddNumberToObject(root, "uploadtime", timestamp);
    if((out = cJSON_PrintUnformatted(root)) == 0 ){
    	printf("print cjson failed\n");
		cJSON_Delete(root);
		return -1;
    }
    nwrite = snprintf(newbuf, 2048, "%s", out);
    nwrite = nwrite+1; // 加上结束符'\0'
    cJSON_Delete(root);
	free(out);
    return nwrite;
}
/*
 * fun:发送数据队列到服务器线程
 * add yan150119
 * */
void json_msgsnd(int id, int type, char *buff, int buff_size) //add yan150119
{
	msgform msend;
	int res;
	char new_buff[MAX_MSG_BUF]={0};
	int new_write;
//	GDGL_DEBUG("send msgsnd to server pthread, MSGTYPE[%d]\n", type);
	new_write = add_time_field_in_upload_server_msg(new_buff, buff);
	if(new_write > MAX_MSG_BUF){
		printf("msgsnd error: send size > MAX_MSG_BUF !\n");
		return;
	}
	msend.mtype = type;
	memcpy(msend.mtext, new_buff, new_write);
//	res = msgsnd(id, &msend, new_write, 0);  //flag为0：如果消息队列已满，会一直阻塞等待，该进程会停止在这里。
	res = msgsnd(id, &msend, new_write, IPC_NOWAIT);
	GDGL_DEBUG("send msgsnd to server pthread, MSGTYPE[%d]\n", type);
	if(res <0)
	{
		printf("msgsnd failed, errno=%d\n",errno);
		exit(1);
	}
	memset(msend.mtext,0,sizeof(msend.mtext));
	buff=NULL;

}



/*******************yang  add**************************************************************/
/******************************************************************************************/
/******************************************************************************************/

void json_msgsndclient(int id, int type, char *buff, int buff_size) //add yang
{
	msgform msend;

	if(buff_size > MAX_MSG_BUF){
		printf("msgsnd error: send size > MAX_MSG_BUF !\n");
		return;
	}
	msend.mtype = type;
	memcpy(msend.mtext, buff, buff_size);
	msgsnd(id, &msend, buff_size, 0);
	memset(msend.mtext,0,sizeof(msend.mtext));
	buff=NULL;
}

int client_msg_handle_energy(char *text,int textlen,int tmp_socket)
{
	int MsgType;
	cJSON *root;
	int cfd = tmp_socket;
	cJSON *sroot;
	char *sout;
	int ret;

	root=cJSON_Parse(text);
	if(root)
	{
		MsgType = cJSON_GetObjectItem(root, "MsgType")->valueint;
		//ret = normalTransaction[MsgType-112](root, cfd);  //handle message normal
		ret=parse_json_client(MsgType,root,cfd);
		//respond: when normal set,not request
		if((ret != JSON_OK))
		{
			sroot=cJSON_CreateObject();
			cJSON_AddNumberToObject(sroot,"MsgType",MsgType+0x10);
			cJSON_AddNumberToObject(sroot,"Sn",	ret);
			//cJSON_AddNumberToObject(sroot,"respond_status",				ret);
			sout=cJSON_PrintUnformatted(sroot);
			send_msg_to_client(sout,strlen(sout),cfd);  //tcp send respond
			cJSON_Delete(sroot);
			free(sout);
		}
	}
    cJSON_Delete(root);
    return ret;
}


int msghandle_energy_config(cJSON *root, int fd)
 //int msghandle_security_config(cJSON *root, int fd)
{
	cJSON *_root = root;
	cJSON *NodeList_array, *NodeList_item;	int node_cnt;
	cJSON *SubNode_array, *SubNode_item;	int subnode_cnt;
	char *SecurityNodeID;
	char *Nwkaddr;
//	char opt;
	int i,j;
	//
	term_list *subnod;

	GDGL_DEBUG("set energy config\n");
	////////////////////////////////////////////////////////////////////////////////预取json异常处理
	{
//		if((cJSON_GetObjectItem(_root, "GlobalOpt")) == NULL)
//		{
//			return JSON_KEY_ERROR;
//		}
		if( (NodeList_array = cJSON_GetObjectItem(_root, "NodeList")) == NULL)
		{
			return JSON_KEY_ERROR;
		}
		node_cnt = cJSON_GetArraySize (NodeList_array ); //获取数组的大小
		for(i=0;i<node_cnt;i++)
		{
			if((NodeList_item = cJSON_GetArrayItem(NodeList_array,i)) == NULL)
			{
				return JSON_KEY_ERROR;
			}
			if((cJSON_GetObjectItem(NodeList_item,"IEEE") == NULL)||
					(cJSON_GetObjectItem(NodeList_item,"Nwkaddr")==NULL) ||
					((SubNode_array = cJSON_GetObjectItem (NodeList_item, "TermList"))==NULL)
					)
			{
				return JSON_KEY_ERROR;
			}
			subnode_cnt = cJSON_GetArraySize (SubNode_array); //获取数组的大小
			for(j=0;j<subnode_cnt;j++)
			{
				if((SubNode_item = cJSON_GetArrayItem(SubNode_array,j)) == NULL)
				{
					return JSON_KEY_ERROR;
				}
				if((cJSON_GetObjectItem(SubNode_item, "TermCode")==NULL)||
						(cJSON_GetObjectItem(SubNode_item, "TermType")==NULL)||
						(cJSON_GetObjectItem(SubNode_item, "TermInfo")==NULL)||
						(cJSON_GetObjectItem(SubNode_item,"TermPeriod")==NULL))
				{
					return JSON_KEY_ERROR;
				}
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	//opt = cJSON_GetObjectItem(_root, "GlobalOpt")->valueint;
	//转发给其他客户端
	char *out;
	cJSON_ReplaceItemInObject(_root, "MsgType", cJSON_CreateNumber(ConfigAckMsg));
	out = cJSON_PrintUnformatted(_root);
	send_msg_to_all_client(out, strlen(out));
	//转发给服务器
	#ifdef USE_SERVER_THREAD
	cJSON_ReplaceItemInObject(_root, "MsgType", cJSON_CreateNumber(ConfigReportMsg));
	out = cJSON_PrintUnformatted(_root);
	json_msgsnd(MsgserverTxId, ConfigReportMsg, out, strlen(out));
	#endif

	free(out);
	//配置更新到数据库
	//opt = cJSON_GetObjectItem(_root, "GlobalOpt")->valueint;
	sqlite_delete_allmsg_from_etable();
	NodeList_array = cJSON_GetObjectItem (_root, "NodeList");
	//sqlite_updata_global_operator(opt);//配置不更新global operator
	if(NodeList_array)
	{
		node_cnt = cJSON_GetArraySize (NodeList_array ); //获取数组的大小
		for(i=0;i<node_cnt;i++)
		{
			NodeList_item = cJSON_GetArrayItem(NodeList_array,i);
		    if(NodeList_item)
		    {
		    	SecurityNodeID = cJSON_GetObjectItem(NodeList_item,"IEEE")->valuestring;
		    	Nwkaddr = cJSON_GetObjectItem(NodeList_item,"Nwkaddr")->valuestring;
		    	//下发任意透传消息给透传节点触发透传节点能上传透传消息
		    	send_data_to_dev_security(Nwkaddr, "wake up", 8);//add yanly150114
		    	//printf("ieee:%s,addr:%s\n",SecurityNodeID,Nwkaddr);
		    	SubNode_array = cJSON_GetObjectItem (NodeList_item, "TermList");
		    	subnode_cnt = cJSON_GetArraySize (SubNode_array ); //获取数组的大小
//		    	printf("subnode_cnt=%d==============================\n",subnode_cnt);
		    	subnod = (term_list *)malloc(subnode_cnt*sizeof(term_list));//malloc //
		    	if(subnod ==NULL)
		    	{
		    		GDGL_DEBUG("malloc fail!\n");
		    		return MALLOC_ERROR;
		    	}
		    	for(j=0;j<subnode_cnt;j++)
		    	{
		    		SubNode_item = cJSON_GetArrayItem(SubNode_array,j);
		    		if(SubNode_item)
		    		{
		    			subnod[j].TermCode = cJSON_GetObjectItem(SubNode_item, "TermCode")->valuestring;
		    			subnod[j].TermType = cJSON_GetObjectItem(SubNode_item, "TermType")->valueint;
		    			subnod[j].TermInfo= cJSON_GetObjectItem(SubNode_item, "TermInfo")->valuestring;
		    			subnod[j].TermPeriod= cJSON_GetObjectItem(SubNode_item,"TermPeriod")->valueint;
		    			//subnod[j].operator = OPERATER_CLOSE;//cJSON_GetObjectItem(SubNode_item, "OperatorType")->valueint;
//		    			printf("type:%d,subtype:%d,num:%d,info:%s,operator:%d\n",subnod[j].type,
//		    					subnod[j].subtype,subnod[j].num,subnod[j].info,subnod[j].operator);

                         if(subnod[0].TermPeriod>0)
		    			 {
							Term_rcvPeriod=60*subnod[j].TermPeriod;
							//printf("Term_rcvPeriod=%d\n",Term_rcvPeriod);
						 }
						else
						 {Term_rcvPeriod=TERM_PEIOD;
							 //printf("Term_rcvPeriod=%d\n",Term_rcvPeriod);
						 }
						ModTimer(Term_rcvPeriod,time2);

		    		}
		    	}
		    	//handle insert config to database
		    	if(subnode_cnt >0)  //add yanly150521
		    	{
		    		sqlite_insert_energy_config_table(SecurityNodeID, Nwkaddr, subnod, subnode_cnt);//配置更新到数据库
		    	}
		    	if(subnod!=NULL)
		    	{
		    		free(subnod); //malloc must to free()  //add yanly150521
		    	}
		    }
		}
	}
	return JSON_OK;
}


int msghandle_energy_term(cJSON *root, int tmp_socket)
{

    msgform term_msg;

	char *IEEE_ID=NULL;
	char *Nwkaddr_ID=NULL;
	char *Term_Code=NULL;

	char term_len=0;
	char node_len=0;
	 if((cJSON_GetObjectItem(root, "IEEE")==NULL)||
		(cJSON_GetObjectItem(root, "Nwkaddr")==NULL)||
		(cJSON_GetObjectItem(root, "TermCode")==NULL)||
		 (cJSON_GetObjectItem(root,"Sn")==NULL))
	   {
		      return JSON_KEY_ERROR;
	   }


	                Client_Sn=cJSON_GetObjectItem(root,"Sn")->valueint;
				    Client_Socket=tmp_socket;
				    IEEE_ID=cJSON_GetObjectItem(root,"IEEE")->valuestring;
				    Nwkaddr_ID=cJSON_GetObjectItem(root,"Nwkaddr")->valuestring;
	                Term_Code=cJSON_GetObjectItem(root,"TermCode")->valuestring;

	                msgfromflg=MsgComClient;
	               // SendDataToDev((uint8*)Term_Code,(uint8*)Nwkaddr_ID);
	                term_len=strlen(Term_Code);
	                node_len=strlen(Nwkaddr_ID);

	                term_msg.mtext[0]=term_len;
				    memcpy(term_msg.mtext+1,Term_Code,term_len);

				    term_msg.mtext[1+term_len]=node_len;
				    memcpy(term_msg.mtext+term_len+2,Nwkaddr_ID,node_len);

				    //printf("mtext=%s\n",term_msg.mtext);
				    json_msgsndclient(MsgtermTxId, TermQueAckMsg, term_msg.mtext, strlen(term_msg.mtext));
				    memset(term_msg.mtext,0,sizeof(term_msg.mtext));
	                //msgsnd(MsgtermTxId,&term_msg,strlen(term_msg.mtext),0);
	                return JSON_OK;

}

int msghandle_ConfigQueAckMsg(int msg_sn,uint16 tmp_socket)
{
	char **q_data=NULL;
	char **ieee=NULL;int ieee_row,ieee_col;
	int q_row,q_col;int i,j;
	int index_ieee,index_q;
	char *sql_req_config = "select ieee,nwkaddr,TermCode,Termtype,TermInfo,TermPeriod from etable where nwkaddr not null";
	char *sql_req_ieee = "SELECT  DISTINCT ieee,nwkaddr FROM etable where nwkaddr not null";
    //int ret=0;
	cJSON *sroot;
	cJSON *NodeList_array, *NodeList_item;
	cJSON *SubNodeItem, *SubNodeArray;
    //int opt ;
    char *out=NULL;
   // printf("receive client msg: energy config check\n");
//query database:
    q_data = sqlite_query_msg(&q_row, &q_col, sql_req_config);
    if(q_data ==NULL)
    {
//    	printf("query db error\n");
    	sqlite_free_query_result(q_data);
    	return JSON_VALUE_ERROR;
    }
    ieee = sqlite_query_msg(&ieee_row, &ieee_col,sql_req_ieee);
    if(ieee == NULL)
    {
    	printf("query db error\n");
    	sqlite_free_query_result(ieee);
    	return JSON_VALUE_ERROR;
    }

//organize json package
    sroot = cJSON_CreateObject();
	cJSON_AddNumberToObject(sroot, "MsgType", ConfigQueAckMsg);
	cJSON_AddNumberToObject(sroot, "Sn", msg_sn);
	cJSON_AddItemToObject(sroot, "NodeList", NodeList_item =cJSON_CreateArray());
	for(i=0;i<ieee_row;i++)
	{
		index_ieee = i*ieee_col+ieee_col;//ieee data的每行头
		cJSON_AddItemToArray(NodeList_item, NodeList_array=cJSON_CreateObject());
		cJSON_AddStringToObject(NodeList_array, "IEEE", ieee[index_ieee]);
		cJSON_AddStringToObject(NodeList_array, "Nwkaddr", ieee[index_ieee+1]);
		cJSON_AddItemToObject(NodeList_array, "TermList", SubNodeItem =cJSON_CreateArray());
		for(j=0;j<q_row;j++)
		{
			index_q = j*q_col + q_col;//data的每行头
			if(strcmp(q_data[index_q], ieee[index_ieee])==0)
			{
				cJSON_AddItemToArray(SubNodeItem, SubNodeArray=cJSON_CreateObject());
				cJSON_AddStringToObject(SubNodeArray, "TermCode", q_data[index_q+2]);
				cJSON_AddNumberToObject(SubNodeArray, "TermType", atoi(q_data[index_q+3]));
				cJSON_AddStringToObject(SubNodeArray, "TermInfo", q_data[index_q+4]);
				cJSON_AddNumberToObject(SubNodeArray, "TermPeriod", atoi(q_data[index_q+5]));

			}

		}
	}

	out = cJSON_PrintUnformatted(sroot);
	//printf("out=%s\n",out);
    //send msg to client:
	//printf("send msg to all client>>");
	send_msg_to_client(out, strlen(out), tmp_socket);
	//send_msg_to_all_client(out, strlen(out));

	cJSON_Delete(sroot);
	free(out);
	sqlite_free_query_result(q_data);
	sqlite_free_query_result(ieee);
	return JSON_OK;
}

int parse_json_client(int msgtype,cJSON *root,int tmp_socket)
{


	int ret;
	int msg_sn;

   switch(msgtype)
	{

		     case ConfigMsg: //收到配置下发消息

			  ret=msghandle_energy_config(root,tmp_socket);

			   break;
			 case ConfigQueMsg: //收到配置查询消息
				 if(cJSON_GetObjectItem(root, "Sn")==NULL)
				    {
					   ret= JSON_KEY_ERROR;
				    }
			    msg_sn=cJSON_GetObjectItem(root,"Sn")->valueint;
			   ret=msghandle_ConfigQueAckMsg(msg_sn,tmp_socket);//发送配置查询响应消息
			   break;
			 case TermQueMsg: //收到表读数查询消息
			   ret= msghandle_energy_term(root,tmp_socket);
			    break;
			 case TermNumReportAckMsg:	//收到表读数上报响应消息
				ret=JSON_OK;
			   break;
			 default:break;

			 }
	  return ret;
}


void parse_json_node(char *text,uint8 textlen)
{

	cJSON *root =NULL;
	int msgtype;
    char *Time=NULL ;
    char *IEEE=NULL;
    char *Nwkaddr=NULL;
    char *EP=NULL;
    char *data =NULL;
	uint8 databuf_len=0;
	char *Rx_msgbuff=NULL;
	char *heart_beat=NULL;
	//uint8 heart_buff[20]={"0200070007"};
    int i=0,len=0,ret=0;
	//char *databuf=NULL;
	char databuf[MAXBUF]={0};

    Rx_msgbuff=text;
    //printf("node_Rx_msgbuff=%s\n",Rx_msgbuff);
    root=cJSON_Parse(Rx_msgbuff);
	if (!root)
    {
	 printf("****Parse client JSON Error before: %s\n****",cJSON_GetErrorPtr());
	 ret = JSON_PARSE_FAILED;
    }
	{
          msgtype=cJSON_GetObjectItem(root,"msgtype")->valueint;
          //printf("node_msgtype=%d\n",msgtype);
          switch(msgtype)
          {
          case TERM_MSGTYPE:
			     Time =cJSON_GetObjectItem(root, "Time")->valuestring;
			     //printf("Time=%s\n",Time);

	             IEEE =cJSON_GetObjectItem(root, "IEEE")->valuestring;
	             //printf("IEEE=%s\n",IEEE);

	             Nwkaddr=cJSON_GetObjectItem(root, "Nwkaddr")->valuestring;
	             //printf("Nwkaddr=%s\n",Nwkaddr);

			     EP=cJSON_GetObjectItem(root, "EP")->valuestring;
			     //printf("EP=%s\n",EP);

              data=cJSON_GetObjectItem(root, "data")->valuestring;
              //printf("data=%s\n",data);

	         if(IEEE)
	          {
	          	 if(data)
	          	 {
	          		sprintf(get_systime,"%s",Time);
					printf("get_systime=%s\n",get_systime);

                   len=strlen(data)/2;
                   //databuf=(char *)malloc(sizeof(char)*len);
                   printf("databuf=");
                   for(i=0;i<len;i++)
                   {
                   sscanf(data+i*2,"%02X",databuf+i);
                   printf("%0X ",databuf[i]);
                   }
                   printf("\n");

                   databuf_len=len;
                   printf("databuf_len=%d\n",databuf_len);
				   unpack_term_msg((uint8*)databuf,databuf_len);
				   //free(databuf);
				   }
				  else
				  {
                      printf("from node no data\n" );
				  }

			    }
			  else
			  {printf("node return error\n" );}
	         break;

          case HEART_MSGTYPE:

        	  heart_beat=cJSON_GetObjectItem(root,"heartbeat")->valuestring;

        	  printf("node heart_ack=%s\n",heart_beat);
        	  break;
          default: break;
          }
	   }
		cJSON_Delete(root);
		return;

   }



/*设置全局布撤防状态
 * */
void set_global_opertor_status(char status)
{
	global_operator = status;
	GDGL_DEBUG("set global_opertor is %d\n", global_operator);
}

