/***************************************************************************
 Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
 FileName:      net.cpp
 Author:        jiang
 Version :      1.0
 Date:          2014-02-27
 Description:
 History:
 <author>  <time>   <version >   <desc>
 ***************************************************************************/
#include <sys/ioctl.h>
#include <string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<linux/if_ether.h>
#include<linux/if.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<fcntl.h>
#include<limits.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>

#include <ifaddrs.h>  //add yan150112
#include"json.h"
#include"cJSON.h"
#include"net.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"
#include "appSqlite.h"

//���ͳ������ݵ���Ϣ����
//extern int MsgtermRxId; //���ܳ������ݵ���Ϣ����
extern int MsgserverTxId;//���ͷ��������ݵ���Ϣ����


//extern uint8 msgfromflg;
//extern uint8 term_send_flag;
//extern uint8 term_rcv_flag;
//extern uint8 client_send_count;
//extern volatile uint8 server_heart_flag;
extern volatile uint8 node_heart_flag;
extern volatile uint8 client_flag; //add yanly150512
extern pthread_mutex_t mutex;
/****************************************************************************/
uint8 Client_Sn = 0;
int Client_Socket = 0;

uint8 server_heart_sn = 0;

//node_list node_table[NODE_NUM]={0};


char local_addr[1025];//add yan150112 delete yanly150513
/**********************************************************************************************/
pthread_t thread_do[2]; //pthread id
//int handle_connect(int sockfd);
//int handle_request();
//static function
void *handle_connect(void *argv);
void *handle_request(void *argv);
void sig_process(int signo);
void sig_pipe(int signo);
static int connect_to_gateway_init(void);//add yanly141229
//int send_server_heartbeat(int fd);
static void close_client(int i);
static void set_heart_beat(int i, char heart_status);
int send_msg_to_server(char *text, int text_size, int fd);
void send_all_security_config_to_server(int fd);
void parse_receive_servermsg(msgform msg_Rque, int rcv_num, int scfd);

//uint8 node_ip[20]="192.168.0.102";
//client_status client_list[MAX_CLIENT_NUM];
volatile int server_connected_fd = 0; //add yanly150513 服务器已连接上的标记
int connect_host[MAX_CLIENT_NUM];//add yanly141229
char islocalip_flag[MAX_CLIENT_NUM]={0};
//char clientaddr[MAX_CLIENT_NUM][20];
#ifdef PUSH_TO_SERVER_NEW_LOGIC
int clientfd_local_2_cloudproxy = -1; //add yanly150511
#endif
char connect_host_online[MAX_CLIENT_NUM] =
{ HEARTBEAT_NOT_OK }; //add yan150115增加检测心跳包,规定时间内没收到心跳包断开连接
uint16 client_num = 0; //how many client socket
//int connect_number = 0;
// add by yang 2015319
uint8 ConfigReportMsg_flag = 0;
int ConfigReportMsg_count = 0;

uint8 TermDataReportMsg_flag = 0;
int TermDataReportMsg_count = 0;
int ConnectClient()
{
	int s_s;
	int optval = 1;
	struct sockaddr_in local;
	int i;
	for (i = 0; i < MAX_CLIENT_NUM; i++)
	{
		connect_host[i] = -1;
		islocalip_flag[i] = 0;
	}
	//SETUP tcp socket
	s_s = socket(AF_INET, SOCK_STREAM, 0);
	if (setsockopt(s_s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)//允许重复使用本地地址和端口?
	{
		perror("setsockopt");
		exit(1);
	}
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(argv[1]);//htonl(INADDR_ANY); //local addr anyway
	local.sin_port = htons(CLIENT_PORT);

	//bind
	bind(s_s, (struct sockaddr *) &local, sizeof(local));
	listen(s_s, 5);
	return s_s;
}
/*
 *get ipaddr by funtionc: getifaddrs and "eth0"
 * */
void get_local_ipaddr()
{
	struct ifaddrs *ifaddr, *ifa;
	int s;
	char ret = 0;
	if (getifaddrs(&ifaddr) == -1)
	{
		perror("getifaddrs");
		return;
	}
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
			continue;
		if (strcmp(ifa->ifa_name, "eth0") != 0)
			continue;
		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), local_addr,
				NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0)
		{
			printf("getnameinfo() failed: %s\n", gai_strerror(s));
		} else
		{
			printf("address:%s\n", local_addr);
			ret = 1;
			break;
		}
	}
	if (ret != 1)
	{
		printf("get local addr error\n");
		exit(1);
	}
	freeifaddrs(ifaddr);
}

void *node_msg_thread(void *p)
{

	while (1)
	{
		nodeSocket();
	}
	return NULL;
}
int connect_to_server_init(void);

#ifdef PUSH_TO_SERVER_NEW_LOGIC
/*
 * 服务器上传逻辑修改，服务器相当于连接网关的一个本地客户端
 * */
void *server_msg_thread(void *p)
{
	char send_buffer[MAX_MSG_BUF]=
	{	0};
	int send_len;
	//    int recbytes=0;
	//	int ret=0;
	int r_msg_num=0;
	msgform msg_Rxque;

	//    do
	//   	  {cfd = connect_to_server_init();}
	//   	while(!cfd);
	while(1)
	{
		r_msg_num = msgrcv(MsgserverTxId,&msg_Rxque,(sizeof(msgform)-4),0,0);//返回队列中最老的消息,不阻塞，没有消息时返回ENOMSG
		if((r_msg_num ==-1)&&(errno !=ENOMSG))
		{
			printf("errno:%d\n",errno);
			perror("msgrcv:");
		}
		else if(r_msg_num >0)
		{
			printf("be will sending msg to server, type:%d,size:%d\n",msg_Rxque.mtype,r_msg_num);
			send_len = add_time_field_in_upload_server_msg(send_buffer, msg_Rxque.mtext);
			if(send_len >0)
			send_msg_to_server(send_buffer, send_len, clientfd_local_2_cloudproxy);
		}
		// if(cfd!=-10)
		// close(cfd);
	}
	return NULL;
}
#else
/*
 * yanly150512: 发现如果运行过程中服务器断开，没有重连机制
 * */
