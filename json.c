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
#include"pthread.h"
#include <sqlite3.h>  //add yan150104


#include"net.h"
#include"json.h"
#include"cJSON.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"
#include "appSqlite.h" //add yanly150105

//#include"public.h"
extern uint16 client_num;
extern int MsgtermTxId; //���ͳ������ݵ���Ϣ����
extern int MsgtermRxId; //���ܳ������ݵ���Ϣ����
extern int MsgserverTxId;//���ͷ��������ݵ���Ϣ����

extern node_list node_table[NODE_NUM];
extern client_status client_list[MAX_CLIENT_NUM];
extern uint8 Term_Num[NODE_NUM];
extern uint8 Client_Sn;
extern int Client_Socket;
//extern uint8 msgfromflg;

#define JSON_PARSE_FAILED	 -1
uint8 Rx_count1=0;
uint8 Rx_count2=0;

char get_systime[128]={0};
char *GatewayID="8888888";
uint8 server_sn=0;

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
	cJSON *origin = root;
	cJSON *respond;
	char *out;
	int g_operator;
	char sql[128];

	printf("set global operator>>");
	////////////////////////////////////////////////////////////////////////////////预取json异常处理
	if(cJSON_GetObjectItem(origin, "OperatorType") == NULL)
		return JSON_KEY_ERROR;
	////////////////////////////////////////////////////////////////////////////////
	//数据解包
	g_operator = cJSON_GetObjectItem(origin, "OperatorType")->valueint;
	if((g_operator >= OPERATER_MAX)||(g_operator == 0))
		return JSON_VALUE_ERROR;

	//数据插入数据库
	sprintf(sql,"UPDATE stable SET operator = %d WHERE ieee = '%s'",g_operator,GLOBAL_OPERATOR_IN_IEEE_NAME);
	sqlite_updata_msg(sql);

	//响应
	respond  = cJSON_CreateObject();
	cJSON_AddNumberToObject(respond, "MsgType", SECURITY_SET_GLOBAL_OPERATOR_MSG_RES);
	cJSON_AddNumberToObject(respond, "Sn", 10);
	//cJSON_AddNumberToObject(respond, "respond_status", JSON_OK);
	out = cJSON_PrintUnformatted(respond);
	send_msg_to_client(out, strlen(out),fd);
	free(out);
	cJSON_Delete(respond);

	//转发给其他客户端
	cJSON_ReplaceItemInObject(origin, "MsgType", cJSON_CreateNumber(SECURITY_UPLOAD_GLOBAL_OPERATOR_MSG));
	out = cJSON_PrintUnformatted(origin);
	send_msg_to_all_client(out, strlen(out));

	//转发给服务器
	//...wait to do in the feature!

	//release
	free(out);
	printf("over!\n");
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

	printf("set device operator>>");
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
	sqlite_updata_msg(sql);

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

	//release
	free(out);
	printf("over!\n");
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
	char opt;
	int i,j;
	//
	subsecurityConfig_t *subnod;

	printf("set security config >> ");
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
	printf("send to all client >> ");
	//转发给服务器
	//...wait to do in the feature!
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
		    	//printf("ieee:%s,addr:%s\n",SecurityNodeID,Nwkaddr);
		    	SubNode_array = cJSON_GetObjectItem (NodeList_item, "SubNode");
		    	subnode_cnt = cJSON_GetArraySize (SubNode_array ); //获取数组的大小
		    	subnod = (subsecurityConfig_t *)malloc(subnode_cnt*sizeof(subsecurityConfig_t));//malloc //
		    	if(subnod ==NULL)
		    	{
		    		printf("malloc fail!\n");
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
		    		printf("request switch status for ieee %s,num",SecurityNodeID);

		    		if(subnod[j].type == SECURITY_SWITCH_TYPE)
		    		{
		    			char buff[12] ={0x01,0x01,0x53,0x74,0x72,0x69,0x6E,0x67,0x00,0x00,0x00,0x01};
		    			buff[10] = subnod[j].num;
		    			buff[11] = subnod[j].operator;
		    			send_data_to_dev_security(Nwkaddr, buff, 12);
		    			printf("%d,",j);
		    		}
		    	}
		    	printf(">>over!\n");
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
    int opt ;
    char *out=NULL;
    printf("security config check>>");
//query database:
    q_data = sqlite_query_msg(&q_row, &q_col, sql_req_config);
    if(q_data ==NULL)
    {
    	printf("query db error>>\n");
    	sqlite_free_query_result(q_data);
    	return JSON_VALUE_ERROR;
    }
    ieee = sqlite_query_msg(&ieee_row, &ieee_col,sql_req_ieee);
    if(ieee == NULL)
    {
    	printf("query db error>>\n");
    	sqlite_free_query_result(ieee);
    	return JSON_VALUE_ERROR;
    }
    opt = sqlite_query_global_operator();
    if(opt ==-1)
    {
    	printf("query db error>>\n");
    	return JSON_VALUE_ERROR;
    }
