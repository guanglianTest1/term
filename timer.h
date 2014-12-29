#ifndef _Timerr_H
#define _Timerr_H

#include"sysinit.h"

#define N 100            //�������Ķ�ʱ����

#define time1 1
#define time2 2
#define time3 3
#define time4 4

#define enable 1
#define disable 0

#define native 2
#define client 1



struct Timer           //Timer�ṹ�壬��������һ����ʱ������Ϣ
 { 
 
	 int total_time;   //ÿ��total_time��
	 int left_time;    //��ʣleft_time��
	 int func;         //�ö�ʱ����ʱ��Ҫִ�еĴ���ı�־
 }myTimer[N];          //����Timer���͵����飬�����������еĶ�ʱ��
  

 


void setTimer(int t,int f);
void timeout()	;
void *time_t_thread(void *p) ;
void time_t_process();


#endif