int server_msg_thread_loop()
{
	int cfd;
	char buffer[MAXBUF] =
	{ 0 };
	int recbytes = 0;
	int count_s = 0, count_r = 0;
	int ret = 0;
	int r_msg_num = 0;
	msgform msg_Rxque;
	int send_res;

	do
	{
		cfd = connect_to_server_init();
	} while (!cfd);
	while (1)
	{
		r_msg_num = msgrcv(MsgserverTxId, &msg_Rxque, (sizeof(msgform) - 4), HeartReportMsg, IPC_NOWAIT);//有心跳消息时先获取心跳
		if((r_msg_num == -1)&& (errno == ENOMSG))
		{
			r_msg_num = msgrcv(MsgserverTxId, &msg_Rxque, (sizeof(msgform) - 4), 0, IPC_NOWAIT);//返回队列中最老的消息,不阻塞，没有消息时返回ENOMSG
		}
		if ((r_msg_num == -1) && (errno != ENOMSG))
		{
			GDGL_DEBUG("errno:%d\n", errno);
			perror("msgrcv:");
			exit(1);
		}
		else if (r_msg_num > 0)
		{
			GDGL_DEBUG("sending msg to server, type:%d,size:%d\n",
					msg_Rxque.mtype, r_msg_num);
			//printf("server msg que:%s\n",msg_Rxque.mtext);
			count_s = 0;
			while (count_s++ < 3)
			{
				send_res = send_msg_to_server(msg_Rxque.mtext, r_msg_num, cfd); //old
				if(send_res <0)
					return -1;
				printf("send_msg_to_server: ok\n");
				count_r = 0;
				while (count_r++ < 3)
				{
					//printf("test server send 22222\n");
					recbytes = recv(cfd, buffer, MAXBUF, MSG_DONTWAIT);
					printf("server recv flag is: %d\n",recbytes);
					if ((recbytes == -1) && (errno != EAGAIN))
					{
						perror("socket read server data fail");
						close(cfd);
						server_connected_fd =0;
						return -1;
					} else if (recbytes > 0)
					{
//						printf("receive server 5031 size=%d \n", recbytes);
						if ((ret = parse_received_server_msg(buffer)) < 0)
						{
							GDGL_DEBUG("server msg format error:%d\n", ret);
						}
						memset(buffer, 0, MAXBUF);
						break;
					} else if (recbytes == 0)
					{
						GDGL_DEBUG(
								"connected server 5031,but the other side close this socket\n");
						close(cfd);
						server_connected_fd =0;
						return -2;
					}
					printf("but not recved\n");
					sleep(1);
				}
				if (count_r < 3) {
					printf("server recved\n");
					break;
				}
			}
		}
//		//NO MSG
//		/*
//		 * 处理心跳逻辑
//		 * */
//		send_res = send_server_heartbeat(cfd);
//		if(send_res <0)
//			return -1;
//		/*
//		 * */
	}
	return 0;
}
void *server_msg_thread(void *p)
{
	while (1)
	{
		server_msg_thread_loop();
	}
	return NULL;
}
//void *server_msg_thread(void *p)
//{
//	int cfd;
//	char buffer[MAXBUF]={0};
//    int recbytes=0;
//    int count_s=0,count_r=0;
//	int ret=0;
//    int r_msg_num=0;
//    msgform msg_Rxque;
//    //add by yanly150512
//	char send_buffer[MAX_MSG_BUF]={0};
//	int send_len;
//	//
//    do
//   	  {cfd = connect_to_server_init();}
//   	while(!cfd);
//    while(1)
//    {
//
//	           r_msg_num = msgrcv(MsgserverTxId,&msg_Rxque,(sizeof(msgform)-4),0,0);//返回队列中最老的消息,不阻塞，没有消息时返回ENOMSG
//
//	           if((r_msg_num ==-1)&&(errno !=ENOMSG))
//	  	  		{
//	  	  			printf("errno:%d\n",errno);
//	  	  			perror("msgrcv:");
//
//	  	  		}
//	  	  		else if(r_msg_num >0)
//	  	  		{
//
//
//	  	  			printf("be will sending msg to server, type:%d,size:%d\n", msg_Rxque.mtype,r_msg_num);
//	  				send_len = add_time_field_in_upload_server_msg(send_buffer, msg_Rxque.mtext);//add by yanly150512
//	  				if(send_len <=0)
//	  					continue;
//	  	             //printf("server msg que:%s\n",msg_Rxque.mtext);
//
//	  	  			count_s = 0;
//	  	  			while (count_s++< 3)
//	  	  			{
//	  	  				//send_msg_to_server(msg_Rxque.mtext, r_msg_num, cfd); //old
//		  				send_msg_to_server(send_buffer, send_len, cfd);
//
//	  	  				count_r = 0;
//	  	  				while (count_r++ < 3)
//	  	  				{
//
//	  	  				  //printf("test server send 22222\n");
//	  	  					recbytes = recv(cfd, buffer, MAXBUF,MSG_DONTWAIT);
//                           // printf("test recv 555555 %d\n",recbytes);
//	  	  					if((recbytes==-1)&&(errno!=EAGAIN))
//	  	  					{
//	  	  						printf("recbytes:%d\n",recbytes);
//	  	  						perror("socket read server data fail");
//	  	  						//close(cfd);
//	  	  						//return -1;
//	  	  					}
//	  	  					else if(recbytes>0)
//	  	  					{
//	  	  						printf("receive server 5031 size=:%d \n",recbytes);
//	  	  						if((ret=parse_received_server_msg(buffer))<0)
//	  	  						{
//	  	  							printf("server msg format error:%d\n",ret);
//	  	  						}
//	  	  						memset(buffer,0,MAXBUF);
//	  	  						break;
//	  	  					}
//	  	  					else if(recbytes==0)
//	  	  					{
//	  	  						printf("connected server 5031,but the other side close this socket\n");
//	  	  						//close(cfd);
//	  	  						//return -2;
//	  	  					}
//	  	  					sleep(1);
//	  	  				}
//	  	  				if (count_r < 3)
//	  	  					break;
//
//	  	  			}
//
//	  	  			//printf("server11111 \n");
//            }
//	        // if(cfd!=-10)
//	        // close(cfd);
//   }
//  return NULL;
//}
#endif

