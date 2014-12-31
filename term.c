#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include"json.h"
#include"cJSON.h"
#include"net.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"

extern char get_systime[64];
//extern uint8 oneclient_send_flag;
//extern uint8 allclient_send_flag;

//extern int MsgtermTxId; //���ͳ������ݵ���Ϣ����
//extern int MsgtermRxId; //���ܳ������ݵ���Ϣ����
extern int MsgserverTxId;//���ͷ��������ݵ���Ϣ����

extern uint8 time1_flag;
extern uint8 time2_flag;
extern node_list node_table[NODE_NUM];
extern uint16 client_num;
extern client_status client_list[MAX_CLIENT_NUM];
extern uint8 Client_Sn;
extern int Client_Socket;

uint8 term_send_flag=enable;
uint8 term_rcv_flag=disable;
uint8 msgfromflg=0;
//uint8 client_rcv_flag=0;
//uint8 native_rcv_flag=0;

uint8 client_send_count=0;
uint8 native_send_count=0;
uint8 nnum=0;
uint8 tnum=0;
extern uint8 Term_Num[NODE_NUM];          //���ÿ���ڵ�����ܱ����

uint8 native_sn=0;
uint32 pwvalue=0;

//term_list term_tab[TERM_NUM]={{"123",1,"ammeter",60,0,0,0,0,0,0,0,0},{"456",2,"lengwater",60,0,0,0,0,0,0,0,0}, {"789",2,"lengwater",60,0,0,0,0,0,0,0,0},{0},{0}};

void term_msg_process()
{
	pthread_t term_thread;
	pthread_create(&term_thread,NULL,term_msg_thread,NULL);
}

