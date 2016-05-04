#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <pthread.h>
#include<errno.h>
#include <unistd.h>

#include"json.h"
#include"cJSON.h"
#include"net.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"
#include "appSqlite.h"



//extern int MsgtermRxId; //���ܳ������ݵ���Ϣ����
extern int MsgserverTxId;//���ͷ��������ݵ���Ϣ����
extern int MsgtermTxId;
extern volatile uint8 time1_flag;
extern volatile uint8 time2_flag;
//extern uint8 time4_flag;
extern char get_systime[INFOLEN];

extern uint8 Client_Sn;
extern int Client_Socket;

extern uint8 nodenum;
extern uint8 Term_Num[NODE_NUMM];          //���ÿ���ڵ�����ܱ����
extern node_native nodetable_native[NODE_NUMM];
//extern int Term_rcvPeriod;

volatile uint8 term_send_flag=enable;
uint8 term_clientrcv_flag=disable;
uint8 term_nativercv_flag=disable;
uint8 msgfromflg=0;

uint8 client_send_count=0;
uint8 native_send_count=0;
uint16 nnum=0;
uint16 tnum=0;


uint32 pwvalue=0;
//uint8 server_sn=0;

uint8 TermDataReportMsg_sn=0;
uint8 native_sn=0;

char termcode[INFOLEN]={0};
char nodeid[INFOLEN]={0};
char term_len=0;
char node_len=0;


void *client_term_thread(void *p)
{
  msgform term_msg={0};
  int msgsize=0;
  char tmp_msg[MAX_MSG_BUF]={0};

 while(1)
 {
  //if(time4_flag==enable)
  // {
	//  time4_flag=disable;

	 //msgsize=msgrcv(MsgtermTxId, &term_msg,sizeof(msgform),0,0);
	 msgsize=msgrcv(MsgtermTxId, &term_msg,(sizeof(msgform)-4),0,IPC_NOWAIT);
     //printf("test yang msgsize=%d\n",msgsize);
	 //if((msgsize>0)&&(term_send_flag==enable))
     if((msgsize ==-1)&&(errno !=ENOMSG))
     {
     	printf("errno:%d\n",errno);
     	perror("msgrcv:");
     }
     else if(msgsize >0)
     //if(msgsize>0)
      {
    	memset(tmp_msg,0,MAX_MSG_BUF);
        memcpy(tmp_msg,term_msg.mtext,strlen(term_msg.mtext));
        memset(term_msg.mtext,0,sizeof(term_msg.mtext));

    	term_len=tmp_msg[0];
    	//termcode=(char*)malloc(sizeof(char)*term_len);
    	memcpy(termcode,tmp_msg+1,term_len);
    	printf("term_code=%s\n",termcode);

    	node_len=tmp_msg[1+term_len];
    	//nodeid=(char*)malloc(sizeof(char)*node_len);
		memcpy(nodeid,tmp_msg+term_len+2,node_len);
		printf("node_id=%s\n",nodeid);

		//term_send_flag=disable;
		//msgfromflg=MsgComClient;
		//SendDataToDev((uint8*)termcode,(uint8*)nodeid);
		//client_send_count++;

      }
    	if(client_send_count<3)
    	{
    		 if(term_clientrcv_flag==enable)
	           {

    		    term_clientrcv_flag=disable;
    		    term_send_flag=enable;
			    client_send_count=0;
			    //memset(tmp_msg,0,sizeof(tmp_msg));
			    memset(termcode,0,sizeof(termcode));
		        memset(nodeid,0,sizeof(nodeid));
		        term_len=0;
		        node_len=0;

	          }
	         else
	         {
	    	       //json_msgsnd(MsgtermTxId,TermQueAckMsg,tmp_msg, strlen(tmp_msg));
	    	       if((term_len>0)&&(node_len>0))
	    	       {
	    	    	 term_send_flag=disable;
	    	    	 msgfromflg=MsgComClient;
	    	         SendDataToDev((uint8*)termcode,(uint8*)nodeid);
	    	         client_send_count++;
	    	        // printf("client_send_count=%d\n",client_send_count);
	    	       }
	         }
    	  }
	    else
	    {
                 term_send_flag=enable;
		         client_send_count=0;
		         memset(termcode,0,sizeof(termcode));
		         memset(nodeid,0,sizeof(nodeid));
		         //memset(tmp_msg,0,sizeof(tmp_msg));
		         term_len=0;
		         node_len=0;

	    }
  	   //}
      sleep(1);
  	}

  return 0;
}