/*
 * connect gateway 5018 init
 * */
static int connect_to_gateway_init(void)
{
	int cfd = 0;
	struct sockaddr_in s_add;
	int rc = -1;
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd == -1)
	{
		GDGL_DEBUG("node socket fail ! \r\n");
		return -1;
	}
	bzero(&s_add, sizeof(struct sockaddr_in));
	s_add.sin_family = AF_INET;
	s_add.sin_addr.s_addr = inet_addr("127.0.0.1");//modify by yan
	s_add.sin_port = htons(NODE_PORT);
	//	do
	//	{
	//		printf("gateway 5018 not connected!\n");
	//		sleep(RECONNECT_GATAWAY5018_TIME);
	rc = connect(cfd, (struct sockaddr *) (&s_add), sizeof(struct sockaddr));
	//	}
	//	while(rc ==-1);
	if (rc != -1)
		GDGL_DEBUG("connected gateway 5018 socket success,socket:%d\n", cfd);
	return cfd;
}
/*
 * connect server 5040 init
 * */
int connect_to_server_init(void)
{
	int cfd = 0;
	struct sockaddr_in s_add;
	int rc = -1;

	server_connected_fd = 0;
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd == -1)
	{
		GDGL_DEBUG("connect to server socket setup fail ! \r\n");
		return -1;
	} else
	{
//		printf("connect to server socket setup sucess:cfd= %d! \r\n", cfd);
	}
	bzero(&s_add, sizeof(struct sockaddr_in));
	s_add.sin_family = AF_INET;
	s_add.sin_addr.s_addr = inet_addr(SERVER_IPADDR_DEBUG);//(local_addr);//(SERVER_IPADDR_DEBUG);//debug by yang
	s_add.sin_port = htons(SERVER_PORT);

	do
	{
		//sleep(RECONNECT_SERVER_TIME);
		rc
				= connect(cfd, (struct sockaddr *) (&s_add),
						sizeof(struct sockaddr));
		//		printf("SERVER_PORT=%d\n",SERVER_PORT);
		//		printf("s_add.sin_port=%d\n",s_add.sin_port);
		//		printf("s_add.sin_addr.s_addr=%d\n",s_add.sin_addr.s_addr);
		//		printf("rc=%d\n",rc);
		sleep(RECONNECT_SERVER_TIME);
		if (rc == -1)
		{
			GDGL_DEBUG("server not connected!\n");
		}
	} while (rc == -1);

	GDGL_DEBUG("connected server socket success,socket:%d\n", cfd);
	server_connected_fd = cfd;
	sleep(RECONNECT_SERVER_TIME);
	//网关首次连接到服务器，发送配置给服务器
	send_all_security_config_to_server(cfd);
	send_all_energy_config_to_server(cfd);
	return cfd;
}
/*
 * 处理5018callback的单元
 * */
