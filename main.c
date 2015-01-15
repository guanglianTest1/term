
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

#include "appSqlite.h"

 static pthread_t time_thread;
 static pthread_t server_thread;
 static pthread_t term_thread;
 static pthread_t node_thread;
// static pthread_t client_thread;
 static pthread_t client_type;
pthread_t thread_do[2];
static  pthread_t check_client_thread;
//pthread_t threadtest;
//extern uint8 listen_thread_isover;

pthread_mutex_t mutex;   //互斥区
/*
 * 线程互斥的变量: char connect_host_online[MAX_CLIENT_NUM]
 * */

void wake_up_zb_serial_node();



int main()
{
	int i;
	int s_s;


	db_init();
	sysInit();
	get_local_ipaddr();//get gateway ip addr
	s_s= ConnectClient();
	wake_up_zb_serial_node();//唤醒透传节点 add by yan150114

	pthread_mutex_init(&mutex, NULL);//线程锁初始化

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


	pthread_create(&node_thread,NULL,node_msg_thread,NULL);       //与网关5002建立连接，获取callback,客户端形式


//	pthread_create(&server_thread,NULL,server_msg_thread,NULL);   //���������ͨ���߳�
			
	pthread_create(&check_client_thread,NULL,check_client_heartbeat,NULL);

//	while (1)
//		{
//          if(listen_thread_isover ==1)
//		   listen_thread_isover = 0;
//		//DBG_PRINT("1111\n");
//        }


    //wait pthread over
    for(i=0; i<2; i++)
    	pthread_join(thread_do[i], NULL);
    pthread_join(node_thread, NULL);
    pthread_join(time_thread, NULL);
    pthread_join(check_client_thread, NULL);

    pthread_mutex_destroy(&mutex);  /*销毁互斥*/

    close(s_s);
    return 0;
}

void wake_up_zb_serial_node()
{
	char sql[] = "select distinct nwkaddr from stable where nwkaddr not null";
	char **data;
	int row,col;
	int i,index;
	data = sqlite_query_msg(&row, &col, sql);

	if(data!= NULL)
	{
		for(i=0; i<row; i++)
		{
			index = i*col+col;
			send_data_to_dev_security(data[index], "wake up",8);
			printf("nwkaddr:%s\n",data[index]);
		}
	}
	sqlite_free_query_result(data);
}