void *term_msg_thread(void *p)
{

  uint8 term_code[INFOLEN]={0};
  uint8 node_id[INFOLEN]={0};

  while(1)
  {

	  if((time2_flag==enable)&&(time1_flag==enable)&&(term_send_flag==enable))
      {
    	  time1_flag=disable;
			  //DBG_PRINT("7777\n");
           if(nnum<nodenum)
             {
            	 // DBG_PRINT("8888\n");
                if(tnum<Term_Num[nnum])
				 {
				   	  native_send_count++;
					  if(native_send_count<=3)
					     {
				          if(term_nativercv_flag==enable)
				           {

						    tnum++;
						    term_nativercv_flag=disable;
						    //DBG_PRINT("term_nativercv_flag= %d\n",term_nativercv_flag);
						    native_send_count=0;
				           }
				          else
				           {
				        	  memcpy(node_id,nodetable_native[nnum].Nwkaddr_native,strlen(nodetable_native[nnum].Nwkaddr_native));
				        	  memcpy(term_code,nodetable_native[nnum].termtable_native[tnum].TermCode_native,strlen(nodetable_native[nnum].termtable_native[tnum].TermCode_native));
				        	  //printf("node_id[%d]=%s\n",nnum,node_id);
				        	 // printf("TermCode[%d][%d]=%s\n",nnum,tnum,term_code);
				        	  msgfromflg=MsgComNative;
				        	  SendDataToDev(term_code,node_id);
				           }
                         }	

                        else
                        {
                         //printf("read term error\n");
                         tnum++;
                         native_send_count=0;
					    }
                  	 }
                    else
					 {
					   nnum++;
					   tnum=0;
					 }

			     }
			    else
			    {
                 time2_flag=disable;
				 term_send_flag=enable;
				 nnum=0;
				 tnum=0;
				 printf("read term finish\n");
//				 sqlite_query_to_native();

				}

		    }
		  usleep(2000);
      	}

  return 0;
 }

/*----------------------------------------------------------------------------------------
|*函数名：UINT8 GetCheckCS(UINT8 *dataBuf, )
|
|*功能描述：计算校验码
|
|*输入参数：UINT8 *dataBuf, 。
|
|*输出参数：无。
|
|*返回值：。
|*时间：2011-07-05 17:50:40
----------------------------------------------------------------------------------------*/
uint8 GetCheckCS(uint8 *dataBuf, uint8 datalen)
{
	uint8 ret=0;

	while(datalen!=0)
	{
		ret+=dataBuf[datalen-1];
		datalen--;
	}
	// DBG_PRINT("ret= %d\n",ret);
	return ret;
}/*End of GetCheckCS*/




uint8 SendDataToDev(uint8 *termid,uint8 *nodeid)
{

	uint8 i=0;
	uint8 datalen=0;
	//uint8 data_Buf[256];
	uint8 dataBuf[MAXBUF]={0};
	uint8 ret=0;
    //int termlen=0;
	char termlen=0;
    uint8 *tmp_code=NULL;

	dataBuf[datalen++]=0xFE;
	dataBuf[datalen++]=0xFE;
	dataBuf[datalen++]=0xFE;
	dataBuf[datalen++]=0x68;

	 termlen= strlen(termid)/2;
	 tmp_code=(uint8 *)malloc(sizeof(uint8)*termlen);
	// printf("tmp_code= ");
	 for(i=0;i<termlen;i++)
	 {
	   sscanf(termid+i*2,"%02x",tmp_code+i);
	   //printf("%02x",tmp_code[i]);
	 }


	for(i=0;i<termlen;i++)
	{
		dataBuf[datalen++]=tmp_code[TERM_LEN-i-1];
	}

	dataBuf[datalen++]=0x68;

	dataBuf[datalen++]=READ_AMME_DATA;

	dataBuf[datalen++]=0x02; // msg_len

	dataBuf[datalen++]=((AMMTER_GET_PWVAL_REQ>>8)&0xff);
	dataBuf[datalen++]=(AMMTER_GET_PWVAL_REQ&0xff);

    ret=GetCheckCS(&dataBuf[3],datalen-3);
	dataBuf[datalen++]=ret;

	dataBuf[datalen++]=0x16;


	//DBG_PRINT("datalen=%d\n",datalen);
	//DBG_PRINT("dataBuf=");

	//for(i=0;i<datalen;i++)
	//{
	//	DBG_PRINT("%0x-",dataBuf[i]);
	//}
	//DBG_PRINT("\n");
	child_doit(dataBuf,datalen,nodeid);
//#endif
	return datalen;
/*FE FE FE 68 42 25 11 00 00 00 68 01 02 43 C3 51 16*/	
}/*End of SendDataToDev*/
void send_data_to_dev_security(char *nwkaddr, char *text, int text_len)
{
	uint8 *buff = (uint8 *)text;
	uint8 *id = (uint8 *)nwkaddr;
	child_doit(buff,text_len,id);
}