int nodeSocket() //modify141230
{
	uint8 msg[20] = "0200070007";
	int cfd;
	char buffer[MAXBUF] =
	{ 0 };
	int recbytes;
	int s_status;
	int detach_obj;

	cfd = connect_to_gateway_init();
	memset(buffer, 0, MAXBUF);
	while (cfd)// connected
	{
		if (node_heart_flag == enable)
		{
			node_heart_flag = disable;
			//printf("send node heart beat\n");
			s_status = send(cfd, msg, sizeof(msg), MSG_DONTWAIT);
			if (s_status == -1)
			{
				perror("connected gateway 5018,but send error\n");
				close(cfd);
				return -1;
			} else if (s_status == 0)
			{
				GDGL_DEBUG(
						"connected gateway 5018,but the other side close this socket\n");
				close(cfd);
				return -2;
			} else
			{
			}//normal send
		}
		recbytes = recv(cfd, buffer, MAXBUF, MSG_DONTWAIT); //���ӳɹ�,�ӷ���˽����ַ�
		if ((recbytes == -1) && (errno != EAGAIN))
		{
			//printf("recbytes:%d\n",recbytes);
			perror("node read data fail");
			close(cfd);
			return -1;
		} else if (recbytes > 0)
		{
			//printf("receive 5018 [size=%d][buff=%s]\n",recbytes,buffer);
			//GDGL_DEBUG("receive 5018 [size=%d]\n", recbytes);
			detach_obj = detach_5002_message22(buffer, recbytes);
			//printf("5002message22:%d\n",detach_obj);
			switch (detach_obj)
			{
				case DETACH_BELONG_ENERGY: /*这个case处理节能的信息*/
					//printf("parse_json_node 1111111\n");
					parse_json_node(buffer, recbytes);
					break;
				case DETACH_BELONG_SECURITY: /*这个case处理安防的信息*/
					parse_json_node_security(buffer, recbytes);
					break;
				case DETACH_BELONG_IN_COMMON:
					origin_callback_handle(buffer, recbytes); //设备节点重新上电产生的callback处理
					break;
				default:
					break;
			}
			memset(buffer, 0, MAXBUF);
		} else if (recbytes == 0)
		{
			GDGL_DEBUG(
					"connected gateway 5018,but the other side close this socket\n");
			close(cfd);
			return -2;
		}
		usleep(1000);
	}
	//close(cfd);
	return 0;
}
/***************************************************************************************/
/*
 * 处理服务器业务的单元
 * 采用轮询方式：执行发送心跳包>>发送消息队列数据>>接收服务器发过来的消息,不阻塞
 * */

void parse_receive_servermsg(msgform msg_Rque, int rcv_num, int scfd)
{
	//msgform msg_que;
	//memcpy(msg_Rque,msg_Rxque,rcv_num);
	switch (msg_Rque.mtype)
	{
		case ConfigReportMsg:

			if (ConfigReportMsg_count < 3)
			{
				if (ConfigReportMsg_flag == 1)
				{
					ConfigReportMsg_flag = 0;
					ConfigReportMsg_count = 0;
				} else
				{
					send_msg_to_server(msg_Rque.mtext, rcv_num, scfd);
					ConfigReportMsg_count++;
					json_msgsnd(MsgserverTxId, ConfigReportMsg, msg_Rque.mtext,
							rcv_num);
				}
			} else
				ConfigReportMsg_count = 0;
			break;

		case TermDataReportMsg:

			if (TermDataReportMsg_count < 3)
			{
				if (TermDataReportMsg_flag == 1)
				{
					TermDataReportMsg_flag = 0;
					TermDataReportMsg_count = 0;
				} else
				{
					json_msgsnd(MsgserverTxId, TermDataReportMsg,
							msg_Rque.mtext, rcv_num);
					send_msg_to_server(msg_Rque.mtext, rcv_num, scfd);
					TermDataReportMsg_count++;
				}
			} else
				TermDataReportMsg_count = 0;
			break;
		default:
			break;
	}
}
#if 0
int ServerSocket() //modify141230

{
	int cfd;
	char buffer[MAXBUF];
	int recbytes;
	int ret;
	int r_msg_num;
	msgform msg_Rxque;
	//printf("server11111 \n");
	cfd = connect_to_server_init();
	memset(buffer,0,MAXBUF);
	while(cfd)// connected

	{
		//心跳
		if(send_server_heartbeat(cfd) <0)
		return -1;

		//接收server msg
		recbytes = recv(cfd, buffer, MAXBUF,MSG_DONTWAIT);
		if((recbytes==-1)&&(errno!=EAGAIN))
		{
			//printf("recbytes:%d\n",recbytes);
			perror("socket read server data fail");
			//close(cfd);
			return -1;
		}
		else if(recbytes>0)
		{
			printf("receive server 5040 size:%d \n",recbytes);
			if((ret=parse_received_server_msg(buffer))<0)
			{
				printf("server msg format error:%d\n",ret);
			}
			memset(buffer,0,MAXBUF);
		}
		else if(recbytes==0)
		{
			printf("connected server 5040,but the other side close this socket\n");
			//close(cfd);
			return -2;
		}
		//发送接收的消息队列
		r_msg_num = msgrcv(MsgserverTxId,&msg_Rxque,sizeof(msgform),0,IPC_NOWAIT);//返回队列中最老的消息,不阻塞，没有消息时返回ENOMSG
		if((r_msg_num ==-1)&&(errno !=ENOMSG))
		{
			printf("errno:%d\n",errno);
			perror("msgrcv:");
		}
		else if(r_msg_num >0)
		{
			//send
			printf("be will sending msg to server, type:%ld,size:%d\n",msg_Rxque.mtype,r_msg_num);
			//			printf("server msg que:%s\n",msg_Rxque.mtext);
			send_msg_to_server(msg_Rxque.mtext, r_msg_num, cfd);
			// parse_receive_servermsg(msg_Rxque,r_msg_num,cfd);

		}

	}
	//close(cfd);
	return 0;
}

#endif
/*
 * send server heartbeat
 * */
