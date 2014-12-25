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

 int time_num=0,t=1;                //time_num����ʱ���ĸ�����t��ʾʱ�䣬�������

 //uint8 time1_flag=disable;
// uint8 time2_flag=disable;


 uint8 time1_flag=disable;
 uint8 time2_flag=disable;
 //uint8 time3_flag=disable;
 uint8 client_flag=disable;
 uint8 node_heart_flag=disable;
 uint8 server_heart_flag=disable;
 
 void setTimer(int t,int f)       //�½�һ����ʱ��
 { 
 
	 struct Timer a;
	 a.total_time=t;
	 a.left_time=t;
	 a.func=f;
	 myTimer[time_num++]=a;
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
			   	 DBG_PRINT("time1_flag= %d\n",time1_flag);
			     break;
			case time2:
				 time2_flag=enable;  
				 DBG_PRINT("time2_flag= %d\n",time2_flag);
			     break;
		    case time3:
		    	//DBG_PRINT("3333\n");
				//time3_flag=enable;       //40S��ڵ��豸������������־
				 client_flag=enable;     //40S��ѯ�ͻ���������Ϣ
				 node_heart_flag=enable;
				 server_heart_flag=enable;
				 DBG_PRINT("client_flag= %d\n",client_flag);
				 DBG_PRINT("node_heart_flag= %d\n",node_heart_flag);
				 DBG_PRINT("server_heart_flag= %d\n",server_heart_flag);
				 break;
		     
				 	 
		   	  default:break;  
			
			}
			
			myTimer[j].left_time=myTimer[j].total_time; //ѭ����ʱ
		}
	 }
 }
  
 
 void *time_t_thread(void *p)     
 { 
 
	 setTimer(60,time1);              //���һ���ӳ�һ����
	 setTimer(86400,time2);          //һ�쳭��һ��
	 //setTimer(6,time1);                //test
	 //setTimer(10,time2);               //test
	 
	 setTimer(20,time3);              //���40����ڵ��豸����������Ϣ��
	// DBG_PRINT("aaaaa\n");
	 
	 signal(SIGALRM,timeout);         //�ӵ�SIGALRM�źţ���ִ��timeout����
	 
 
	 while(1)
	 { 
 
		sleep(1);                    //ÿ��һ�뷢��һ��SIGALRM
 
		kill(getpid(),SIGALRM);
	 }
	 exit(0);
 }
 
 