#if 0
void termGetValResClient(uint8 *msgBuf)
  {
	  uint8 c=0,k=0;
	  uint32 pwval=0;
	  GetAmmereValRes_T *pGetAmmereValRes=NULL;
	  AmmeteRxMsgHead_T *pAmmeteRxMsgHead=NULL;

	  TxMsg *Term_TxMsg=NULL;
	  char term_code[2*TERM_LEN]={0};
	  char tmp_code[2*TERM_LEN]={0};
	 // char term_code[INFOLEN]={0};
	 // char tmp_code[INFOLEN]={0};
	  //int term_len=0;
	  cJSON *root;
	  char *out;
	  float term_pwval;

	  pAmmeteRxMsgHead=(AmmeteRxMsgHead_T*)msgBuf;
	  pGetAmmereValRes=(GetAmmereValRes_T*)msgBuf;


     if((pAmmeteRxMsgHead->msglen)>2)
     {

    	 pGetAmmereValRes->smdata-=VAL_PARA;
    	 pGetAmmereValRes->bigdata[0]-=VAL_PARA;
    	 pGetAmmereValRes->bigdata[1]-=VAL_PARA;
    	 pGetAmmereValRes->bigdata[2]-=VAL_PARA;

    	 pwval=0;
    	 for(c=0; c<6; c++)
    	 {
    		pwval*=10;
    		if(c%2==0)
    		{
    		  pwval+=(pGetAmmereValRes->bigdata[2-c/2]>>4);
    		}
    		else
    		{
    		 pwval+=(pGetAmmereValRes->bigdata[2-c/2]&0xF);
    		}
    	}
    	pwvalue=pwval*100+HEXBCDTODEC(pGetAmmereValRes->smdata);
    	//DBG_PRINT("power_value=%lu\n",pwvalue);
    	term_pwval=(float)pwvalue;
    	term_pwval=term_pwval/100;
    	DBG_PRINT("term_pwval=%lf\n",term_pwval);
    	//printf("uint32: %"PRIu32"\n", pwvalue);

	     // DBG_PRINT("devid=");
		  for(k=0;k<TERM_LEN;k++)
		  {
			// DBG_PRINT("%02x",pAmmeteRxMsgHead->devid[k]);
			 tmp_code[k]=pAmmeteRxMsgHead->devid[TERM_LEN-k-1];
		  }
		  //DBG_PRINT("\n");

		 // DBG_PRINT("tmp_code=");
	      //for(k=0;k<TERM_LEN;k++) {
		  //DBG_PRINT("%02x",tmp_code[k]);
		  //}
		 // DBG_PRINT("\n");

		  sprintf(term_code,"%02x%02x%02x%02x%02x%02x",tmp_code[0],tmp_code[1],tmp_code[2],tmp_code[3],tmp_code[4],tmp_code[5]);

		  Term_TxMsg = (TxMsg *)malloc(sizeof(TxMsg));

		   Term_TxMsg->TermCode=term_code;
		   Term_TxMsg->TermTime=get_systime;
		   Term_TxMsg->ReportData=term_pwval;
		   SendmsgToclient(Term_TxMsg);
		   Term_TxMsg=NULL;
		   // send  TermNumReportMsg to server
		   root=cJSON_CreateObject();
		   cJSON_AddNumberToObject(root,"MsgType", TermDataReportMsg);
		   cJSON_AddNumberToObject(root,"Sn",TermDataReportMsg_sn);
		   if(TermDataReportMsg_sn<255)
			   TermDataReportMsg_sn++;
		   else
			   TermDataReportMsg_sn=0;
		   cJSON_AddStringToObject(root,"TermCode",Term_TxMsg->TermCode);
		   cJSON_AddStringToObject(root,"TermTime",Term_TxMsg->TermTime);
		   cJSON_AddNumberToObject(root,"ReportData",Term_TxMsg->ReportData);

		    out=cJSON_PrintUnformatted(root);
		    cJSON_Delete(root);
		    json_msgsnd(MsgserverTxId, TermDataReportMsg, out, strlen(out));
		    free(out);

	  }
      else
     {DBG_PRINT("term no return data\n");}
	 return;
 }