#if 0
int send_server_heartbeat(int fd)
{
	cJSON* root;
	char *out;
	int ret = 1;
	if(server_heart_flag == enable)
	{
		printf("send server heartbeat\n");
		server_heart_flag = disable;
		root=cJSON_CreateObject();
		cJSON_AddNumberToObject(root,"MsgType", 64);
		cJSON_AddNumberToObject(root,"Sn", server_heart_sn);
		if(server_heart_sn<255)
			server_heart_sn++;
		else
			server_heart_sn=0;
		out = cJSON_PrintUnformatted(root);
		ret = send(fd,out,strlen(out),MSG_DONTWAIT);
		if(ret ==-1)
		{
			perror("connected server 5040,but send error\n");
			close(fd);
			return -1;
		}
		else if(ret ==0)
		{
			printf("connected server 5040,but the other side close this socket\n");
			close(fd);
			return -2;
		}
		else
		{
			printf("send server heartbeat\n");
		}//normal send
		cJSON_Delete(root);
		free(out);
	}
	return ret;
}
#else
void send_server_heartbeat()
{
	cJSON* root;
	char *out;
//	int ret = 1;

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root,"MsgType", HeartReportMsg);
	cJSON_AddNumberToObject(root,"Sn", server_heart_sn);
	if (server_heart_sn < 255)
		server_heart_sn++;
	else
		server_heart_sn = 0;
	out = cJSON_PrintUnformatted(root);
	json_msgsnd(MsgserverTxId, HeartReportMsg, out, strlen(out)); //转发给服务器
	//ret = send(fd,out,strlen(out),MSG_DONTWAIT);
	cJSON_Delete(root);
	free(out);
	//return ret;
}
#endif
/***************************************************************************************/

/*
 * 修改客户端socket的heartbeat值
 * */
static void set_heart_beat(int i, char heart_status)
{
	pthread_mutex_lock(&mutex);
	connect_host_online[i] = heart_status;
	pthread_mutex_unlock(&mutex);
}
/*
 * 收到heartbeat
 * */
void set_heart_beat_client(int client_fd)
{
	int i;
	for (i = 0; i < MAX_CLIENT_NUM; i++)
	{
		if (connect_host[i] == client_fd)
		{
			set_heart_beat(i, HEARTBEAT_OK);
		}
	}
}
/*
 * 检测心跳单元
 * */
void *check_client_heartbeat(void *p)
{
	int i;
	while (1)
	{
		sleep(1);//不加延时，client_flag 可能会被编译器优化 //add yanly150512
		if (client_flag == 1)
		{
			//			printf("check_client_heartbeat\n");
			client_flag = 0;
			for (i = 0; i < MAX_CLIENT_NUM; i++)
			{
				if (client_num <= 0)
					break;
				if (connect_host[i] != -1)
				{
					if (connect_host_online[i] == HEARTBEAT_NOT_OK)
					{
						printf(
								"Didn't receive the heartbeat packets within the regulation time:clolse socket%d\n",
								connect_host[i]);
						close_client(i);
					} else
					{
						set_heart_beat(i, HEARTBEAT_NOT_OK);
					}
				}
			}
		}
	}
	return 0;
}
/*
 * 客户端连接断开，进行关闭操作
 * */
static void close_client(int i)
{
	close(connect_host[i]);
	pthread_mutex_lock(&mutex);
	connect_host[i] = -1;
	islocalip_flag[i] = 0;//add 0624
	client_num--;
	pthread_mutex_unlock(&mutex);
}
/*
 * 建立客户端连接单元
 * */