//organize json package
    sroot = cJSON_CreateObject();
	cJSON_AddNumberToObject(sroot, "MsgType", 129);
	cJSON_AddNumberToObject(sroot, "Sn", 10);
	cJSON_AddNumberToObject(sroot, "GlobalOpt", opt);
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
	printf("send msg to all client>>");
	send_msg_to_client(out, strlen(out), fd);
//free:
	cJSON_Delete(sroot);
	free(out);
	sqlite_free_query_result(q_data);
	sqlite_free_query_result(ieee);
	printf("over!\n");
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

	printf("control switch>>");
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
	printf("respond>>over!\n");
	return JSON_OK;
}
//////////////
static void upload_sensor_change_respond(char *addr, int num, int text_len)
{
	char buff[11] ={0x00,0x02,0x53,0x74,0x72,0x69,0x6E,0x67,0x00,0x00,0x00};
	int b_len = 11;
	buff[10] = num;
	printf("sensor upload respond>>>>buff:%s\n",buff);
	send_data_to_dev_security(addr, buff, b_len);
}


/*********************************************************************************/
uint8 parse_json_client(char *text,uint8 textlen,int tmp_socket)
{
 
	int msgtype;
    int sn;
    cJSON *root;
	cJSON *NodeList=NULL;
	cJSON *TermList=NULL;
	cJSON *nodeitem=NULL;
	cJSON *termitem=NULL;

	uint8 i,j,k,ret;

    msgform term_msg;
	msgform server_msg;
	char *IEEE_ID=NULL;
	char *EnergyNode_ID=NULL;
	char *Term_Code=NULL;
	char *Term_Info=NULL;
    uint8 termlen=0;
    uint8 tmp_code[TERM_LEN]={0};
    //uint8 termcode[TERM_LEN]={0};
//    uint8 EnergyNodeID[NODE_LEN]={0};
    uint8 node_num=0;
    uint8 term_num=0;
	printf("text=%s\n",text);

	root=cJSON_Parse(text);
        if (!root)
        {
            printf("****Parse client JSON Error before: %s\n****",cJSON_GetErrorPtr());
            ret = JSON_PARSE_FAILED;
        }
        else
        {

		 msgtype = cJSON_GetObjectItem(root, "MsgType")->valueint;
		 sn = cJSON_GetObjectItem(root, "Sn")->valueint;
		 printf("MsgType=%02x\n",msgtype);
		 printf("Sn=%02x\n",sn);

		 switch(msgtype)

		 {
		  case HeartMsg:  //收到心跳消息

			  printf("client HeartMsg\n");
		       
			   client_list[client_num].client_alive=online;
			   client_list[client_num].client_count=client_heart_count;//心跳检测时间为3*40=120s

			   package_json_client(HeartAckMsg,sn,tmp_socket);     //发送心跳响应消息
			   break;

		  case ConfigMsg: //收到配置下发消息

			   printf("client ConfigMsg\n");
			   memset(node_table,0,sizeof(node_table));
			   NodeList=cJSON_GetObjectItem(root, "NodeList");
			   if(NodeList)
			   {
				 node_num=cJSON_GetArraySize (NodeList); 
				 if(node_num)
				 {
				  for(i=0;i<node_num;i++)
				  {
				    nodeitem=cJSON_GetArrayItem(NodeList, i);

				    IEEE_ID=cJSON_GetObjectItem(nodeitem,"IEEE")->valuestring;
				    memcpy(node_table[i].IEEE,IEEE_ID,strlen(IEEE_ID));
					printf("IEEE=%s\n",node_table[i].IEEE);

					EnergyNode_ID=cJSON_GetObjectItem(nodeitem,"EnergyNodeID")->valuestring;
					memcpy(node_table[i].EnergyNodeID,EnergyNode_ID,strlen(EnergyNode_ID));
					printf("EnergyNodeID=%s\n",node_table[i].EnergyNodeID);

					TermList=cJSON_GetObjectItem(nodeitem,"TermList");
					term_num=cJSON_GetArraySize(TermList);
					Term_Num[i]=term_num;
					if(term_num)
					 {
					  for(j=0;j<term_num;j++)
						{
						      termitem=cJSON_GetArrayItem(TermList,j);
							   
							   Term_Code=cJSON_GetObjectItem(termitem,"TermCode")->valuestring;
							   termlen=strlen(Term_Code);

							   if(termlen>2*TERM_LEN)
							   {
								   printf("TERM LEN error\n");
							   }
							   else
							   {

								 sscanf(Term_Code, "%2x %2x %2x %2x %2x %2x ",&tmp_code[0], &tmp_code[1], &tmp_code[2], &tmp_code[3], &tmp_code[4], &tmp_code[5]);

								 memcpy(node_table[i].term_table[j].TermCode,tmp_code,TERM_LEN);
								 printf("TermCode=\n");
								 for(k=0;k<TERM_LEN;k++)
								 { printf("%02x",node_table[i].term_table[j].TermCode[k]);}
								 printf("\n");

							     }


							   node_table[i].term_table[j].TermType=cJSON_GetObjectItem(termitem,"TermType")->valueint;
							   printf("TermType=%d\n",node_table[i].term_table[j].TermType);

							   Term_Info=cJSON_GetObjectItem(termitem, "TermInfo")->valuestring;
							   memcpy(node_table[i].term_table[j].TermInfo,Term_Info,strlen(Term_Info));
							   printf("Term_Info=%s\n",node_table[i].term_table[j].TermInfo);

							   node_table[i].term_table[j].TermPeriod=cJSON_GetObjectItem(termitem, "TermPeriod")->valueint;
							   printf("TermPeriod=%d\n",node_table[i].term_table[j].TermPeriod);

							   }
						   }
					   }
				   }
				   
			  }

			   package_json_client(ConfigAckMsg,sn,tmp_socket);  //发送配置下发响应消息

			   server_msg.mtype=0;
              // memset(server_msg.mtext,0,MAXBUF);

			   server_msg.mtype=ConfigReportMsg;
			   printf("server_msg.mtype=%04x",server_msg.mtype);
			  // memcpy(server_msg.mtext,text,textlen);
			  // printf("server_msg.mtext=");
			  // for(i=0;i<textlen;i++)
			  // {
				//  printf("%02x",server_msg.mtext[i]);
			  // }
			 //  printf("\n");
               msgsnd(MsgserverTxId, &server_msg, sizeof(msgform),0);  //向服务器发送配置上报消息
			   break;
			 case ConfigQueMsg: //收到配置查询消息
			   package_json_client(ConfigQueAckMsg,sn,tmp_socket);	//发送配置查询响应消息
			   break;
			 case TermQueMsg: //收到表读数查询消息
				term_msg.mtype=0x0000;
			    memset(term_msg.mtext,0,MAXBUF);
				Client_Sn=sn;
			    Client_Socket=tmp_socket;
			    IEEE_ID=cJSON_GetObjectItem(root,"IEEE")->valuestring;
			    EnergyNode_ID=cJSON_GetObjectItem(root,"EnergyNodeID")->valuestring;
                Term_Code=cJSON_GetObjectItem(root,"TermCode")->valuestring;

                sscanf(Term_Code, "%2x %2x %2x %2x %2x %2x ",&tmp_code[0], &tmp_code[1], &tmp_code[2], &tmp_code[3], &tmp_code[4], &tmp_code[5]);

                term_msg.mtype=client;
			    memcpy(term_msg.mtext,tmp_code,TERM_LEN);
			    memcpy(term_msg.mtext+TERM_LEN,EnergyNode_ID,strlen(EnergyNode_ID));
			    printf("mtext=");
			    for(i=0;i<(TERM_LEN+strlen(EnergyNode_ID));i++)
			    {printf("%02x",term_msg.mtext[i]);}
			    printf("\n");
                msgsnd(MsgtermTxId,&term_msg,sizeof(msgform),0);
			    break;
			 case TermNumReportAckMsg:	//收到表读数上报响应消息
			   break;
			 default:break;

			 }

        }
      cJSON_Delete(root);
	  return ret;
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
		printf("****Parse client JSON Error before: %s\n****",cJSON_GetErrorPtr());
		return DETACH_PRASE_ERROR;//need to return immediately 空指针需立即返回
	}
	if(cJSON_GetObjectItem(root, "MsgType") == NULL)
	{
		printf("msgtype error!\n");
		cJSON_Delete(root);
		return DETACH_PRASE_ERROR;
	}
	msgtype = cJSON_GetObjectItem(root, "MsgType")->valueint;
	if((msgtype>= 0x10)&&(msgtype<0x70))
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
	int ret;

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
			sout=cJSON_Print(sroot);
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
	cJSON *root;
	int cfd = fd;
	cJSON *sroot;
	char *sout;

	root=cJSON_Parse(buff);
	if(root)
	{
		MsgType = cJSON_GetObjectItem(root, "MsgType")->valueint;
		sroot=cJSON_CreateObject();
		switch (MsgType)
		{
			case HeartMsg:
				cJSON_AddNumberToObject(sroot,"MsgType",		MsgType+0x10);
				cJSON_AddNumberToObject(sroot,"Sn",				10);
				sout=cJSON_Print(sroot);

				break;
			default:break;
		}
		send_msg_to_client(sout,strlen(sout),cfd);  //tcp send respond
		cJSON_Delete(sroot);
		free(sout);
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
	sout=cJSON_Print(sroot);
	send_msg_to_client(sout,strlen(sout),fd);  //tcp send respond
	cJSON_Delete(sroot);
	free(sout);
	cJSON_Delete(root);
}
void package_json_client(uint8 sendmsgtype,int msg_sn,uint16 tmp_socket)
{
    
    char *out=NULL;
    
	cJSON *root;
	cJSON *TermList=NULL;
	cJSON* Term=NULL;
	cJSON *NodeList=NULL;
	cJSON* node=NULL;
	char IEEID[INFOLEN]={0};
	char EnergyNodeID[NODE_LEN]={0};
	char TermCode[TERM_LEN]={0};
	char TermInfo[INFOLEN]={0};
	char *Term_Code=NULL;
	char  Tmp_Code[TERM_LEN]={0};
	uint16 net;
	msgform msg_Rxque;
	uint8 Rx_msgnum=0;
//	uint8 termc[TERM_LEN]={0};
//	uint8 termt;
//	uint32 termd;
	uint8 i,j;
	TxMsg *Term_TxMsg=NULL ;
    uint8 Tmp_Buf[MAXBUF]={0};
    char *term_code=NULL;


    root=cJSON_CreateObject();	
    switch(sendmsgtype)
   	{
       case HeartAckMsg:    //发送心跳响应消息
            cJSON_AddNumberToObject(root,"MsgType", HeartAckMsg);
            cJSON_AddNumberToObject(root,"Sn",msg_sn);

			out=cJSON_PrintUnformatted(root); 
            cJSON_Delete(root);
             printf("out=%s\n",out);
			net=send(tmp_socket,out, strlen(out),MSG_DONTWAIT);
            break;
	   case ConfigAckMsg:  //发送表配置响应消息

            cJSON_AddNumberToObject(root,"MsgType", ConfigAckMsg);
            cJSON_AddNumberToObject(root,"Sn",msg_sn);

			out=cJSON_PrintUnformatted(root); 
			cJSON_Delete(root);
			net=send(tmp_socket,out, strlen(out),MSG_DONTWAIT);
		    break;

	  case ConfigQueAckMsg: //发送表配置查询响应消息

            cJSON_AddNumberToObject(root,"MsgType", ConfigQueAckMsg);
            cJSON_AddNumberToObject(root,"Sn",msg_sn);
			cJSON_AddItemToObject(root,"NodeList",NodeList=cJSON_CreateArray());
	       for (i=0;i<NODE_NUM;i++)
	        {
	    	   cJSON_AddItemToArray(NodeList, node = cJSON_CreateObject());
	    	   memcpy(IEEID,node_table[i].IEEE,sizeof(node_table[i].IEEE));
	    	   cJSON_AddItemToObject(node, "IEEE", cJSON_CreateString(IEEID));

	    	   memcpy(EnergyNodeID,node_table[i].EnergyNodeID,sizeof(node_table[i].EnergyNodeID));
	    	   cJSON_AddItemToObject(node, "EnergyNodeID", cJSON_CreateString(EnergyNodeID));


	    	   cJSON_AddItemToObject(node, "TermList",TermList= cJSON_CreateArray());
			   for(j=0;j<TERM_NUM;j++)
		      {
				 cJSON_AddItemToArray(TermList, Term= cJSON_CreateObject());

                 memcpy(TermCode,node_table[i].term_table[j].TermCode,TERM_LEN);
                 sprintf(Tmp_Code,"%02x%02x%02x%02x%02x%02x",TermCode[0],TermCode[1],TermCode[2],TermCode[3],TermCode[4],TermCode[5]);
                 Term_Code=Tmp_Code;
                 cJSON_AddItemToObject(Term,"TermCode",cJSON_CreateString(Term_Code));

				 cJSON_AddItemToObject(Term,"TermType",cJSON_CreateNumber(node_table[i].term_table[j].TermType));

				 memcpy(TermInfo,node_table[i].term_table[j].TermInfo,sizeof(node_table[i].term_table[j].TermInfo));
				 cJSON_AddItemToObject(Term,"TermInfo",cJSON_CreateString(TermInfo));

				 cJSON_AddItemToObject(Term,"TermPeriod",cJSON_CreateNumber(node_table[i].term_table[j].TermPeriod));

	          }

	       	}
		     
			out=cJSON_PrintUnformatted(root); 
            cJSON_Delete(root);
			net=send(tmp_socket,out, strlen(out),MSG_DONTWAIT);
	   	    break;

	  case TermQueAckMsg: //发送表读数查询响应消息

            cJSON_AddNumberToObject(root,"MsgType", TermQueAckMsg);
            cJSON_AddNumberToObject(root,"Sn",msg_sn);

			//cJSON_AddItemToObject(root,"Termdata",Term_data=cJSON_CreateArray());

		    Rx_msgnum=msgrcv(MsgtermRxId,&msg_Rxque,(sizeof(msg_Rxque)-4),0, 0);
		    printf("Rx_msgnum=%d\n",Rx_msgnum);
			if(Rx_msgnum!=0)
			{
				memcpy(Tmp_Buf,msg_Rxque.mtext,sizeof(msg_Rxque.mtext));
				Term_TxMsg=(TxMsg *)Tmp_Buf;
				memcpy(term_code,Term_TxMsg->TermCode,sizeof(Term_TxMsg->TermCode));
				//term_code=Term_TxMsg->TermCode;
			   cJSON_AddStringToObject(root,"TermCode",term_code);
               cJSON_AddNumberToObject(root,"TermType",Term_TxMsg->TermType);
			   cJSON_AddStringToObject(root,"TermTime",get_systime);
			   cJSON_AddNumberToObject(root,"ReportData",Term_TxMsg->ReportData);

			   out=cJSON_PrintUnformatted(root);
               cJSON_Delete(root);
			   send(tmp_socket,out, strlen(out),MSG_DONTWAIT);

			}
			else
			printf("no receive term data");
	        break;
	  default:break;
   	}

}

