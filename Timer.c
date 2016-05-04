/*
 * cpkTimer.c
 *
 *  Created on: Mar 19, 2014
 *      Author: ema
 */

#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
//#include<timer.h>
#include<sys/time.h>
#include <stdio.h>
#include <string.h>
 
#include"json.h"
#include"cJSON.h"
#include"net.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"
#include "appSqlite.h"
#include "user_config.h"

 int time_num=0,t=1;                //time_num����ʱ���ĸ�����t��ʾʱ�䣬�������

 //uint8 time1_flag=disable;
// uint8 time2_flag=disable;


 volatile uint8 time1_flag=disable;
 volatile uint8 time2_flag=disable;
 volatile uint8 time4_flag=disable;
  //uint8 time3_flag=disable;
  volatile uint8 client_flag=disable;
  volatile uint8 node_heart_flag=disable;
  volatile uint8 server_heart_flag=disable;
  extern node_native nodetable_native[NODE_NUMM];
  int Term_rcvPeriod=TERM_PEIOD;  //24 hour
  extern volatile int server_connected_fd;
 
 void setTimer(int t,int f)       //�½�һ����ʱ��
 { 
 
	 struct Timer a;
	 a.total_time=t;
	 a.left_time=t;
	 a.func=f;
	 myTimer[time_num++]=a;
 }
  
 void ModTimer(int newtime,int timeid)       //�½�һ����ʱ��
  {
	 int j=0;
	 for(j=0;j<time_num;j++)
	 {
		 if(myTimer[j].func==timeid)
		 {
			 myTimer[j].total_time=newtime;
			 myTimer[j].left_time=newtime;
			 break;
	 	 }
	 }
  }

 void timeout()                 //�ж϶�ʱ���Ƿ�ʱ���Լ���ʱʱ��Ҫִ�еĶ���
 {
	 int j;

	 //uint8 *termcode;
	 //uint8 termtype;
	// msgform term_msg;
	 //printf("Time: %d\n",t++);
	 for(j=0;j<time_num;j++)
	 { 
 
		if(myTimer[j].left_time!=0)
			myTimer[j].left_time--;
		else
		{ 
 
			switch(myTimer[j].func)     //ͨ��ƥ��myTimer[j].func���ж���һ��ѡ�����ֲ���
			{	   
			case time1:
			   	 time1_flag=enable;
     		   	 //DBG_PRINT("time1_flag= %d\n",time1_flag);//屏蔽 yanly150108
			     break;
			case time2:
				 time2_flag=enable;  
				 sqlite_query_to_native();  //add by yang 150522
    			 //DBG_PRINT("time2_flag= %d\n",time2_flag);//屏蔽 yanly150108
			     break;
		    case time3:
		    	//DBG_PRINT("3333\n");
				//time3_flag=enable;       //40S��ڵ��豸������������־
				 client_flag=enable;     //40S��ѯ�ͻ���������Ϣ
				 node_heart_flag=enable;
				 server_heart_flag=enable;
				 if(server_connected_fd)   //如果没有连接到服务器，不应该向队列发送心跳包。
					 send_server_heartbeat();
				 break;
		    case time4:
		    	 time4_flag=enable;       //40S��ڵ��豸�����������־
		    	 break;
		   	  default:break;  
			
			}
			
			myTimer[j].left_time=myTimer[j].total_time; //ѭ����ʱ
		}
	 }
 }
  
#if 1
 void *time_t_thread(void *p)     
 { 

	 setTimer(TERM_INTER,time1);                //test
	 setTimer(Term_rcvPeriod,time2);               //test
	 //DBG_PRINT("Term_rcvPeriod=%d\n",Term_rcvPeriod);
	 setTimer(HEART_TIME,time3);              //40心跳
	 setTimer(CLIENT_TIME,time4);              //40心跳
	// DBG_PRINT("aaaaa\n");
	 if(nodetable_native[0].termtable_native[0].TermPeriod_native>0)
	 	 {
	 		Term_rcvPeriod=60*nodetable_native[0].termtable_native[0].TermPeriod_native;
	 	    //printf("Term_rcvPeriod=%d\n",Term_rcvPeriod);
	 	 }
	    else
	 	 {
	 	   Term_rcvPeriod=TERM_PEIOD;
	 	   //printf("Term_rcvPeriod=%d\n",Term_rcvPeriod);
	 	 }
	 	ModTimer(Term_rcvPeriod,time2);
	 
	 signal(SIGALRM,timeout);         //�ӵ�SIGALRM�źţ���ִ��timeout����
	 
 
	 while(1)
	 { 
 
		sleep(1);                    //ÿ��һ�뷢��һ��SIGALRM
 
		kill(getpid(),SIGALRM);
	 }
	 exit(0);
 }
 
#endif
 