#ifdef PUSH_TO_SERVER_NEW_LOGIC
void *handle_connect(void *argv)
{
	int s_s = *((int *)argv);
	struct sockaddr_in from;
	socklen_t len = sizeof(from);
	for(;;)
	{
		int i = 0;
		int s_c = accept(s_s, (struct sockaddr*)&from, &len);
		printf("a client connect,from: %s,sock:%d \n", inet_ntoa(from.sin_addr),s_c);
		if(clientfd_local_2_cloudproxy <0)
		{
			if(memcmp(inet_ntoa(from.sin_addr), "127.0.0.1", 9) ==0)
			{
				printf("local client connected!\n");
				clientfd_local_2_cloudproxy = s_c;
			}
		}
		//put the client socket to connect_host
		for(i=0; i<MAX_CLIENT_NUM; i++)
		{
			if(connect_host[i] == -1)
			{
				connect_host[i] = s_c;
				client_num ++;
				set_heart_beat(i, HEARTBEAT_OK);
				break;
			}
		}
	}
	return NULL;
}
#else
void *handle_connect(void *argv)
{
	int s_s = *((int *) argv);
	struct sockaddr_in from;
	socklen_t len = sizeof(from);
	for (;;)
	{
		int i = 0;
		int s_c = accept(s_s, (struct sockaddr*) &from, &len);
		GDGL_DEBUG("a client connect,from: %s,sock:%d \n",
				inet_ntoa(from.sin_addr), s_c);
		//put the client socket to connect_host
		for (i = 0; i < MAX_CLIENT_NUM; i++)
		{
			if (connect_host[i] == -1)
			{
				connect_host[i] = s_c;
				if( memcmp(inet_ntoa(from.sin_addr), "127.0.0.1", 9) ==0 ) {
					islocalip_flag[i] =1;
					printf("socket[%d] is localip\n",s_c);
				}
				client_num++;
				if(client_num >MAX_CLIENT_NUM) {
					printf("client exceed max!\n");
					exit(1);
				}
				printf("client num=%d\n",client_num);
				set_heart_beat(i, HEARTBEAT_OK);
				break;
			}
		}
		if(client_num >MAX_CLIENT_NUM) {
			GDGL_DEBUG("client connect exceed max!\n");
			exit(1);
		}
	}
	return NULL;
}
#endif
void *handle_request(void *argv)
{
	char buff[MAX_MSG_BUF];
	char respondBUff[MAXBUF];
	int n = 0;
	int maxfd = -1;
	fd_set scanfd;
	struct timeval timeout_tv={1, 0};
//	timeout_tv.tv_sec = 3;
//	timeout_tv.tv_usec = 0;
	int i = 0;
	int err = -1;
	int __buffType;

	for (;;)
	{
		timeout_tv.tv_sec = 1;  //add yanly150521
		timeout_tv.tv_usec = 0;
		maxfd = -1;
		FD_ZERO(&scanfd);
		for (i = 0; i < MAX_CLIENT_NUM; i++) //resolve scanfd and maxfd
		{
			if (connect_host[i] != -1)
			{
				FD_SET(connect_host[i], &scanfd);
				if (maxfd < connect_host[i])
				{
					maxfd = connect_host[i];
				}
			}
		}
		//select wait socket
		err = select(maxfd + 1, &scanfd, NULL, NULL, &timeout_tv);
		switch (err)
		{
			case 0:
				break; //timeout之后会被清零
			case -1:
				GDGL_DEBUG("select error\n");
				exit(1);
				break; //error happen
			default:
				if (client_num <= 0)
					break;
				for (i = 0; i < MAX_CLIENT_NUM; i++)
				{
					if (connect_host[i] != -1)
					{
						if (FD_ISSET(connect_host[i], &scanfd))
						{
							memset(buff, 0, MAX_MSG_BUF);
							n = recv(connect_host[i], buff, MAX_MSG_BUF, 0);
							if (n > 0)
							{
								printf("recv client msg----------------------------------\n");
								printf("recv client size[%d],fd[%d],msg: %s\n", n, connect_host[i], buff);
								//printf("buff[0]=%d,buff[n-1]=%d \n",buff[0],buff[n-1]);
								//printf("sever recieve data:%s \n", buff);
								//									memcpy(client_list[i].client_buff,buff,n);
								// printf("**client_buff=%s ****\r\n",buff);
								//									client_list[i].client_buff_len=n;
								//printf("size:%d\n",n);
								__buffType = detach_interface_msg_client(buff,
										n);//detach interface
								switch (__buffType)
								{
									case DETACH_PRASE_ERROR:
										memcpy(respondBUff,
												"data format error:", 18);
										memcpy(respondBUff + 18, buff, n);
										send(connect_host[i], respondBUff, n
												+ 18, 0);
										GDGL_DEBUG(
												"received client message,but prase json error\n");
										break;
									case DETACH_MSGTYPE_ERROR:
										client_msg_handle_in_msgtype_error(
												buff, n, connect_host[i]);
										break;
										/*这个case处理节能的客户端信息*/
									case DETACH_BELONG_ENERGY:
										//parse_json_client(buff, n, connect_host[i]);
										client_msg_handle_energy(buff, n,
												connect_host[i]);
										break;
										/*这个case处理安防的客户端信息*/
									case DETACH_BELONG_SECURITY:
										//printf("received client message >>for security >>");
										client_msg_handle_security(buff, n,
												connect_host[i]);
										break;
									case DETACH_BELONG_IN_COMMON:
										client_msg_handle_in_common(buff, n,
												connect_host[i]);
										break;
									default:
										break;
								}
								//send(connect_host[i],buff,strlen(buff),0);
								memset(buff, 0, MAX_MSG_BUF);
							} else if (n == 0)
							{
								GDGL_DEBUG("client socket%d disconneted!\n",
										connect_host[i]);
								close_client(i);
//								connect_host[i] = -1;
//								client_num--;
//								close(connect_host[i]);
							} else if (n == -1)
							{
								GDGL_DEBUG("client socket%d disconneted!\n",
										connect_host[i]);
								close_client(i);
//								connect_host[i] = -1;
//								client_num--;
//								close(connect_host[i]);
							}
						}
					}
				}
				break;
		}
	}
	//free
	for (i = 0; i < MAX_CLIENT_NUM; i++) //resolve scanfd and maxfd
	{
		if (connect_host[i] != -1)
		{
			close(connect_host[i]);
		}
	}
	return NULL;
}