void  SendmsgToclient(TxMsg *TmpBuf)
{
	 char *out=NULL;
     cJSON* root = NULL;

	 if(msgfromflg==MsgComNative)
	 	{
		    msgfromflg=0;
		    term_nativercv_flag=enable;
	 		root=cJSON_CreateObject();
	 	    cJSON_AddNumberToObject(root,"MsgType", TermNumReportMsg);
	 	    cJSON_AddNumberToObject(root,"Sn",native_sn);
	 	    if(native_sn<255)
	 	    	native_sn++;
	 	    else
	 	    	native_sn=0;
	 	    cJSON_AddStringToObject(root,"TermCode",TmpBuf->TermCode);
	 	    cJSON_AddStringToObject(root,"TermTime",TmpBuf->TermTime);
	 	    cJSON_AddNumberToObject(root,"ReportData",TmpBuf->ReportData);
	 	    out=cJSON_PrintUnformatted(root);
	 	   	//DBG_PRINT("out=%s\n",out);
	 	    cJSON_Delete(root);
	 	    send_msg_to_all_client(out,strlen(out));
	 	    DBG_PRINT("SendmsgTo_allclient\n");

	 	}
	  else if(msgfromflg==MsgComClient)
	   {
		     msgfromflg=0;
		     term_clientrcv_flag=enable;
		     root=cJSON_CreateObject();
		     cJSON_AddNumberToObject(root,"MsgType", TermQueAckMsg);
		     cJSON_AddNumberToObject(root,"Sn",Client_Sn);

		     cJSON_AddStringToObject(root,"TermCode",TmpBuf->TermCode);
		     cJSON_AddStringToObject(root,"TermTime",TmpBuf->TermTime);
		 	 cJSON_AddNumberToObject(root,"ReportData",TmpBuf->ReportData);
		 	 out=cJSON_PrintUnformatted(root);
		 	 //DBG_PRINT("out=%s\n",out);
		 	 cJSON_Delete(root);
		 	 send_msg_to_client(out, strlen(out), Client_Socket);
		 	 DBG_PRINT("SendmsgTo_oneclient\n");

	  }
  return ;
	// exit(0);
}

#endif

void termGetValResClient(uint8 *msgBuf)
  {
	  uint8 c=0,k=0;
	  uint32 pwval=0;
	  GetAmmereValRes_T *pGetAmmereValRes=NULL;
	  AmmeteRxMsgHead_T *pAmmeteRxMsgHead=NULL;

	  //TxMsg *Term_TxMsg=NULL;
	  char term_code[2*TERM_LEN]={0};
	  char tmp_code[2*TERM_LEN]={0};

	  //int term_len=0;
	  cJSON *root=NULL;
	  char *out=NULL;

	  float term_pwval=0;

	  pAmmeteRxMsgHead=(AmmeteRxMsgHead_T*)msgBuf;
	  pGetAmmereValRes=(GetAmmereValRes_T*)msgBuf;


     if((pAmmeteRxMsgHead->msglen)>2)
     {

    	 pGetAmmereValRes->smdata-=VAL_PARA;
    	 pGetAmmereValRes->bigdata[0]-=VAL_PARA;
    	 pGetAmmereValRes->bigdata[1]-=VAL_PARA;
    	 pGetAmmereValRes->bigdata[2]-=VAL_PARA;

    	 pwval=0;
    	 for(c=0; c<6; c++)
    	 {
    		pwval*=10;
    		if(c%2==0)
    		{
    		  pwval+=(pGetAmmereValRes->bigdata[2-c/2]>>4);
    		}
    		else
    		{
    		 pwval+=(pGetAmmereValRes->bigdata[2-c/2]&0xF);
    		}
    	}
    	pwvalue=pwval*100+HEXBCDTODEC(pGetAmmereValRes->smdata);
    	//DBG_PRINT("power_value=%lu\n",pwvalue);
    	//term_pwval=(float)pwvalue;
    	term_pwval=(float)pwvalue;
    	term_pwval=term_pwval/100;
    	DBG_PRINT("term_pwval=%lf\n",term_pwval);
    	//printf("uint32: %"PRIu32"\n", pwvalue);

	     // DBG_PRINT("devid=");
		  for(k=0;k<TERM_LEN;k++)
		  {
			// DBG_PRINT("%02x",pAmmeteRxMsgHead->devid[k]);
			 tmp_code[k]=pAmmeteRxMsgHead->devid[TERM_LEN-k-1];
		  }
		  //DBG_PRINT("\n");

		 // DBG_PRINT("tmp_code=");
	      //for(k=0;k<TERM_LEN;k++) {
		  //DBG_PRINT("%02x",tmp_code[k]);
		  //}
		 // DBG_PRINT("\n");

		  sprintf(term_code,"%02x%02x%02x%02x%02x%02x",tmp_code[0],tmp_code[1],tmp_code[2],tmp_code[3],tmp_code[4],tmp_code[5]);

		 // Term_TxMsg = (TxMsg *)malloc(sizeof(TxMsg));

		 //  Term_TxMsg->TermCode=term_code;
		 //  Term_TxMsg->TermTime=get_systime;
		  // Term_TxMsg->ReportData=term_pwval;
		   SendmsgToclient(term_code,get_systime,term_pwval);
		  // Term_TxMsg=NULL;
		   // send  TermNumReportMsg to server

		   root=cJSON_CreateObject();
		   cJSON_AddNumberToObject(root,"MsgType", TermDataReportMsg);
		   cJSON_AddNumberToObject(root,"Sn",TermDataReportMsg_sn);
		   if(TermDataReportMsg_sn<255)
			   TermDataReportMsg_sn++;
		   else
			   TermDataReportMsg_sn=0;
		   cJSON_AddStringToObject(root,"TermCode",term_code);
		   cJSON_AddStringToObject(root,"TermTime",get_systime); //modify yanly150521
		   cJSON_AddNumberToObject(root,"ReportData",term_pwval);

		    out=cJSON_PrintUnformatted(root);
		    cJSON_Delete(root);
		    json_msgsnd(MsgserverTxId, TermDataReportMsg, out, strlen(out));
		    free(out);

	  }
      else
     {DBG_PRINT("term no return data\n");}
	 //return;
 }


