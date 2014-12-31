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
#include<arpa/inet.h>
#include<netdb.h>
#include"pthread.h"


#include"net.h"
#include"json.h"
#include"cJSON.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"


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
typedef void (* functionP_t) (cJSON *root, int fd);
static void msghandle_security_config(cJSON *root, int fd);
static void msghandle_security_config_check(cJSON *root, int fd);
static void msghandle_sensor_state_check(cJSON *root, int fd);
static void msghandle_switch_state_check(cJSON *root, int fd);
static void msghandle_switch_state_ctrl(cJSON *root, int fd);
//
//send respond to node
static void upload_sensor_change_respond(char *addr, char *text, int text_len);


const functionP_t normalTransaction[]=
{
	msghandle_security_config, //0x70
	msghandle_security_config_check,
	msghandle_sensor_state_check,
	msghandle_switch_state_check,
	msghandle_switch_state_ctrl
};
static void msghandle_security_config(cJSON *root, int fd){}
static void msghandle_security_config_check(cJSON *root, int fd){}
static void msghandle_sensor_state_check(cJSON *root, int fd){}
static void msghandle_switch_state_check(cJSON *root, int fd){}
static void msghandle_switch_state_ctrl(cJSON *root, int fd)
{

	cJSON *_root = root;
//	int MsgType;
	int Sn;
	char *SecurityNodeID;
	char *Nwkaddr;
	int SwitchNum;
	int SwitchStatus;
	char buff[30] ={0x01,0x01,0x53,0x74,0x72,0x69,0x6E,0x67,0x00,0x00,0x00,0x01};
	int b_len;

	printf("msghandle_switch_state_ctrl\n");
	Sn = cJSON_GetObjectItem(_root, "Sn")->valueint;
	SecurityNodeID = cJSON_GetObjectItem(_root,"SecurityNodeID")->valuestring;
	Nwkaddr = cJSON_GetObjectItem(_root,"Nwkaddr")->valuestring;
	SwitchNum = cJSON_GetObjectItem(_root, "SwitchNum")->valueint;
	SwitchStatus = cJSON_GetObjectItem(_root, "SwitchStatus")->valueint;
	buff[10] = SwitchNum;
	buff[11] = SwitchStatus;
	b_len = 12;

	send_data_to_dev_security(Nwkaddr, buff, b_len);

}
//////////////
static void upload_sensor_change_respond(char *addr, char *text, int text_len)
{
	char buff[11] ={0x00,0x02,0x53,0x74,0x72,0x69,0x6E,0x67,0x00,0x00,0x00};
	int b_len = 11;
	buff[10] = text[9];
	printf("sensor num%d\n",buff[10]);
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
    uint8 EnergyNodeID[NODE_LEN]={0};
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

	if((text[0]>=0x30)&&(text[0]<=0x39))
	{
		return DETACH_PRASE_ERROR;
	}
	root=cJSON_Parse(text);
	if (!root)
	{
		printf("****Parse client JSON Error before: %s\n****",cJSON_GetErrorPtr());
		return DETACH_PRASE_ERROR;//need to return immediately 空指针需立即返回
	}
	msgtype = cJSON_GetObjectItem(root, "MsgType")->valueint;
	if((msgtype>= 0x10)&&(msgtype<0x70))
	{
		status = DETACH_BELONG_ENERGY;
	}
	else if((msgtype>= 0x70)&&(msgtype<0x80))
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
//	int Sn;
//	char Nwkaddr[4];
//	char SecurityNodeID[16];
//	int SensorNum;
//	int SwitchStatus;
	cJSON *root;
	int cfd = fd;
	cJSON *sroot;
	char *sout;
	int i;

	root=cJSON_Parse(buff);
	if(root)
	{
		//printf("12345789\n");
		MsgType = cJSON_GetObjectItem(root, "MsgType")->valueint;
		normalTransaction[MsgType-112](root, cfd);
		//respond
		sroot=cJSON_CreateObject();
		cJSON_AddNumberToObject(sroot,"MsgType",		MsgType+10);
		cJSON_AddNumberToObject(sroot,"Sn",				10);
		sout=cJSON_Print(sroot);
		if(send(cfd,sout,strlen(sout),0)<=0)
		{
			//send error handle
			for(i=0;i<MAX_CLIENT_NUM;i++)
			{
				if(connect_host[i] == cfd)
				{
					connect_host[i] = -1;
					client_num --;
					break;
				}
			}
			close(cfd);
			printf("sock%d send error ,may be disconneted\n",cfd);
		}
		cJSON_Delete(sroot);
		free(sout);
	}
    cJSON_Delete(root);
    return 0;
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
//
int parse_json_node_security(char *text,int textlen)
{

	cJSON *root ;
	cJSON *sroot;
	char *sout;

	int msgtype;
    char *IEEE=NULL;
    char *Nwkaddr=NULL;
    char *data =NULL;
	uint8 databuf_len;
	char switchnum,switchstatus,sensornum,sensorstatus;
	char *databuf=NULL;
	int s_len;
	int i;

    root=cJSON_Parse(text);
	if (!root)
    {
	 	return JSON_PARSE_FAILED;
    }
	{
		msgtype=cJSON_GetObjectItem(root,"msgtype")->valueint;
		printf("node_msgtype=%d\n",msgtype);
		switch(msgtype)
        {
			case TERM_MSGTYPE:
				IEEE =cJSON_GetObjectItem(root, "IEEE")->valuestring;
				printf("IEEE=%s\n",IEEE);

				Nwkaddr=cJSON_GetObjectItem(root, "Nwkaddr")->valuestring;
				printf("Nwkaddr=%s\n",Nwkaddr);

				data=cJSON_GetObjectItem(root, "data")->valuestring;
				printf("data=%s\n",data);

				databuf_len=strlen(data)/2;
				databuf=(uint8 *)malloc(sizeof(uint8)*databuf_len);
				printf("databuf= ");
				for(i=0;i<databuf_len;i++)
				{
					sscanf(data+i*2,"%02X",databuf+i);
					printf("%0X ",databuf[i]);
				}
				printf("databuf_len=%d \n",databuf_len);

				//handle msg:
				switch (databuf[1])
				{
					case 0x00://传感器上传的
//						printf("upload :node sensor upload\n");
//						sensornum = databuf[databuf_len-2];
//						sensorstatus = databuf[databuf_len-1];
//						sroot=cJSON_CreateObject();
//						cJSON_AddNumberToObject(sroot,"MsgType",				118);
//						cJSON_AddNumberToObject(sroot,"Sn",						10);
//						cJSON_AddStringToObject(sroot,"SecurityNodeID",		IEEE);
//						cJSON_AddStringToObject(sroot,"Nwkaddr",				Nwkaddr);
//						cJSON_AddNumberToObject(sroot,"SensorNum",				sensornum);
//						cJSON_AddNumberToObject(sroot,"SensorStatus",			sensorstatus);
//						sout = cJSON_Print(sroot);
//						cJSON_Delete(sroot);
//						printf("%s\n",sout);
//						s_len = strlen(sout);
//						//发送给所有在线客户端
//						send_msg_to_all_client(sout, s_len);
//						//发送给云代理服务器  141230
//						//....no do
//						//响应给节点
//						upload_sensor_change_respond(Nwkaddr, databuf, databuf_len);
//						free(sout);
//						free(databuf);
						break;
					case 0x02://控制智能插座的响应
						printf("respond :ctrl switch\n");
						switchnum = databuf[databuf_len-2];
						switchstatus = databuf[databuf_len-1];
						sroot=cJSON_CreateObject();
						cJSON_AddNumberToObject(sroot,"MsgType",				117);
						cJSON_AddNumberToObject(sroot,"Sn",						10);
						cJSON_AddStringToObject(sroot,"SecurityNodeID",		IEEE);
						cJSON_AddStringToObject(sroot,"Nwkaddr",				Nwkaddr);
						cJSON_AddNumberToObject(sroot,"SwitchNum",				switchnum);
						cJSON_AddNumberToObject(sroot,"SwitchStatus",			switchstatus);
						sout = cJSON_Print(sroot);
						cJSON_Delete(sroot);
						printf("%s\n",sout);
						s_len = strlen(sout);
						//发送给所有在线客户端
						send_msg_to_all_client(sout, s_len);
						//发送给云代理服务器  141230
						//....no do
						free(sout);
						free(databuf);
						break;
					default:break;
				}
				break;

			case HEART_MSGTYPE:
//        	  heart_beat=cJSON_GetObjectItem(root,"heartbeat")->valuestring;
//
//        	  printf("node heart_ack=%s\n",heart_beat);
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