void sig_process(int signo)
{
	int i;
	for (i = 0; i < MAX_CLIENT_NUM; i++) //resolve scanfd and maxfd
	{
		if (connect_host[i] != -1)
		{
			close(connect_host[i]);
		}
	}

	_exit(0);
}
void sig_pipe(int signo)
{
	_exit(0);
}
//发送给所有在线客户端   //add yanly150616
void send_msg_to_all_client(char *text, int text_size)
{
	int i;
	int buf_len, sendlen;
	char buf[65535]={0};
	for (i = 0; i < MAX_CLIENT_NUM; i++)
	{
		if (client_num < 0)
		{
			GDGL_DEBUG("send msg to all client error: no client is connected!\n");
			break;
		}
		if (connect_host[i] != -1)
		{
			printf("fd[%d]\n",connect_host[i]);
			if(islocalip_flag[i] ==1) {
				printf("send client msg to channel\n");
				printf("send[%d][%s]\n",text_size, text);
				sendlen = send(connect_host[i], text, text_size, 0);
			}
			else {
				buf_len = snprintf(buf, sizeof(buf)+1, "#%d#data%s", text_size, text);
				printf("send[%d][%s]\n",buf_len, buf);
				sendlen = send(connect_host[i], buf, buf_len, 0);
			}
			printf("real send all client size[%d]\n",sendlen);
			if (sendlen <= 0)
			{
				GDGL_DEBUG("sock%d send error ,may be disconneted\n", connect_host[i]);
				close_client(i);
			}
		}
	}
}
//发送给所有在线客户端   //add yanly141230
//void send_msg_to_all_client(char *text, int text_size)
//{
//	int i;
//	int buf_len, sendlen;
//	char buf[65535]={0};
//	buf_len = snprintf(buf, sizeof(buf)+1, "#%d#data%s", text_size, text);
//	GDGL_DEBUG("send all client[%d][%s]\n",buf_len, buf);
//	for (i = 0; i < MAX_CLIENT_NUM; i++)
//	{
//		if (client_num < 0)
//		{
//			GDGL_DEBUG("send msg to all client error: no client is connected!\n");
//			break;
//		}
//		if (connect_host[i] != -1)
//		{
////			sendlen = send(connect_host[i], text, text_size, 0);
//			sendlen = send(connect_host[i], buf, buf_len, 0);
//			GDGL_DEBUG("real send all client size[%d]\n",sendlen);
//			if (sendlen <= 0)
//			{
//				//				connect_host[i] =-1;
//				//				client_num--;
//				//				close(connect_host[i]);
//				close_client(i);
//			}
//			//printf("send msg to all client,this is socket%d>>",connect_host[i]);
//		}
//	}
////	GDGL_DEBUG("send msg to all client\n");
//}
//发送给one客户端
void send_msg_to_client(char *text, int text_size, int fd)
{
	int i;
	int buf_len, sendlen;
	char buf[65535]={0};
	int socket_num =-1;
	for (i = 0; i < MAX_CLIENT_NUM; i++)
	{
		if (connect_host[i] == fd)
		{
			socket_num = i;
			printf("socket_num=%d\n",socket_num);
			break;
		}
	}
//	//获取ip地址
//	struct sockaddr_in peerAddr;
//	int peerLen;
//	char ipAddr[INET_ADDRSTRLEN];//保存点分十进制的地址
//	getpeername(fd, (struct sockaddr *)&peerAddr, &peerLen);
//	if( memcmp(inet_ntop(AF_INET, &peerAddr.sin_addr, ipAddr, sizeof(ipAddr)), "127.0.0.1", 9) ==0 ) {
	if(socket_num >=0)
	{
		if(islocalip_flag[socket_num] ==1) {
			printf("send client msg to channel\n");
			printf("send[%d][%s]\n",text_size, text);
			sendlen = send(fd, text, text_size, 0);
		}
		else {
			buf_len = snprintf(buf, sizeof(buf)+1, "#%d#data%s", text_size, text);
			printf("send[%d][%s]\n",buf_len, buf);
			sendlen = send(fd, buf, buf_len, 0);
		}
		printf("real sendsize[%d]\n",sendlen);
		if (sendlen <= 0)
		{
//			if (connect_host[socket_num] == fd)
//			{
				close_client(socket_num);
//			}
			GDGL_DEBUG("sock%d send error ,may be disconneted\n", fd);
		}
	}
	else
	{
		printf("not find socket num\n");
		exit(1);
	}
}
//发送给server
ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft, nwritten;
	const char	*ptr;

	ptr = vptr;	/* can't do pointer arithmetic on void* */
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
			return(nwritten);		/* error */

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

#if 1
int send_msg_to_server(char *text, int text_size, int fd)
{
	if ( writen(fd, text, text_size) != text_size ) {
		GDGL_DEBUG("server sock%d send error ,may be disconneted\n", fd);
		close(fd);
		return -1;
	}
	return 0;
}
#else
int send_msg_to_server(char *text, int text_size, int fd)
{
	if (send(fd, text, text_size, 0) <= 0)
	{
		close(fd);
#ifdef PUSH_TO_SERVER_NEW_LOGIC
		clientfd_local_2_cloudproxy = -1;
#endif
		printf("server sock%d send error ,may be disconneted\n", fd);
		return -1;
	}
	return 0;
}
#endif
/*
 * 网关连接到服务器后，发送安防表配置信息给服务器
 * */
