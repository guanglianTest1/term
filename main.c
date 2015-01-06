
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include"net.h"
#include"json.h"
#include"cJSON.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"
#include<sys/socket.h>
#include<unistd.h>

 static pthread_t time_thread;
 static pthread_t server_thread;
 static pthread_t term_thread;
 static pthread_t node_thread;
// static pthread_t client_thread;
 static pthread_t client_type;
pthread_t thread_do[2];
//pthread_t threadtest;

extern uint8 listen_thread_isover;

//void *thread_test(void *argv)
//{
//	printf("haha\n");
//	return NULL;
//}
int main()
{
	int i;
	int s_s;

	db_init();
	sysInit();
	
	s_s= ConnectClient();

//	pthread_create(&threadtest,NULL,thread_test,NULL);

	pthread_create(&thread_do[0], NULL, handle_connect, (void*)&s_s);
	pthread_create(&thread_do[1], NULL, handle_request, NULL);

//	client_type_socket();

//	time_t_process();
//
//	term_msg_process();
//
//	node_msg_process();
//
//	server_msg_process();
//	pthread_create(&client_thread,NULL,client_msg_thread,NULL);   //���������ͨ���߳�

    
//    pthread_create(&client_type,NULL,client_type_thread,NULL);   //���ͻ�������
//
//
	pthread_create(&time_thread,NULL,time_t_thread,NULL);          //��ʱ���߳�
//
//
//	pthread_create(&term_thread,NULL,term_msg_thread,NULL);       //���ͳ�����Ϣ�߳�


	pthread_create(&node_thread,NULL,node_msg_thread,NULL);       //与网关5018建立连接，获取callback,客户端形式


//	pthread_create(&server_thread,NULL,server_msg_thread,NULL);   //���������ͨ���߳�
			


//	while (1)
//		{
//          if(listen_thread_isover ==1)
//		   listen_thread_isover = 0;
//		//DBG_PRINT("1111\n");
//        }

    //wait pthread over
    for(i=0; i<2; i++)
    	pthread_join(thread_do[i], NULL);

    close(s_s);
    return 0;
}