void  SendmsgToclient(char *code,char *systime,float pwval)
{
	 char *out=NULL;
     cJSON* root = NULL;
     root=cJSON_CreateObject();
     cJSON_AddStringToObject(root,"TermCode",code);
     cJSON_AddStringToObject(root,"TermTime",systime);
     cJSON_AddNumberToObject(root,"ReportData",pwval);

	 if(msgfromflg==MsgComNative)
	 	{
		    msgfromflg=0;
		    term_nativercv_flag=enable;

	 	    cJSON_AddNumberToObject(root,"MsgType", TermNumReportMsg);
	 	    cJSON_AddNumberToObject(root,"Sn",native_sn);
	 	    if(native_sn<255)
	 	    	native_sn++;
	 	    else
	 	    	native_sn=0;
	 	    out=cJSON_PrintUnformatted(root);
	 	   	//DBG_PRINT("out=%s\n",out);
	 	    cJSON_Delete(root);
	 	    send_msg_to_all_client(out,strlen(out));
	 	    free(out);
	 	    DBG_PRINT("SendmsgTo_allclient\n");
	 	}
	  else if(msgfromflg==MsgComClient)
	   {
		     msgfromflg=0;
		     term_clientrcv_flag=enable;
		     cJSON_AddNumberToObject(root,"MsgType", TermQueAckMsg);
		     cJSON_AddNumberToObject(root,"Sn",Client_Sn);
		 	 out=cJSON_PrintUnformatted(root);
		 	 //DBG_PRINT("out=%s\n",out);
		 	 cJSON_Delete(root);
		 	 send_msg_to_client(out, strlen(out), Client_Socket);
		 	 free(out);
		 	 DBG_PRINT("SendmsgTo_oneclient\n");
	  }
  return ;
	// exit(0);
}

  
//fe fe fe fe 68 01 02 03 04 05 06 68 81 03 c3 43 55 99 16
 void serialMsgDeal(uint8 *msgBuf,uint8 msg_len)
 {
	 //uint8 i=0;
	 AmmeteRxMsgHead_T *pAmmeteRxMsgHead=NULL;

	 //DBG_PRINT("msgBuf=");
	 //for(i=0;i<msg_len;i++)
	 //{DBG_PRINT("%02x",msgBuf[i]);}
	 //DBG_PRINT("\n");
 
	 pAmmeteRxMsgHead=(AmmeteRxMsgHead_T*)msgBuf;

	 //DBG_PRINT("cmd_ack=%02x\n",pAmmeteRxMsgHead->cmd.bita.ack);
	 //DBG_PRINT("cmd_rtxflg=%02x\n",pAmmeteRxMsgHead->cmd.bita.rtxflg);
	 //DBG_PRINT("msgType=%02x\n",pAmmeteRxMsgHead->msgType);
	 //DBG_PRINT("pAmmeteRxMsgHead=%s\n",pAmmeteRxMsgHead);
 
	 if(pAmmeteRxMsgHead->cmd.bita.ack==0
	 &&pAmmeteRxMsgHead->cmd.bita.rtxflg==1/*��վӦ��*/)
	 {

		 if(pAmmeteRxMsgHead->cmd.bita.command==READ_AMME_DATA)
		 {
             if(pAmmeteRxMsgHead->msgType==AMMTER_GET_PWVAL_RES)
              {

            	 termGetValResClient(msgBuf);

              }
		 }

		 else if(pAmmeteRxMsgHead->cmd.bita.command==CLOSE_WATER_VALVE)
		 {
			 printf("close water ok\n");
		 }
		 else if(pAmmeteRxMsgHead->cmd.bita.command==OPEN_WATER_VALVE)
		 {
			 printf("open water ok\n");
		 }
		 return;
	 }
	 
	 return;
 }/*End of serialMsgDeal*/
 


 void unpack_term_msg(uint8 *sdata,uint8 slen)
 {
	 uint8 g_SerialdataBuf[INFOLEN]={0};
	 uint8 datalen=0;
	 
	 uint8 g_WTSerialdataBuf[INFOLEN]={0};
	 uint8 wtdatalen=0;
 
	 uint8 g_lbWTSerialdataBuf[INFOLEN]={0};
     uint8 lbwtdatalen=0;
 
	 uint8 g_lbAnnSerialdataBuf[INFOLEN]={0};
	 uint8 lbAnndatalen=0;
	 uint8 i=0;
	 AmmeteRxMsgHead_T *pAmmeteRxMsgHead=NULL;
	 WaterRxMsgHead_T *pWaterRxMsgHead=NULL;
	 lbWaterRxMsgHead_T *pLBWaterRxMsgHead=NULL;
	 lbAmmeteRxMsgHead_T *pLBAmmeteRxMsgHead=NULL;

	 uint8 *pTmp=NULL;
	 uint8 len=0;

	 memcpy(g_SerialdataBuf,sdata,slen);
	 datalen=slen;

	 //DBG_PRINT("g_SerialdataBuf=");
     //for(i=0;i<datalen;i++)
     //{DBG_PRINT("%02x",g_SerialdataBuf[i]);}
     //DBG_PRINT("\n");

	 memcpy(g_WTSerialdataBuf,sdata,slen);
	 wtdatalen=slen;
	 //DBG_PRINT("g_WTSerialdataBuf=");
	 //for(i=0;i<wtdatalen;i++)
	// {DBG_PRINT("%02x",g_WTSerialdataBuf[i]);}
	//  DBG_PRINT("\n");
 

	 memcpy(g_lbWTSerialdataBuf,sdata,slen);
	 lbwtdatalen=slen;
	 //DBG_PRINT("g_lbWTSerialdataBuf=");
     //for(i=0;i<lbwtdatalen;i++)
	 //{DBG_PRINT("%02x",g_lbWTSerialdataBuf[i]);}
	 // DBG_PRINT("\n");
 

	 memcpy(g_lbAnnSerialdataBuf,sdata,slen);
	 lbAnndatalen=slen;
	 //DBG_PRINT("g_lbAnnSerialdataBuf=");
	 //for(i=0;i<lbAnndatalen;i++)
	 //{DBG_PRINT("%02x",g_lbAnnSerialdataBuf[i]);}
     //DBG_PRINT("\n");


	 if(datalen>=lbAmmeteRxMsgHead_len+3)
	 {
		 pAmmeteRxMsgHead=(AmmeteRxMsgHead_T*)g_SerialdataBuf;

		 if(pAmmeteRxMsgHead->frmhead[0]==0xFE
			 &&pAmmeteRxMsgHead->frmhead[1]==0xFE
			 &&pAmmeteRxMsgHead->frmhead[2]==0xFE
			 &&pAmmeteRxMsgHead->frmhead[3]==0xFE
			 &&pAmmeteRxMsgHead->frmStar==0x68
			 &&pAmmeteRxMsgHead->frmEnd==0x68
			 )
		 {

			 if(datalen==pAmmeteRxMsgHead->msglen+lbAmmeteRxMsgHead_len+3-2/*msgtype*/+1/*crc*/+1/*endflg*/
				 &&g_SerialdataBuf[datalen-2]==GetCheckCS(&g_SerialdataBuf[4],datalen-4/*frmhead*/-1/*crc*/-1/*endflg*/)
				 &&g_SerialdataBuf[datalen-1]==0x16)

			 {
				 //DBG_PRINT("111111111\n");
				 //prtfBuf(GET_DEVPAV_MOD,g_SerialdataBuf,datalen,"Rx Serial data");
				 serialMsgDeal(g_SerialdataBuf,datalen);
				 datalen=0;
				 memset(g_SerialdataBuf,0,sizeof(g_SerialdataBuf));
			 }
			 else if(datalen==pAmmeteRxMsgHead->msglen+lbAmmeteRxMsgHead_len+3-2/*msgtype*/+1/*crc*/+1/*endflg*/)
			 {
				 datalen--;
				 for(i=0; i<datalen; i++)
				 {
					 g_SerialdataBuf[i]=g_SerialdataBuf[i+1];
				 }
 
				 g_SerialdataBuf[datalen]=0;
			 }			 
			 
		 }
		 else
		 {
			 datalen--;
			 for(i=0; i<datalen; i++)
			 {
				 g_SerialdataBuf[i]=g_SerialdataBuf[i+1];
			 }
 
			 g_SerialdataBuf[datalen]=0;
		 }
	 }
 
	 /*����ˮ��*/
	 if(wtdatalen>=lbAmmeteRxMsgHead_len+2)
	 {
		 pWaterRxMsgHead=(WaterRxMsgHead_T*)g_WTSerialdataBuf;

		 if(pWaterRxMsgHead->frmhead[0]==0xFE
			 &&pWaterRxMsgHead->frmhead[1]==0xFE
			 &&pWaterRxMsgHead->frmhead[2]==0xFE
			 &&pWaterRxMsgHead->frmStar==0x68
			 &&pWaterRxMsgHead->frmEnd==0x68
			 )
		 {

			 len=sizeof(WaterRxMsgHead_T);
			 //DBG_PRINT("len=%d\n",len);
			// DBG_PRINT("msglen=%d\n",pWaterRxMsgHead->msglen);
			 /*�ж�֡β*/
			 if(wtdatalen==pWaterRxMsgHead->msglen+lbAmmeteRxMsgHead_len+2-2/*msgtype*/+1/*crc*/+1/*endflg*/
				 &&g_WTSerialdataBuf[wtdatalen-2]==GetCheckCS(&g_WTSerialdataBuf[3],wtdatalen-3/*frmhead*/-1/*crc*/-1/*endflg*/)
				 &&g_WTSerialdataBuf[wtdatalen-1]==0x16)

			 {   //DBG_PRINT("22222222\n");
				// prtfBuf(GET_DEVPAV_MOD,g_WTSerialdataBuf,wtdatalen,"Rx Wat Serial data");
				 pTmp=g_WTSerialdataBuf;
				 pTmp--;
				 serialMsgDeal(pTmp,wtdatalen);
				 wtdatalen=0;
				 memset(g_WTSerialdataBuf,0,sizeof(g_WTSerialdataBuf));
			 }
			 else if(wtdatalen==pWaterRxMsgHead->msglen+lbAmmeteRxMsgHead_len+2-2/*msgtype*/+1/*crc*/+1/*endflg*/)
			 {
				 wtdatalen--;
				 for(i=0; i<wtdatalen; i++)
				 {
					 g_WTSerialdataBuf[i]=g_WTSerialdataBuf[i+1];
				 }
 
				 g_WTSerialdataBuf[wtdatalen]=0;
			 }			 
			 
		 }
		 else
		 {
			 wtdatalen--;
			 for(i=0; i<wtdatalen; i++)
			 {
				 g_WTSerialdataBuf[i]=g_WTSerialdataBuf[i+1];
			 }
 
			 g_WTSerialdataBuf[wtdatalen]=0;
		 }
	 }
 

	 if(lbwtdatalen>=lbAmmeteRxMsgHead_len+1)
	 {
		 pLBWaterRxMsgHead=(lbWaterRxMsgHead_T*)g_lbWTSerialdataBuf;

		 if(pLBWaterRxMsgHead->frmhead[0]==0xFE
			 &&pLBWaterRxMsgHead->frmhead[1]==0xFE
			 &&pLBWaterRxMsgHead->frmStar==0x68
			 &&pLBWaterRxMsgHead->frmEnd==0x68
			 )
		 {
			 if(lbwtdatalen==pLBWaterRxMsgHead->msglen+lbAmmeteRxMsgHead_len+1-2/*msgtype*/+1/*crc*/+1/*endflg*/
				 &&g_lbWTSerialdataBuf[lbwtdatalen-2]==GetCheckCS(&g_lbWTSerialdataBuf[2],lbwtdatalen-2/*frmhead*/-1/*crc*/-1/*endflg*/)
				 &&g_lbWTSerialdataBuf[lbwtdatalen-1]==0x16)
			 {   // DBG_PRINT("333333333\n");
				// prtfBuf(GET_DEVPAV_MOD,g_lbWTSerialdataBuf,lbwtdatalen,"Rx LB Wat Serial data");
				 pTmp=g_lbWTSerialdataBuf;
				 pTmp-=2;
				 serialMsgDeal(pTmp,lbwtdatalen);
				 lbwtdatalen=0;
				 memset(g_lbWTSerialdataBuf,0,sizeof(g_lbWTSerialdataBuf));
 
				 lbAnndatalen=0;
				 memset(g_lbAnnSerialdataBuf,0,sizeof(g_lbAnnSerialdataBuf));
 
			 }
			 else if(lbwtdatalen==pLBWaterRxMsgHead->msglen+lbAmmeteRxMsgHead_len+1-2/*msgtype*/+1/*crc*/+1/*endflg*/)
			 {
				 //printf("Error len:0x%bx chksun:%02bX\n",lbwtdatalen,GetCheckCS(&g_lbWTSerialdataBuf[2],lbwtdatalen-2));
				 lbwtdatalen--;
				 for(i=0; i<lbwtdatalen; i++)
				 {
					 g_lbWTSerialdataBuf[i]=g_lbWTSerialdataBuf[i+1];
				 }
 
				 g_lbWTSerialdataBuf[lbwtdatalen]=0;
			 }			 
			 
		 }
		 else
		 {
			 lbwtdatalen--;
			 for(i=0; i<lbwtdatalen; i++)
			 {
				 g_lbWTSerialdataBuf[i]=g_lbWTSerialdataBuf[i+1];
			 }
 
			 g_lbWTSerialdataBuf[lbwtdatalen]=0;
		 }
	 }
 
	 if(lbAnndatalen>=lbAmmeteRxMsgHead_len)
	 {
		 pLBAmmeteRxMsgHead=(lbAmmeteRxMsgHead_T*)g_lbAnnSerialdataBuf;

		 if(pLBAmmeteRxMsgHead->frmhead==0xFE
			 &&pLBAmmeteRxMsgHead->frmStar==0x68
			 &&pLBAmmeteRxMsgHead->frmEnd==0x68
			 )
		 {
			 if(lbAnndatalen==pLBAmmeteRxMsgHead->msglen+lbAmmeteRxMsgHead_len-2/*msgtype*/+1/*crc*/+1/*endflg*/
				 &&g_lbAnnSerialdataBuf[lbAnndatalen-2]==GetCheckCS(&g_lbAnnSerialdataBuf[1],lbAnndatalen-1/*frmhead*/-1/*crc*/-1/*endflg*/)
				 &&g_lbAnnSerialdataBuf[lbAnndatalen-1]==0x16)
			 {   //DBG_PRINT("4444444444\n");
				// prtfBuf(GET_DEVPAV_MOD,g_lbAnnSerialdataBuf,lbAnndatalen,"Rx LB Ann Serial data");
				 pTmp=g_lbAnnSerialdataBuf;
				 pTmp-=3;
				 serialMsgDeal(pTmp,lbAnndatalen);
				 lbAnndatalen=0;
				 memset(g_lbAnnSerialdataBuf,0,sizeof(g_lbAnnSerialdataBuf));
			 }
			 else if(lbAnndatalen==pLBAmmeteRxMsgHead->msglen+lbAmmeteRxMsgHead_len-2/*msgtype*/+1/*crc*/+1/*endflg*/)
			 {
				// printd("Error len:0x%bx chksun:%02bX\n",lbAnndatalen,GetCheckCS(&g_lbAnnSerialdataBuf[1],lbAnndatalen-1));
				 lbAnndatalen--;
				 for(i=0; i<lbAnndatalen; i++)
				 {
					 g_lbAnnSerialdataBuf[i]=g_lbAnnSerialdataBuf[i+1];
				 }
 
				 g_lbAnnSerialdataBuf[lbAnndatalen]=0;
			 }			 
			 
		 }
		 else
		 {
			 lbAnndatalen--;
			 for(i=0; i<lbAnndatalen; i++)
			 {
				 g_lbAnnSerialdataBuf[i]=g_lbAnnSerialdataBuf[i+1];
			 }
 
			 g_lbAnnSerialdataBuf[lbAnndatalen]=0;
		 }
	 }
	 
	 return ;
 }/*End of serialDataPreDeal*/














