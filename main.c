
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "net.h"
#include "json.h"
#include "cJSON.h"
#include "timer.h"
#include "HttpModule.h"
#include "term.h"
#include "sysinit.h"
#include <sys/socket.h>
#include <unistd.h>

#include "appSqlite.h"
#include "user_config.h"

///////////////////////////////////
//////////////////////////////////
 static pthread_t time_thread;
 static pthread_t server_thread;
 static pthread_t term_thread;
 static pthread_t node_thread;
 static pthread_t client_thread;
//static pthread_t client_term;
pthread_t thread_do[2];
static  pthread_t check_client_thread;
//pthread_t threadtest;
//extern uint8 listen_thread_isover;

pthread_mutex_t mutex;   //互斥区
/*
 * 线程互斥的变量: char connect_host_online[MAX_CLIENT_NUM]
 * */

void wake_up_zb_serial_node();
void wake_up_zbenergy_serial_node();
void wati_network_init_ok();
//extern node_list node_table[];

int main()
{
	int i;
	int s_s;
	int opt=2;//默认撤防
	printf("[%s: %s]-----------------------\n",__DATE__, __TIME__);
	printf("-----------------------%s-----------------------\n",SPECIAL_PROJECT_VERSION);
	GDGL_DEBUG("Delay 5 second\n");
	sleep(8);//add by yanly150528
	wati_network_init_ok(); //add by yanly150528
	db_init();
	sqlite_query_to_native();
	sysInit();
	//get_local_ipaddr();//get gateway ip addr
	s_s= ConnectClient();
	http_get_localIASCIEOperation(&opt);  //获取全局布撤防状态 add by yan150512
	set_global_opertor_status((char)opt);
	wake_up_zb_serial_node();//唤醒透传节点 add by yan150114
	wake_up_zbenergy_serial_node(); //唤醒energy透传节点 add by yang150130

	pthread_mutex_init(&mutex, NULL);//线程锁初始化
	pthread_create(&time_thread,NULL,time_t_thread,NULL);          //定时器单元

	pthread_create(&thread_do[0], NULL, handle_connect, (void*)&s_s);//客户端连接单元
	pthread_create(&thread_do[1], NULL, handle_request, NULL);//客户端业务单元

    pthread_create(&client_thread,NULL,client_term_thread,NULL);   //���ͻ�������

	pthread_create(&node_thread,NULL,node_msg_thread,NULL);       //与网关5018单元

#ifdef USE_SERVER_THREAD
	pthread_create(&server_thread,NULL,server_msg_thread,NULL);   //与云代理5040单元
#endif
	pthread_create(&check_client_thread,NULL,check_client_heartbeat,NULL);

	pthread_create(&term_thread,NULL,term_msg_thread,NULL);       //���ͳ�����Ϣ�߳�

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

    pthread_join(client_thread, NULL);
    pthread_join(term_thread, NULL);
    pthread_join(server_thread, NULL);

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
//			printf("nwkaddr:%s\n",data[index]);
		}
	}
	sqlite_free_query_result(data);
}

void wake_up_zbenergy_serial_node()
{
	char sql[] = "select distinct nwkaddr from etable where nwkaddr not null";
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
			//printf("nwkaddr:%s\n",data[index]);
		}
	}
	sqlite_free_query_result(data);
}
void wati_network_init_ok()
{
	int fd;
	struct sockaddr_in	servaddr;

	while ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        GDGL_DEBUG("socket error\n");
		sleep(1);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5018);

	while ( (inet_pton(AF_INET, ALL_IDAWY, &servaddr.sin_addr)) <= 0) {
		GDGL_DEBUG("inet_pton error\n");
		sleep(1);
	}
	while ( connect(fd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0 ) {
		GDGL_DEBUG("connect error, push addr:%s,port:%d\n",ALL_IDAWY,5018);
		sleep(1);
	}
	close(fd);
	GDGL_DEBUG("wati_network_init_ok\n");
}