void send_all_security_config_to_server(int fd)
{
	char **q_data;
	char **ieee;
	int ieee_row, ieee_col;
	int q_row, q_col;
	int i, j;
	int index_ieee, index_q;
	char
			*sql_req_config =
					"select ieee,nwkaddr,type,subtype,num,info,operator from stable where nwkaddr not null";
	char *sql_req_ieee =
			"SELECT  DISTINCT ieee,nwkaddr FROM stable where nwkaddr not null";

	cJSON *sroot;
	cJSON *NodeList_array, *NodeList_item;
	cJSON *SubNodeItem, *SubNodeArray;
//	int opt ;
	char *out = NULL;
	GDGL_DEBUG("send all security config to server\n");
	//query database:
	q_data = sqlite_query_msg(&q_row, &q_col, sql_req_config);
	if (q_data == NULL)
	{
//		printf("query db error\n");
		sqlite_free_query_result(q_data);
		return;
	}
	ieee = sqlite_query_msg(&ieee_row, &ieee_col, sql_req_ieee);
	if (ieee == NULL)
	{
//		printf("query db error\n");
		sqlite_free_query_result(ieee);
		return;
	}
	//    opt = sqlite_query_global_operator();  //delete query global operator by yan150511
	//    if(opt ==-1)
	//    {
	//    	printf("query db error\n");
	//    	return;
	//    }
	//organize json package
	sroot = cJSON_CreateObject();
	cJSON_AddNumberToObject(sroot, "MsgType", SERVER_UPLOAD_ALL_SECURITY_CONFIG_MSG);
	cJSON_AddNumberToObject(sroot, "Sn", 10);
	//	cJSON_AddNumberToObject(sroot, "GlobalOpt", opt); //delete query global operator by yan150511
	cJSON_AddItemToObject(sroot, "NodeList", NodeList_item
			= cJSON_CreateArray());
	for (i = 0; i < ieee_row; i++)
	{
		index_ieee = i * ieee_col + ieee_col;//ieee data的每行头
		cJSON_AddItemToArray(NodeList_item, NodeList_array
				= cJSON_CreateObject());
		cJSON_AddStringToObject(NodeList_array, "SecurityNodeID", ieee[index_ieee]);
		cJSON_AddStringToObject(NodeList_array, "Nwkaddr", ieee[index_ieee+1]);
		cJSON_AddItemToObject(NodeList_array, "SubNode", SubNodeItem
				= cJSON_CreateArray());
		for (j = 0; j < q_row; j++)
		{
			index_q = j * q_col + q_col;//data的每行头
			if (strcmp(q_data[index_q], ieee[index_ieee]) == 0)
			{
				cJSON_AddItemToArray(SubNodeItem, SubNodeArray=cJSON_CreateObject());
				cJSON_AddNumberToObject(SubNodeArray, "Type", atoi(q_data[index_q+2]));
				cJSON_AddNumberToObject(SubNodeArray, "SubType", atoi(q_data[index_q+3]));
				cJSON_AddNumberToObject(SubNodeArray, "Num", atoi(q_data[index_q+4]));
				cJSON_AddStringToObject(SubNodeArray, "Info", q_data[index_q+5]);
				cJSON_AddNumberToObject(SubNodeArray, "OperatorType", atoi(q_data[index_q+6]));
			} else
			{
			}
		}
	}
	out = cJSON_PrintUnformatted(sroot);
	printf("size:%d\n", strlen(out));
	send_msg_to_server(out, strlen(out), fd);
	//	json_msgsnd(MsgserverTxId, SERVER_UPLOAD_ALL_SECURITY_CONFIG_MSG, out, strlen(out));
	//free:
	cJSON_Delete(sroot);
	free(out);
	sqlite_free_query_result(q_data);
	sqlite_free_query_result(ieee);
	return;
}

void send_all_energy_config_to_server(int fd)
{
	char **q_data;
	char **ieee;
	int ieee_row, ieee_col;
	int q_row, q_col;
	int i, j;
	int index_ieee, index_q;
	char
			*sql_req_config =
					"select ieee,nwkaddr,TermCode,Termtype,TermInfo,TermPeriod from etable where nwkaddr not null";
	char *sql_req_ieee =
			"SELECT  DISTINCT ieee,nwkaddr FROM etable where nwkaddr not null";

	cJSON *sroot;
	cJSON *NodeList_array, *NodeList_item;
	cJSON *SubNodeItem, *SubNodeArray;
	char *out = NULL;
	GDGL_DEBUG("send all energy config to server\n");
	//query database:
	q_data = sqlite_query_msg(&q_row, &q_col, sql_req_config);
	if (q_data == NULL)
	{
//		printf("query db error\n");
		sqlite_free_query_result(q_data);
		return;
	}
	ieee = sqlite_query_msg(&ieee_row, &ieee_col, sql_req_ieee);
	if (ieee == NULL)
	{
//		printf("query db error\n");
		sqlite_free_query_result(ieee);
		return;
	}
	//organize json package
	sroot = cJSON_CreateObject();
//	cJSON_AddNumberToObject(sroot, "MsgType", SERVER_UPLOAD_ALL_SECURITY_CONFIG_MSG);
	cJSON_AddNumberToObject(sroot, "MsgType", SERVER_UPLOAD_ALL_ENERGY_CONFIG_MSG); //modify by yanly150513
	cJSON_AddNumberToObject(sroot, "Sn", 10);
	cJSON_AddItemToObject(sroot, "NodeList", NodeList_item
			= cJSON_CreateArray());
	for (i = 0; i < ieee_row; i++)
	{
		index_ieee = i * ieee_col + ieee_col;//ieee data的每行头
		cJSON_AddItemToArray(NodeList_item, NodeList_array
				= cJSON_CreateObject());
		cJSON_AddStringToObject(NodeList_array, "IEEE", ieee[index_ieee]);
		cJSON_AddStringToObject(NodeList_array, "Nwkaddr", ieee[index_ieee+1]);
		cJSON_AddItemToObject(NodeList_array, "TermList", SubNodeItem
				= cJSON_CreateArray());
		for (j = 0; j < q_row; j++)
		{
			index_q = j * q_col + q_col;//data的每行头
			if (strcmp(q_data[index_q], ieee[index_ieee]) == 0)
			{
				cJSON_AddItemToArray(SubNodeItem, SubNodeArray
						= cJSON_CreateObject());
				cJSON_AddStringToObject(SubNodeArray, "TermCode", q_data[index_q+2]);
				cJSON_AddNumberToObject(SubNodeArray, "TermType", atoi(q_data[index_q+3]));
				cJSON_AddStringToObject(SubNodeArray, "TermInfo", q_data[index_q+4]);
				cJSON_AddNumberToObject(SubNodeArray, "TermPeriod", atoi(q_data[index_q+5]));
			} else
			{
			}
		}
	}
	out = cJSON_PrintUnformatted(sroot);
	printf("size:%d\n", strlen(out));
	send_msg_to_server(out, strlen(out), fd);
	//	json_msgsnd(MsgserverTxId, SERVER_UPLOAD_ALL_SECURITY_CONFIG_MSG, out, strlen(out));
	//free:
	cJSON_Delete(sroot);
	free(out);
	sqlite_free_query_result(q_data);
	sqlite_free_query_result(ieee);
	return;
}