#if 0
void parse_json_node(char *text,uint8 textlen)
{

	cJSON *root ;
	uint16 msgtype;
    char *Time=NULL ;
    char *IEEE=NULL;
    char *Nwkaddr=NULL;
    char *EP=NULL;
    char *data =NULL;
	uint8 databuf_len;
	char *Rx_msgbuff=NULL;
	char *heart_beat=NULL;
	uint8 heart_buff[11]={"0200070007"};
    int i,len,rc=0;
	char *databuf=NULL;

		  Rx_msgbuff=text;
		  printf("node_Rx_msgbuff=%s\n",Rx_msgbuff);
		  root=cJSON_Parse(Rx_msgbuff);
          msgtype=cJSON_GetObjectItem(root,"msgtype")->valueint;
          printf("node_msgtype=%d\n",msgtype);
		  if(msgtype==TERM_MSGTYPE)
			 {
			   //printf("AAAAAA\n");

			     Time =cJSON_GetObjectItem(root, "Time")->valuestring;
			     printf("Time=%s\n",Time);

	             IEEE =cJSON_GetObjectItem(root, "IEEE")->valuestring; 
	             printf("IEEE=%s\n",IEEE);

	             Nwkaddr=cJSON_GetObjectItem(root, "Nwkaddr")->valuestring;
	             printf("Nwkaddr=%s\n",Nwkaddr);

			     EP=cJSON_GetObjectItem(root, "EP")->valuestring;
			     printf("EP=%s\n",EP);

                 data=cJSON_GetObjectItem(root, "data")->valuestring;
                 printf("data=%s\n",data);

	            if(IEEE)
	          	  {
	          	  if(data)
	          	  	{
	          		  sprintf(get_systime,"%s",Time);
					  printf("get_systime=%s\n",get_systime);

                      len=strlen(data)/2;
                      databuf=(char *)malloc(sizeof(char)*len);
                      printf("databuf= ");
                      for(i=0;i<len;i++)
                      {
                      sscanf(data+i*2,"%02X",databuf+i);
                      printf("%0X ",databuf[i]);
                      }

                      databuf_len=len;
                      printf("databuf_len=%d \n",databuf_len);
					  unpack_term_msg((uint8*)databuf,databuf_len);
				    }
				  else
				  	{
                     printf("from node no data\n" );
				    }

			      }
			    else
			      {printf("node return error\n" );}

			 }
		  else if(msgtype==HEART_MSGTYPE)
		  {
			  heart_beat=cJSON_GetObjectItem(root,"heartbeat")->valuestring;
			  rc=strcmp(heart_beat,heart_buff);
			  if(rc==0)
			  {
				  printf("node heart_ack=%s\n",heart_beat);
			  }
			  else
			  {
				 printf("node no recv heart ack\n");
			  }
		  }
		cJSON_Delete(root);
   }