void *term_msg_thread(void *p)
{
  //msgform term_msg;
  //uint8 msgsize;
  uint8 term_code[TERM_LEN]={0};
  uint8 node_id[TERM_LEN]={0};
  uint8 i=0;
  while(1)
  {

#if 1

     if((time2_flag==enable)&&(time1_flag==enable))
	 // if(0)
      	{
    	  time1_flag=disable;
    	  DBG_PRINT("1111\n");
		 if(nnum==0&&tnum==0)
			{
              if(term_send_flag==enable)  
               {
                 
            	     DBG_PRINT("2222\n");
				     memcpy(term_code,node_table[nnum].term_table[tnum].TermCode,TERM_LEN);
            	     memcpy(node_id,node_table[nnum].EnergyNodeID,NODE_LEN);
				    // memcpy(term_code,node_table[nnum].term_table[tnum].TermCode,sizeof(node_table[nnum].term_table[tnum].TermCode));
				    // memcpy(node_id,node_table[nnum].EnergyNodeID,sizeof(node_table[nnum].EnergyNodeID));
				     DBG_PRINT("node_id= %s\n",node_id);
				     DBG_PRINT("termcode=\n");
				     for(i=0;i<TERM_LEN;i++)
		             {DBG_PRINT("%02x",term_code[i]);}
				     DBG_PRINT("\n");
				   	 native_send_count++;
					 if(native_send_count<=3)
					  {
						 DBG_PRINT("3333\n");
				         if(term_rcv_flag==enable)
				          {
				           tnum++;
				           DBG_PRINT("tnum=%d\n",tnum);
				           term_rcv_flag=disable;
				           native_send_count=0;
				           term_send_flag=disable;
				           DBG_PRINT("4444\n");
				          // SendmsgToclient();
				          }
				         else
				          {
				           msgfromflg=MsgComNative;
				           SendDataToDev(term_code,node_id);
				          }

                      }	

                     else
                     {
                     // DBG_PRINT("6666\n");
                      tnum++;
                      DBG_PRINT("term_num=%d\n",tnum);
                      DBG_PRINT("node_num=%d\n",nnum);
                      native_send_count=0;
					  term_send_flag=disable; 
                      //���������ߵĿͻ��˺ͷ��������ͳ�����Ӧʧ����Ϣ
					 }
              	}
		 	  }
		   else
		   	{
			  DBG_PRINT("7777\n");
              if(nnum<NODE_NUM)
              	{
            	  DBG_PRINT("8888\n");
                  if(tnum<Term_Num[nnum])
					  { 
                	   // DBG_PRINT("9999\n");

		                memcpy(term_code,node_table[nnum].term_table[tnum].TermCode,TERM_LEN);
		                memcpy(node_id,node_table[nnum].EnergyNodeID,NODE_LEN);
                	   // memcpy(term_code,node_table[nnum].term_table[tnum].TermCode,sizeof(node_table[nnum].term_table[tnum].TermCode));
                	 //	memcpy(node_id,node_table[nnum].EnergyNodeID,sizeof(node_table[nnum].EnergyNodeID));
		                DBG_PRINT("node_id= %s\n",node_id);
		                DBG_PRINT("term_code= %s\n",term_code);
				   	    native_send_count++;
					    if(native_send_count<=3)
					     {
				          if(term_rcv_flag==enable)
				           {
				           // SendmsgToclient(); //���������ߵĿͻ��˺ͷ��������ͳ�����Ӧ��Ϣ
						    tnum++;
						    term_rcv_flag=disable;
						    DBG_PRINT("term_rcv_flag= %d\n",term_rcv_flag);
						    native_send_count=0;
				           }
				          else
				           {
				        	  msgfromflg=MsgComNative;
				        	  SendDataToDev(term_code,node_id);
				           }
                         }	

                        else
                        {
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
				}
		    }
      	}
#endif
  }
  return 0;
  //exit(0);

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
	 DBG_PRINT("ret= %d\n",ret);
	return ret;
}/*End of GetCheckCS*/




uint8 SendDataToDev(uint8 *termid,uint8 *nodeid)
{

	uint8 i=0;
	uint8 datalen=0;
	//uint8 data_Buf[256];
	uint8 dataBuf[MAXBUF]={0};
	uint8 ret=0;


	dataBuf[datalen++]=0xFE;
	dataBuf[datalen++]=0xFE;
	dataBuf[datalen++]=0xFE;

	dataBuf[datalen++]=0x68;

	for(i=0;i<TERM_LEN;i++)
	{
		dataBuf[datalen++]=termid[TERM_LEN-i-1];
	}

	dataBuf[datalen++]=0x68;

	dataBuf[datalen++]=READ_AMME_DATA;

	dataBuf[datalen++]=0x02; // msg_len

	dataBuf[datalen++]=((AMMTER_GET_PWVAL_REQ>>8)&0xff);
	dataBuf[datalen++]=(AMMTER_GET_PWVAL_REQ&0xff);

    ret=GetCheckCS(&dataBuf[3],datalen-3);
	dataBuf[datalen++]=ret;

	dataBuf[datalen++]=0x16;


	DBG_PRINT("datalen=%d\n",datalen);
	DBG_PRINT("dataBuf=");

	for(i=0;i<datalen;i++)
	{
		DBG_PRINT("%0x-",dataBuf[i]);
	}
	DBG_PRINT("\n");
	child_doit(dataBuf,datalen,nodeid);
//#endif
	return datalen;
/*FE FE FE 68 42 25 11 00 00 00 68 01 02 43 C3 51 16*/	
}/*End of SendDataToDev*/
void send_data_to_dev_security(char *nwkaddr, char *text, int text_len)
{
	child_doit(text,text_len,nwkaddr);
}


void termGetValResClient(uint8 *msgBuf)
  {
	  uint8 i=0,j=0,c=0,k=0,rc=0;
	  uint32 pwval=0;
	  //uint32 pwvalue=0;
	  GetAmmereValRes_T *pGetAmmereValRes=NULL;
	  AmmeteRxMsgHead_T *pAmmeteRxMsgHead=NULL;
	  msgform msgsed;
	  TxMsg Term_TxMsg;
	  uint8 term_code[TERM_LEN]={0};
	  uint8 tmp_code[TERM_LEN]={0};


	 // DBG_PRINT("msgBuf=%s\n",msgBuf);
	  //DBG_PRINT("msgBuf=");
	  //for(i=0;i<sizeof(msgBuf);i++)
	 // {DBG_PRINT("%02x",msgBuf[i]);}
	 //  DBG_PRINT("\n");
	  pAmmeteRxMsgHead=(AmmeteRxMsgHead_T*)msgBuf;
	  pGetAmmereValRes=(GetAmmereValRes_T*)msgBuf;


     if((pAmmeteRxMsgHead->msglen)>2)
     {
    	 DBG_PRINT("smdata=%02x\n",pGetAmmereValRes->smdata);
    	 DBG_PRINT("bigdata=");
    	 for(k=0;k<3;k++)
    	 {
    		DBG_PRINT("%02x",pGetAmmereValRes->bigdata[k]);
    	 }
    	 DBG_PRINT("\n");

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

		DBG_PRINT("power_value=%lu\n",pwvalue);
    	//printf("uint32: %"PRIu32"\n", pwvalue);


	  for(i=0;i<NODE_NUM; i++)
	    {
		  for(j=0;j<TERM_NUM; j++)
		  {
			  DBG_PRINT("devid=");
			  for(k=0;k<TERM_LEN;k++)
			  {
			  	DBG_PRINT("%02x",pAmmeteRxMsgHead->devid[k]);
			  	tmp_code[k]=pAmmeteRxMsgHead->devid[TERM_LEN-k-1];
			  }
			  DBG_PRINT("\n");

			  memcpy(term_code,node_table[i].term_table[j].TermCode,TERM_LEN);
			  DBG_PRINT("term_code=");
			  for(k=0;k<TERM_LEN;k++)
			  {DBG_PRINT("%02x",term_code[k]);}
			   DBG_PRINT("\n");


			   DBG_PRINT("tmp_code=");
			   for(k=0;k<TERM_LEN;k++)
			   {
			 	 DBG_PRINT("%02x",tmp_code[k]);
			   }
			   DBG_PRINT("\n");

			  rc=memcmp(tmp_code,term_code,TERM_LEN);
			  DBG_PRINT("rc= %d\n",rc);
			  if(rc==0)
			  {
				  // DBG_PRINT("***1111");

                   memcpy(Term_TxMsg.TermCode,node_table[i].term_table[j].TermCode,TERM_LEN);  //�����ܱ���

                   Term_TxMsg.TermType=node_table[i].term_table[j].TermType; //�����ܱ�����
				   Term_TxMsg.ReportData=pwvalue;
				   //DBG_PRINT("***2222");
				   node_table[i].term_table[j].ReportData=pwvalue;
				   pwvalue=0;


				   memset(msgsed.mtext,0,MAXBUF);
				   msgsed.mtype=TermDataReportMsg;
				   memcpy(msgsed.mtext,&Term_TxMsg,sizeof(Term_TxMsg));

				   DBG_PRINT("msgsed.mtext=");
				  for(k=0;k<sizeof(Term_TxMsg);k++)
				  {DBG_PRINT("%02x",msgsed.mtext[k]);}
				  DBG_PRINT("\n");
				  msgsnd(MsgserverTxId, &msgsed, sizeof(msgform),0);
				  SendmsgToclient(Term_TxMsg);

			  }

		   }
	    }
     }
     else
     {DBG_PRINT("term no return data\n");}
	 return;
 }


void  SendmsgToclient(TxMsg TmpBuf)
{
	 char *out=NULL;
     cJSON* root = NULL;
	 char Tmp_code[TERM_LEN]={0};
	 char termc[TERM_LEN]={0};
     char *term_code=NULL;
     uint8 i=0;
     DBG_PRINT("SendmsgToclient\n");

	// Term_TxMsg=&TmpBuf;
	 memcpy(Tmp_code,TmpBuf.TermCode,TERM_LEN);

	 sprintf(termc,"%02x%02x%02x%02x%02x%02x",Tmp_code[0],Tmp_code[1],Tmp_code[2],Tmp_code[3],Tmp_code[4],Tmp_code[5]);
	 printf("termc=%s\n",termc);
	 term_code=termc;
	// printf("term_code=%s\n",term_code);
	 if(msgfromflg==MsgComNative)
	 	{   msgfromflg=0;
	 		root=cJSON_CreateObject();
	 	    cJSON_AddNumberToObject(root,"msg_Type", TermQueAckMsg);
	 	    cJSON_AddNumberToObject(root,"msg_sn",native_sn);

	 	    cJSON_AddStringToObject(root,"Term_code",term_code);
	 	   // cJSON_AddStringToObject(root,"Term_code",termc);
	        cJSON_AddNumberToObject(root,"Term_type",TmpBuf.TermType);
	 	    cJSON_AddStringToObject(root,"Term_time",get_systime);
	 	    cJSON_AddNumberToObject(root,"Term_data",TmpBuf.ReportData);
	 	    out=cJSON_PrintUnformatted(root);
	 	   	DBG_PRINT("out=%s\n",out);
	 	    cJSON_Delete(root);
	        for(i=0;i<client_num;i++)
	       	{
	       	     if(client_list[i].client_alive==online)
	       		{
	       	        send(client_list[i].client_socket,out, strlen(out), MSG_DONTWAIT);
	       	        //allclient_send_flag=disable;
	       	    }
	       	}
	 	}
	  else if(msgfromflg==MsgComClient)
	   {
		     msgfromflg=0;
		     root=cJSON_CreateObject();
		     cJSON_AddNumberToObject(root,"msg_Type", TermQueAckMsg);
		     cJSON_AddNumberToObject(root,"msg_sn",Client_Sn);

		     cJSON_AddStringToObject(root,"Term_code",term_code);
		     //cJSON_AddStringToObject(root,"Term_code",termc);
		     cJSON_AddNumberToObject(root,"Term_type",TmpBuf.TermType);
		     cJSON_AddStringToObject(root,"Term_time",get_systime);
		 	 cJSON_AddNumberToObject(root,"Term_data",TmpBuf.ReportData);
		 	 out=cJSON_PrintUnformatted(root);
		 	 DBG_PRINT("out=%s\n",out);
		 	 cJSON_Delete(root);
		 	 send(Client_Socket,out, strlen(out), MSG_DONTWAIT);
	  }
  return ;
	// exit(0);
}


  
//fe fe fe fe 68 01 02 03 04 05 06 68 81 03 c3 43 55 99 16
 void serialMsgDeal(uint8 *msgBuf,uint8 msg_len)
 {
	 uint8 i=0;
	 AmmeteRxMsgHead_T *pAmmeteRxMsgHead=NULL;

	 DBG_PRINT("msgBuf=");
	 for(i=0;i<msg_len;i++)
	 {DBG_PRINT("%02x",msgBuf[i]);}
	 DBG_PRINT("\n");
 
	 pAmmeteRxMsgHead=(AmmeteRxMsgHead_T*)msgBuf;

	 DBG_PRINT("cmd_ack=%02x\n",pAmmeteRxMsgHead->cmd.bita.ack);
	 DBG_PRINT("cmd_rtxflg=%02x\n",pAmmeteRxMsgHead->cmd.bita.rtxflg);
	 DBG_PRINT("msgType=%02x\n",pAmmeteRxMsgHead->msgType);
	 //DBG_PRINT("pAmmeteRxMsgHead=%s\n",pAmmeteRxMsgHead);
 
	 if(pAmmeteRxMsgHead->cmd.bita.ack==0
	 &&pAmmeteRxMsgHead->cmd.bita.rtxflg==1/*��վӦ��*/)
	 {

		 if(pAmmeteRxMsgHead->cmd.bita.command==READ_AMME_DATA)
		 {
             if(pAmmeteRxMsgHead->msgType==AMMTER_GET_PWVAL_RES)
              {
            	 termGetValResClient(msgBuf);
            	 term_rcv_flag=enable;
            	 DBG_PRINT("term_rcv_flag=%d\n",term_rcv_flag);
              }
		 }
#if 0
			 switch(pAmmeteRxMsgHead->msgType)
			 {
				 case AMMTER_GET_PWVAL_RES:/*��ѯ����*/

					 termGetValResClient(msgBuf);

					// if(msgfromflg==client)
                 //    {
				//	   client_rcv_flag=enable;
				//	   termGetValResClient(msgBuf);
					   
                //     }
				//	 else if(msgfromflg==native)
				//	 {
                 //       native_rcv_flag=enable;
				//	    termGetValResNative(msgBuf);
				//	 }

					 break;
				 case AMMTER_GET_ADDR_RES:/*������ַ��Ӧ*/
					// ammterGetPWDevIdRes(msgBuf);
					 break;
				 default:
					 break;
			 }
#endif
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
	 uint8 g_SerialdataBuf[64];
	 uint8 datalen=0;
	 
	 uint8 g_WTSerialdataBuf[64];
	 uint8 wtdatalen=0;
 
	 uint8 g_lbWTSerialdataBuf[64];
     uint8 lbwtdatalen=0;
 
	 uint8 g_lbAnnSerialdataBuf[64];
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

	 DBG_PRINT("g_SerialdataBuf=");
     for(i=0;i<datalen;i++)
     {DBG_PRINT("%02x",g_SerialdataBuf[i]);}
     DBG_PRINT("\n");

	 memcpy(g_WTSerialdataBuf,sdata,slen);
	 wtdatalen=slen;
	 DBG_PRINT("g_WTSerialdataBuf=");
	 for(i=0;i<wtdatalen;i++)
	 {DBG_PRINT("%02x",g_WTSerialdataBuf[i]);}
	  DBG_PRINT("\n");
 

	 memcpy(g_lbWTSerialdataBuf,sdata,slen);
	 lbwtdatalen=slen;
	 DBG_PRINT("g_lbWTSerialdataBuf=");
     for(i=0;i<lbwtdatalen;i++)
	 {DBG_PRINT("%02x",g_lbWTSerialdataBuf[i]);}
	  DBG_PRINT("\n");
 

	 memcpy(g_lbAnnSerialdataBuf,sdata,slen);
	 lbAnndatalen=slen;
	 DBG_PRINT("g_lbAnnSerialdataBuf=");
	 for(i=0;i<lbAnndatalen;i++)
	 {DBG_PRINT("%02x",g_lbAnnSerialdataBuf[i]);}
     DBG_PRINT("\n");



	 DBG_PRINT("0000000000\n");
	 if(datalen>=lbAmmeteRxMsgHead_len+3)
	 {
		 pAmmeteRxMsgHead=(AmmeteRxMsgHead_T*)g_SerialdataBuf;
		// DBG_PRINT("111111****\n");
		 /*�ж�֡ͷ*/
		 if(pAmmeteRxMsgHead->frmhead[0]==0xFE
			 &&pAmmeteRxMsgHead->frmhead[1]==0xFE
			 &&pAmmeteRxMsgHead->frmhead[2]==0xFE
			 &&pAmmeteRxMsgHead->frmhead[3]==0xFE
			 &&pAmmeteRxMsgHead->frmStar==0x68
			 &&pAmmeteRxMsgHead->frmEnd==0x68
			 )
		 {
			// DBG_PRINT("111111####\n");
			 /*�ж�֡β*/
			 if(datalen==pAmmeteRxMsgHead->msglen+lbAmmeteRxMsgHead_len+3-2/*msgtype*/+1/*crc*/+1/*endflg*/
				// &&g_SerialdataBuf[datalen-2]==GetCheckCS(&g_SerialdataBuf[4],datalen-4/*frmhead*/-1/*crc*/-1/*endflg*/)
				 &&g_SerialdataBuf[datalen-1]==0x16)

			 {
				 DBG_PRINT("111111111\n");
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
		 //DBG_PRINT("222222****\n");
		 /*�ж�֡ͷ*/
		 if(pWaterRxMsgHead->frmhead[0]==0xFE
			 &&pWaterRxMsgHead->frmhead[1]==0xFE
			 &&pWaterRxMsgHead->frmhead[2]==0xFE
			 &&pWaterRxMsgHead->frmStar==0x68
			 &&pWaterRxMsgHead->frmEnd==0x68
			 )
		 {
			 //DBG_PRINT("222222####\n");
			 len=sizeof(WaterRxMsgHead_T);
			 DBG_PRINT("len=%d\n",len);
			 DBG_PRINT("msglen=%d\n",pWaterRxMsgHead->msglen);
			 /*�ж�֡β*/
			 if(wtdatalen==pWaterRxMsgHead->msglen+lbAmmeteRxMsgHead_len+2-2/*msgtype*/+1/*crc*/+1/*endflg*/
			//	 &&g_WTSerialdataBuf[wtdatalen-2]==GetCheckCS(&g_WTSerialdataBuf[3],wtdatalen-3/*frmhead*/-1/*crc*/-1/*endflg*/)
				 &&g_WTSerialdataBuf[wtdatalen-1]==0x16)

			 {   DBG_PRINT("22222222\n");
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
 
	 /*�����ɱ������ӿƼ����޹�˾ ����Զ��485����ˮ�� 20111017��ˮ��*/
	 if(lbwtdatalen>=lbAmmeteRxMsgHead_len+1)
	 {
		 pLBWaterRxMsgHead=(lbWaterRxMsgHead_T*)g_lbWTSerialdataBuf;
		 //DBG_PRINT("333333****\n");
		 /*�ж�֡ͷ*/
		 if(pLBWaterRxMsgHead->frmhead[0]==0xFE
			 &&pLBWaterRxMsgHead->frmhead[1]==0xFE
			 &&pLBWaterRxMsgHead->frmStar==0x68
			 &&pLBWaterRxMsgHead->frmEnd==0x68
			 )
		 {   // DBG_PRINT("333333####\n");
			 /*�ж�֡β*/
			 if(lbwtdatalen==pLBWaterRxMsgHead->msglen+lbAmmeteRxMsgHead_len+1-2/*msgtype*/+1/*crc*/+1/*endflg*/
				// &&g_lbWTSerialdataBuf[lbwtdatalen-2]==GetCheckCS(&g_lbWTSerialdataBuf[2],lbwtdatalen-2/*frmhead*/-1/*crc*/-1/*endflg*/)
				 &&g_lbWTSerialdataBuf[lbwtdatalen-1]==0x16)
			 {    DBG_PRINT("333333333\n");
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
 
 
	 /*�����ɱ������ӿƼ����޹�˾ ����Զ��485����ˮ�� 20111017��ˮ��*/
	 if(lbAnndatalen>=lbAmmeteRxMsgHead_len)
	 {
		 pLBAmmeteRxMsgHead=(lbAmmeteRxMsgHead_T*)g_lbAnnSerialdataBuf;
		// DBG_PRINT("444444****\n");
		 /*�ж�֡ͷ*/
		 if(pLBAmmeteRxMsgHead->frmhead==0xFE
			 &&pLBAmmeteRxMsgHead->frmStar==0x68
			 &&pLBAmmeteRxMsgHead->frmEnd==0x68
			 )
		 {  //  DBG_PRINT("444444#####\n");
			 /*�ж�֡β*/
			 if(lbAnndatalen==pLBAmmeteRxMsgHead->msglen+lbAmmeteRxMsgHead_len-2/*msgtype*/+1/*crc*/+1/*endflg*/
				 //&&g_lbAnnSerialdataBuf[lbAnndatalen-2]==GetCheckCS(&g_lbAnnSerialdataBuf[1],lbAnndatalen-1/*frmhead*/-1/*crc*/-1/*endflg*/)
				 &&g_lbAnnSerialdataBuf[lbAnndatalen-1]==0x16)
			 {   DBG_PRINT("4444444444\n");
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


#if 0

 int digit2time(char *timeStr, int iLen, tm *ptTime)
 {
	 char	 *pChar = timeStr;
 
	 memset(ptTime, 0, sizeof(*ptTime));
 
	 if (iLen < 6 || iLen % 2 != 0)
	 {
		 return 0;
	 }
 
	 
	 /* ������ʽ, ֻ��: YYYYMMDDHHmmss */
	 if (iLen >= 14)
	 {
		 ptTime->tm_year = (pChar[0] - '0') * 1000 + (pChar[1] - '0') * 100 + 
			 (pChar[2] - '0') * 10 + (pChar[3] - '0');
		 ptTime->tm_mon  = (pChar[4] - '0') * 10 + (pChar[5] - '0');
		 ptTime->tm_mday = (pChar[6] - '0') * 10 + (pChar[7] - '0');
		 ptTime->tm_hour = (pChar[8] - '0') * 10 + (pChar[9] - '0');
		 ptTime->tm_min  = (pChar[10] - '0') * 10 + (pChar[11] - '0');
		 ptTime->tm_sec  = (pChar[12] - '0') * 10 + (pChar[13] - '0');
		 if (ptTime->tm_year >= 1900 && ptTime->tm_year < 2036 &&
			 ptTime->tm_mon < 13 && ptTime->tm_hour < 24 && ptTime->tm_min < 60 && ptTime->tm_sec < 60)
		 {
			 return FLAG_SEC | FLAG_MIN | FLAG_HOUR | 
				 FLAG_DAY | FLAG_MONTH | FLAG_YEAR;
		 }
	 }
     return 0;

 	}

#endif