#endif

//int  parse_json_node(char *text,uint8 textlen)
//{
//
//	cJSON *root ;
//	int msgtype;
//    char *Time=NULL ;
//    char *IEEE=NULL;
//    char *Nwkaddr=NULL;
//    char *EP=NULL;
//    char *data =NULL;
//	uint8 databuf_len;
//	char *Rx_msgbuff=NULL;
//	char *heart_beat=NULL;
//	//uint8 heart_buff[20]={"0200070007"};
//    int i=0,len=0,ret=0;
//	char *databuf=NULL;
//
//    Rx_msgbuff=text;
//    printf("node_Rx_msgbuff=%s\n",Rx_msgbuff);
//    root=cJSON_Parse(Rx_msgbuff);
//	if (!root)
//    {
//		printf("****Parse client JSON Error before: %s\n****",cJSON_GetErrorPtr());
//	 //ret = JSON_PARSE_FAILED;
//	 	return JSON_PARSE_FAILED; //modify yanly141230
//    }
//	{
//		msgtype=cJSON_GetObjectItem(root,"msgtype")->valueint;
//		printf("node_msgtype=%d\n",msgtype);
//		switch(msgtype)
//        {
//			case TERM_MSGTYPE:
////				Time =cJSON_GetObjectItem(root, "Time")->valuestring;
////				printf("Time=%s\n",Time);
////				EP=cJSON_GetObjectItem(root, "EP")->valuestring;
////				printf("EP=%s\n",EP);
//
//				IEEE =cJSON_GetObjectItem(root, "IEEE")->valuestring;
//				printf("IEEE=%s\n",IEEE);
//
//				Nwkaddr=cJSON_GetObjectItem(root, "Nwkaddr")->valuestring;
//				printf("Nwkaddr=%s\n",Nwkaddr);
//
//				data=cJSON_GetObjectItem(root, "data")->valuestring;
//				printf("data=%s\n",data);
//
//				if(IEEE)
//				{
//					if(data)
//					{
////						sprintf(get_systime,"%s",Time);
////						printf("get_systime=%s\n",get_systime);
//
//						len=strlen(data)/2;
//						databuf=(char *)malloc(sizeof(char)*len);
//						printf("databuf= ");
//						for(i=0;i<len;i++)
//						{
//							sscanf(data+i*2,"%02X",databuf+i);
//							printf("%0X ",databuf[i]);
//						}
//
//						databuf_len=len;
//						printf("databuf_len=%d \n",databuf_len);
//						unpack_term_msg((uint8*)databuf,databuf_len);
//					}
//					else
//					{
//					  printf("from node no data\n" );
//					}
//
//				}
//				else
//				{
//					printf("node return error\n" );
//				}
//				break;
//
//			case HEART_MSGTYPE:
//				return 0;
////        	  heart_beat=cJSON_GetObjectItem(root,"heartbeat")->valuestring;
////
////        	  printf("node heart_ack=%s\n",heart_beat);
//				break;
//			default: break;
//        }
//	}
//		cJSON_Delete(root);
//		return databuf_len;
//}
void parse_json_node(char *text,uint8 textlen)
{

	cJSON *root ;
	int msgtype;
    char *Time=NULL ;
    char *IEEE=NULL;
    char *Nwkaddr=NULL;
    char *EP=NULL;
    char *data =NULL;
	uint8 databuf_len;
	char *Rx_msgbuff=NULL;
	char *heart_beat=NULL;
	//uint8 heart_buff[20]={"0200070007"};
    int i=0,len=0,ret=0;
	char *databuf=NULL;

    Rx_msgbuff=text;
    printf("node_Rx_msgbuff=%s\n",Rx_msgbuff);
    root=cJSON_Parse(Rx_msgbuff);
	if (!root)
    {
	 printf("****Parse client JSON Error before: %s\n****",cJSON_GetErrorPtr());
	 ret = JSON_PARSE_FAILED;
    }
	{
          msgtype=cJSON_GetObjectItem(root,"msgtype")->valueint;
          printf("node_msgtype=%d\n",msgtype);
          switch(msgtype)
          {
          case TERM_MSGTYPE:
			     Time =cJSON_GetObjectItem(root, "Time")->valuestring;
			     printf("Time=%s\n",Time);

	             IEEE =cJSON_GetObjectItem(root, "IEEE")->valuestring;
	             printf("IEEE=%s\n",IEEE);

	             Nwkaddr=cJSON_GetObjectItem(root, "Nwkaddr")->valuestring;
	             printf("Nwkaddr=%s\n",Nwkaddr);

			     EP=cJSON_GetObjectItem(root, "EP")->valuestring;
			     printf("EP=%s\n",EP);

              data=cJSON_GetObjectItem(root, "data")->valuestring;
              printf("data=%s\n",data);

	         if(IEEE)
	          {
	          	 if(data)
	          	 {
	          		sprintf(get_systime,"%s",Time);
					printf("get_systime=%s\n",get_systime);

                   len=strlen(data)/2;
                   databuf=(char *)malloc(sizeof(char)*len);
                   printf("databuf= ");
                   for(i=0;i<len;i++)
                   {
                   sscanf(data+i*2,"%02X",databuf+i);
                   printf("%0X ",databuf[i]);
                   }

                   databuf_len=len;
                   printf("databuf_len=%d \n",databuf_len);
				   unpack_term_msg((uint8*)databuf,databuf_len);
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
/****************************************************************************************/
int detach_5002_message22(char *text, int textlen)
{
	cJSON *root=NULL;
	int msgtype;
    char *ieee;

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
	if(msgtype != TERM_MSGTYPE)
	{
		cJSON_Delete(root);
		return (ret = DETACH_MSGTYPE_ERROR);
	}
	printf("recevie 5002 callback message 22 >>");
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
					printf("malloc fail!\n");
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
						printf("upload sensor change >>");
						sensornum = databuf[databuf_len-2];
						sensorstatus = databuf[databuf_len-1];
						//响应给节点
						upload_sensor_change_respond(Nwkaddr, sensornum, databuf_len);
						//从数据库中查询布撤防状态，看是否需要上传传感器变化状态 //add yanly150107
						char q_sql[128];int qrow,qcol;char **qopr;int opt;char qcnt;
						for(qcnt=0;qcnt<2;qcnt++)
						{
							if(qcnt==0)
								sprintf(q_sql, "select operator from stable where ieee = 'global operator'");
							if(qcnt==1)
								sprintf(q_sql, "select operator from stable where ieee = '%s' "
										"and type=%d and num=%d",IEEE,SECURITY_SENSOR_TYPE,sensornum);
							qopr = sqlite_query_msg(&qrow, &qcol, q_sql);
							if(qopr!=NULL)
							{
								opt = atoi(qopr[qcol]);
								if(opt==OPERATER_OPEN)
								{
								}
								else
								{
									//sqlite_free_query_result(qopr);
									printf("ieee operator is not set \n");
									qopr = NULL;
									break;
								}
							}
							else
							{
								//sqlite_free_query_result(qopr);
								printf("ieee  operator is null\n");
								break;
							}
						}
						if(qopr!=NULL)
						{
							printf("format correct>>");

							if(sensorstatus == SENSOR_STATUS_ALARM)
							{//控制报警器
								http_ctrl_iasWarningDeviceOperation(WARNING_DEVICE_IEEE);

								cJSON_AddNumberToObject(sroot,"MsgType",				118);
								cJSON_AddNumberToObject(sroot,"Sn",						10);
								cJSON_AddStringToObject(sroot,"SecurityNodeID",		IEEE);
								cJSON_AddStringToObject(sroot,"Nwkaddr",				Nwkaddr);
								cJSON_AddNumberToObject(sroot,"SensorNum",				sensornum);
								cJSON_AddNumberToObject(sroot,"SensorStatus",			sensorstatus);
								sout = cJSON_PrintUnformatted(sroot);
								//printf("%s\n",sout);
								s_len = strlen(sout);
								//printf("send message to all client and server!\n");
								//发送给所有在线客户端
								send_msg_to_all_client(sout, s_len);
								//发送给云代理服务器  141230
								//....no do
								free(sout);
							}
							else
							{
								printf("not need upload sensor because sensor change is normal>>");//传感器变成正常no need send
							}

						}
						else
						{
							printf("uneable to upload sensor>>");
						}
						sqlite_free_query_result(qopr);
						break;
					case 0x01://控制智能插座的响应
						printf("respond after the client control switch >>");
						cmd_respond_status = databuf[9];
						if(cmd_respond_status !=SECURITY_SERIAL_RESPOND_STATUS_SUCCESS)
						{
							printf("serial data respond status error>>");
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

					case 0x02:
						printf("respond after the client control switch >>");
						cmd_respond_status = databuf[9];
						if(cmd_respond_status !=0)
						{
							printf("serial data format error>>");
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
				printf("over!\n");
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

void parse_json_server(char *text,uint8 textlen)
{
	 cJSON* root = NULL;
	 uint8 msgtype=0;
	 char *Rx_msgbuff=NULL;

	 Rx_msgbuff=text;
	 printf("Rx_msgbuff=%s\r\n",Rx_msgbuff);
     root=cJSON_Parse(Rx_msgbuff);

     msgtype=cJSON_GetObjectItem(root,"MsgType")->valueint;
     printf("msgtype=%02x\r\n",msgtype);
	 server_sn=cJSON_GetObjectItem(root,"Sn")->valueint;
	 printf("server_sn=%02x\r\n",server_sn);
	if(msgtype==ConfigReportAckMsg)
	 {
	      printf("receive ConfigReportAckMsg success \r\n");
     }
    else
     {
    	if(msgtype==TermDataReportAckMsg)
	     {
	      printf("receive TermDataReportAckMsg success \r\n");
	     }
    	else if(msgtype==HeartReportAckMsg)
    	{
    	  printf("receive heartReportAckMsg success \r\n");
        }
     }


	cJSON_Delete(root);
    return;
}

void package_json_server(msgform *msg_Rxque ,uint8 Rx_msgnum,uint16 socketflg)
{
  msgform msgRxque;
  char *out;
  uint8 msg_type;
  cJSON* root = NULL;
  cJSON* TermList;
  cJSON* Term;
  uint8 termc[TERM_LEN]={0};
  uint8 termt;
  uint32 termd;
  char buffer[MAXBUF]={0};
  uint8 recbytes;
  uint16 Rx_msgtype;
  uint8 i,j,k;
  uint8 tmp_buf[TERM_LEN*2]={0};
  TxMsg *Term_TxMsg;

  printf("msgRxque.mtype=%04x\n",msg_Rxque->mtype);
  msg_type= (uint8)(msg_Rxque->mtype);
  printf("msg_type=%02x\n",msg_type);

  switch(msg_type)
    {
            
      case ConfigReportMsg:

            printf("server ConfigReportMsg\n");
			root=cJSON_CreateObject();
            cJSON_AddNumberToObject(root,"MsgType", ConfigReportMsg);
            cJSON_AddNumberToObject(root,"Sn",server_sn);
			cJSON_AddStringToObject(root,"GatewayID",GatewayID);
			
			cJSON_AddItemToObject(root,"TermList",TermList=cJSON_CreateArray());
			
			cJSON_AddItemToArray(TermList, Term = cJSON_CreateObject());
			

			for (i=0;i<NODE_NUM;i++)
	        {
			
			 for(j=0;j<Term_Num[i];j++)
			 
		      {
				 for (k=0;k<TERM_LEN;k++)
				 {sprintf(tmp_buf+k*2,"%02X",node_table[i].term_table[j].TermCode[k]);}
				 printf("[%s]\n",tmp_buf);

		        cJSON_AddItemToObject(Term,"TermCode", cJSON_CreateString((const char *)tmp_buf));
		        cJSON_AddItemToObject(Term,"TermType", cJSON_CreateNumber(node_table[i].term_table[j].TermType));
		        cJSON_AddItemToObject(Term,"TermInfo", cJSON_CreateString((const char *)node_table[i].term_table[j].TermInfo));
		        //cJSON_AddItemToObject(Term,"Term_Period", cJSON_CreateNumber(node_table[i].term_table[j].TermPeriod));
		      }

	       	}
									
			out=cJSON_PrintUnformatted(root); 
            cJSON_Delete(root);
            send(socketflg,out, strlen(out),MSG_DONTWAIT);
			break;
			
      case TermDataReportMsg:

	        printf("server TermDataReportMsg\n");
			root=cJSON_CreateObject();
            cJSON_AddNumberToObject(root,"MsgType", TermDataReportMsg);
            cJSON_AddNumberToObject(root,"Sn",server_sn);
            memcpy(buffer,msg_Rxque->mtext,sizeof(msg_Rxque->mtext));
            Term_TxMsg=(TxMsg*)buffer;

            for (k=0;k<TERM_LEN;k++)
            {sprintf(termc+k*2,"%02X",Term_TxMsg->TermCode[k]);}
             printf("[termc=%s]\n",termc);


               cJSON_AddStringToObject(root,"TermCode",(char*)termc);
               cJSON_AddNumberToObject(root,"TermType",Term_TxMsg->TermType);
               printf("get_systime=%s",get_systime);
			   cJSON_AddStringToObject(root,"TermTime ",get_systime);
			   cJSON_AddNumberToObject(root,"ReportData",Term_TxMsg->ReportData);
									
			   out=cJSON_PrintUnformatted(root); 
               cJSON_Delete(root);
			   send(socketflg,out, strlen(out),MSG_DONTWAIT);

              break;

	  default :break;
   }
				
  return;
}




