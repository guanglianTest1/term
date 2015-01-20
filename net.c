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
#include<sys/ioctl.h>
#include<string.h>
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



extern int MsgtermTxId; //���ͳ������ݵ���Ϣ����
//extern int MsgtermRxId; //���ܳ������ݵ���Ϣ����
extern int MsgserverTxId;//���ͷ��������ݵ���Ϣ����
extern uint8 client_flag;

extern uint8 msgfromflg;
extern uint8 term_send_flag;
extern uint8 term_rcv_flag;
extern uint8 client_send_count;
extern uint8 server_heart_flag;
extern uint8 node_heart_flag;
extern pthread_mutex_t mutex;
/****************************************************************************/
uint8 Client_Sn=0;
int Client_Socket=0;
uint8 listen_thread_isover=0;
#if 0
node_list node_table[NODE_NUM]={
		                        {"1234567890","ce75",{{{0x00,0x00,0x00,0x11,0x25,0x52},1,"was",60,0,0,0,0,0,0,0,0},{{0x00,0x00,0x12,0x34,0x56,0x78},2,"water",60,0,0,0,0,0,0,0,0},{{0xaa,0x12,0x34,0x56,0x78,0x90},3,"water",60,0,0,0,0,0,0,0,0}}},
		                        {"1234567890","7436",{{{0x04,0x04,0x04,0x04,0x04,0x04},1,"war",60,0,0,0,0,0,0,0,0},{{0x05,0x05,0x05,0x05,0x05,0x05},2,"water",60,0,0,0,0,0,0,0,0},{{0x06,0x06,0x06,0x06,0x06,0x06},2,"water",60,0,0,0,0,0,0,0,0}}}
                               };
uint8 Term_Num[NODE_NUM]={3,3};
#endif

#if 1
node_list node_table[NODE_NUM]={0};
uint8 Term_Num[NODE_NUM]={0};
#endif
char local_addr[1025];//add yan150112
/**********************************************************************************************/
pthread_t thread_do[2];  //pthread id
//int handle_connect(int sockfd);
//int handle_request();
//static function
void *handle_connect(void *argv);
void *handle_request(void *argv);
void sig_process(int signo);
void sig_pipe(int signo);
static int	connect_to_gateway_init(void);//add yanly141229
static int send_server_heartbeat(int fd);
static void close_client(int i);
static void set_heart_beat(int i, char heart_status);
void send_msg_to_server(char *text, int text_size, int fd);
void send_all_security_config_to_server(int fd);


//uint8 node_ip[20]="192.168.0.102";
client_status client_list[MAX_CLIENT_NUM];
int connect_host[MAX_CLIENT_NUM];//add yanly141229
char connect_host_online[MAX_CLIENT_NUM] = {HEARTBEAT_NOT_OK}; //add yan150115增加检测心跳包,规定时间内没收到心跳包断开连接
uint16 client_num=0;   //how many client socket
//int connect_number = 0;

void client_type_socket()
{   pthread_t client_type;
	pthread_create(&client_type,NULL,client_type_thread,NULL);
}
void *client_type_thread(void *p)
{
  msgform term_msg;
  uint8 msgsize;
  uint8 term_code[TERM_LEN]={0};
  uint8 node_id[NODE_LEN]={0};
  uint8 i=0;
  while(1)
  	{
#if 1
     msgsize=msgrcv(MsgtermTxId, &term_msg,sizeof(msgform),0,0);
     printf("msgsize=%d\n",msgsize);
	 if((msgsize>0)&&(term_send_flag==enable))
       {
		memcpy(term_code,term_msg.mtext,TERM_LEN);
		printf("client term_code=");
		for(i=0;i<TERM_LEN;i++)
		{printf("%02x",term_code[i]);}
		printf("\n");
		memcpy(node_id,(term_msg.mtext+TERM_LEN),NODE_LEN);
		printf("node_id=%s\n",node_id);
		msgfromflg=MsgComClient;
		SendDataToDev(term_code,node_id);
		client_send_count++;

	   }
     if(client_send_count<=3)
	   {
	     if(term_rcv_flag==enable)
	     	{
             //oneclient_send_flag=enable;//��ͻ��˷��ͳ�����Ӧ��Ϣ
			 term_send_flag=enable;
			 term_rcv_flag=disable;
			 client_send_count=0;
			// printf("22222");
	        }
	     else
	     {
	    	 msgfromflg=MsgComClient;
	    	 SendDataToDev(term_code,node_id);
	     }

       }
	  else
	   {
          term_send_flag=enable;
		  client_send_count=0;
	   }
#endif


     if(client_flag==enable)
     	{
     	 for(i=0;i<MAX_CLIENT_NUM;i++)
          {

     		printf("client_count%d=%d\n",i+1,client_list[i].client_count);
		   if(client_list[i].client_count>0)
            {
			  client_list[i].client_count--;
              client_flag=disable;
			  //client_list[i].client_alive=online;
		   	}
		   else
		 	{
             client_list[i].client_alive=offline;
             client_list[i].client_count=0;
			 client_flag=disable;
			 DBG_PRINT("yangguixin5\n");
			// exit(1);
		    }
		   printf("client_status%d=%d\n",i+1,client_list[i].client_alive);
     	  }

	    }

    }
  return 0;
}


//void ConnectClient()
//{
//	pthread_t client_thread;
//	pthread_create(&client_thread,NULL,client_msg_thread,NULL);
//}
//void *client_msg_thread(void *p)
//int ConnectClient()
//{
//    struct sockaddr_in servaddr;
//    int s32Rtn;
//    int s32Socket_value = 1;
//
//    unsigned int pTcpPort;
//    int sockfd;
////	char receive_buf[MAXBUF];
////	 int Recvsize_2 = 0;
//	uint8 i=0;
//	//pthread_t thread_do[2];
//	 for(i=0;i<MAX_CLIENT_NUM;i++)
//	 {
//		 client_list[i].client_socket=-1;
//	 }
//	 signal(SIGINT, sig_process);
//	 signal(SIGPIPE, sig_pipe);
//    pTcpPort =CLIENT_PORT;
//    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    if (sockfd == -1)
//    { perror("client socket error!");}
//
////    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &s32Socket_value,sizeof(int)) == -1)
////    { printf("client set socketopt err!!!!\n");
////	  goto ERR;
////    }
//
//    memset(&servaddr, 0, sizeof(servaddr));
//    servaddr.sin_family = AF_INET;
//    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//    servaddr.sin_port = htons(pTcpPort);
//
//    s32Rtn = bind(sockfd, (struct sockaddr *) &servaddr,sizeof(struct sockaddr_in));
//    if (s32Rtn < 0)
//    {
//    	printf(" ****listen client  connected  listening on port %d, bind err, %d !!!!\n", pTcpPort,s32Rtn);
//    	goto ERR;
//    }
//    else
//    {
//        printf(" **** listen client connected  listening on port %d \n", pTcpPort);
//    }
//
//    s32Rtn = listen(sockfd, 5);
//    if (s32Rtn < 0)
//    {
//        printf("client listening on port %d, listen err, %d \n", pTcpPort,s32Rtn);
//	    goto ERR;
//    }
//    return sockfd;
//   // else
//    	//printf("---listen client connected thread start----\r\n");
//
//	//memset(receive_buf,0,MAXBUF);
//	//memset(client_list,0,sizeof(client_status));
//
////while(1)
//	//{
//
//	  //handle_connect(sockfd);
//	  //handle_request();
//	  //printf(" yyyyyyyyy\n ");
////	  pthread_create(&thread_do[0], NULL, handle_connect, (void*)&sockfd);
////	  pthread_create(&thread_do[1], NULL, handle_request, NULL);
//      // break;
//	//}
//ERR:
//      //close(client_list[client_num].client_socket);
//	  //close(sockfd);
//  //    printf(" socket connected thread stop \n ");
//      listen_thread_isover = 1;
//     return 0;
//}
int ConnectClient()
{
	int s_s;
	int optval =1;
	struct sockaddr_in local;
	int i;
	for(i=0; i<MAX_CLIENT_NUM;i++)
	{
		connect_host[i] = -1;
	}
//	memset(connect_host, -1, MAX_CLIENT_NUM); //用memset发现有些值没有被置-1；
//	for(i=0; i<MAX_CLIENT_NUM;i++)
//	{
//		printf("connect host:%d\n",connect_host[i]);
//	}
	//SETUP tcp socket
	s_s = socket(AF_INET, SOCK_STREAM, 0);
	if (setsockopt(s_s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)//允许重复使用本地地址和端口?
	{
		perror("setsockopt");
		exit(1);
	}
	printf("server fd:%d\n",s_s);
	//server_fd =s_s;
	//init addr
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(argv[1]);//htonl(INADDR_ANY); //local addr anyway
    local.sin_port = htons(CLIENT_PORT);

    //bind
    bind(s_s, (struct sockaddr *)&local, sizeof(local));
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
    char ret =0;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return ;
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        if(strcmp(ifa->ifa_name, "eth0")!=0)
        	continue;
		s = getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),
				local_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0) {
			printf("getnameinfo() failed: %s\n", gai_strerror(s));
		}
		else
		{
			printf("address:%s\n", local_addr);
			ret =1;
			break;
		}
    }
    if(ret !=1){
    	printf("get local addr error\n");
    	exit(1);
    }
    freeifaddrs(ifaddr);
}
//
void node_msg_process()
{
	pthread_t node_thread;
	pthread_create(&node_thread,NULL,node_msg_thread,NULL);

}
void *node_msg_thread(void *p)
{

  while(1)
  {
	  nodeSocket();
  }
  return NULL;
}

void server_msg_process()
{
	pthread_t server_thread;
	pthread_create(&server_thread,NULL,server_msg_thread,NULL);
}
void *server_msg_thread(void *p)
{

  while(1)
  {
	  ServerSocket();
  }
  return NULL;
}


//int nodeSocket()
//    {
//        uint8 msg[20] = "0200070007";
//		//int bytes_sent=0;
//		int cfd=0;                                           //�ļ�������
//		int recbytes=0;
//		char buffer[MAXBUF]={0};	                          //���ܻ�����
//		struct sockaddr_in s_add;                   //�洢����˺ͱ��˵�ip���˿ڵ���Ϣ�ṹ��
//		uint16 nodeport=0;
//		uint8 rc=0;
//		//cJSON *root;
//	    //char *out=NULL;
//
//		nodeport=NODE_PORT;
//	    //nodeport=5002;
//		cfd = socket(AF_INET, SOCK_STREAM, 0);           //����socket ʹ����������TCP������
//		if(cfd == -1 )
//		{
//			printf("node socket fail ! \r\n");
//			return -1;
//		}
//		printf("node socket ok !\r\n");
//
//		bzero(&s_add,sizeof(struct sockaddr_in));        //����������˵�ip�Ͷ˿���Ϣ������ṹ����Բ�����
//		s_add.sin_family=AF_INET;
//		s_add.sin_addr.s_addr= inet_addr(GATEWAY_IPADDR);    //ipת��Ϊ4�ֽ����Σ�ʹ��ʱ��Ҫ���ݷ����ip���и���
//		s_add.sin_port=htons(nodeport);
//
//		printf("node_addr = %#x ,nodeport=: %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port); // �����ӡ������С�˺�����ƽʱ���������෴�ġ�
//
//		if((rc=connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))==-1 )// �ͻ������ӷ���������������Ϊsocket�ļ�����������ַ��Ϣ����ַ�ṹ��С
//		{
//			printf("node connect fail !\r\n");
//			return -1;
//		}
//		else
//		{
//		  printf("node socket connect ok ! rc=%d\r\n",rc);
//		  if(node_heart_flag==enable)
//		    {
//			  node_heart_flag=disable;
//			  printf("send node heart beat\n");
//			  send(cfd,msg,sizeof(msg),MSG_DONTWAIT);
//			 }
//
//		  memset(buffer,0,MAXBUF);
//          recbytes = recv(cfd, buffer, MAXBUF,0);     //���ӳɹ�,�ӷ���˽����ַ�
//          DBG_PRINT("recbytes= %d\n",recbytes);
//
//		  if(recbytes==-1)
//		  {
//			printf("node read data fail !\r\n");
//			return -1;
//		  }
//         else if(recbytes>0)
//		  {
//            //DBG_PRINT("3333\n");
//		    printf("node rxbuffer=%s\r\n",buffer);
//		    if((buffer[0]=='{')&&(buffer[recbytes-1]=='}'))
//	        {parse_json_node(buffer,recbytes);}
//		    else
//		    {printf("receive node data error\n");}
//       	  }
//
//	}
//		//close(cfd);
//		return 0;
//
//}

/*
 * connect gateway 5018 init
 * */
static int connect_to_gateway_init(void)
{
	int cfd=0;
	struct sockaddr_in s_add;
	int rc=-1;
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(cfd == -1 )
	{
		printf("node socket fail ! \r\n");
		return -1;
	}
	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr= inet_addr(local_addr);//modify by yan
	s_add.sin_port=htons(NODE_PORT);
//	do
//	{
//		printf("gateway 5018 not connected!\n");
//		sleep(RECONNECT_GATAWAY5018_TIME);
		rc = connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr));
//	}
//	while(rc ==-1);
	if(rc !=-1)
		printf("connected gateway 5018 socket success,socket:%d\n",cfd);
	return cfd;
}
/*
 * connect server 5040 init
 * */
int connect_to_server_init(void)
{
	int cfd=0;
	struct sockaddr_in s_add;
	int rc=-1;
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(cfd == -1 )
	{
		printf("connect to server socket setup fail ! \r\n");
		return -1;
	}
	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr= inet_addr(local_addr);//(SERVER_IPADDR_DEBUG);//debug by yan
	s_add.sin_port=htons(SERVER_PORT);
	printf("server not connected!\n");
	do
	{
		sleep(RECONNECT_SERVER_TIME);
		rc = connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr));
	}
	while(rc ==-1);

	printf("connected server socket success,socket:%d\n",cfd);
	sleep(RECONNECT_SERVER_TIME);
	//网关首次连接到服务器，发送配置给服务器
	send_all_security_config_to_server(cfd);
	return cfd;
}
/*
 * 处理5018callback的单元
 * */
int nodeSocket() //modify141230
{
	uint8 msg[20] = "0200070007";
	int cfd;
	char buffer[1024];
	int recbytes;
	int s_status;
	int detach_obj;
	cfd = connect_to_gateway_init();
	memset(buffer,0,MAXBUF);
	while(cfd)// connected
	{
		if(node_heart_flag == enable)
		{
			node_heart_flag=disable;
			//printf("send node heart beat\n");
			s_status = send(cfd,msg,sizeof(msg),MSG_DONTWAIT);
			if(s_status ==-1)
			{
				perror("connected gateway 5018,but send error\n");
				close(cfd);
				return -1;
			}
			else if(s_status ==0)
			{
				printf("connected gateway 5018,but the other side close this socket\n");
				close(cfd);
				return -2;
			}
			else
			{}//normal send
		}
		recbytes = recv(cfd, buffer, MAXBUF,MSG_DONTWAIT);     //���ӳɹ�,�ӷ���˽����ַ�
		if((recbytes==-1)&&(errno!=EAGAIN))
		{
			//printf("recbytes:%d\n",recbytes);
			perror("node read data fail");
			close(cfd);
			return -1;
		}
		else if(recbytes>0)
		{
			printf("receive 5018 size:%d \n",recbytes);
			detach_obj = detach_5002_message22(buffer,recbytes);
			//printf("5002message22:%d\n",detach_obj);
			//detach_obj = DETACH_BELONG_SECURITY;
			switch(detach_obj)
			{
				case  DETACH_BELONG_ENERGY:     /*这个case处理节能的信息*/
					//parse_json_node(buffer,recbytes);
					break;
				case  DETACH_BELONG_SECURITY:	/*这个case处理安防的信息*/
					parse_json_node_security(buffer,recbytes);
					break;
				case  DETACH_BELONG_IN_COMMON:
					origin_callback_handle(buffer, recbytes); //设备节点重新上电产生的callback处理
					break;
				default :
					break;
			}
			memset(buffer,0,MAXBUF);
		}
		else if(recbytes==0)
		{
			printf("connected gateway 5018,but the other side close this socket\n");
			close(cfd);
			return -2;
		}

	}
	//close(cfd);
	return 0;
}
/***************************************************************************************/
/*
 * 处理服务器业务的单元
 * 采用轮询方式：执行发送心跳包>>发送消息队列数据>>接收服务器发过来的消息,不阻塞
 * */
int ServerSocket() //modify141230
{
	int cfd;
	char buffer[1024];
	int recbytes;
	int ret;
	int r_msg_num;
	msgform msg_Rxque;
	cfd = connect_to_server_init();
	memset(buffer,0,MAXBUF);
	while(cfd)// connected
	{
//心跳
		if(send_server_heartbeat(cfd) <0)
			return -1;
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
		}
//接收server msg
		recbytes = recv(cfd, buffer, MAXBUF,MSG_DONTWAIT);
		if((recbytes==-1)&&(errno!=EAGAIN))
		{
			//printf("recbytes:%d\n",recbytes);
			perror("socket read server data fail");
			close(cfd);
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
			close(cfd);
			return -2;
		}

	}
	//close(cfd);
	return 0;
}
/*
 * send server heartbeat
 * */
static int send_server_heartbeat(int fd)
{
	cJSON* root;
	char *out;
	int ret = 1;

	if(server_heart_flag == enable)
	{
		server_heart_flag = disable;

		root=cJSON_CreateObject();
	    cJSON_AddNumberToObject(root,"MsgType", 64);
	    cJSON_AddNumberToObject(root,"Sn",		10);
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
		{}//normal send
	    cJSON_Delete(root);
	    free(out);
	}
    return ret;
}
/***************************************************************************************/
uint8 ServerHeart_sn=0;
#if 0
int ServerSocket()
    {
//        uint8 msg[20] = "0200070007";
		//int bytes_sent=0;
		int cfd=0;                                           //�ļ�������
		int recbytes=0;
		char buffer[MAXBUF]={0};	                          //���ܻ�����
		struct sockaddr_in s_add;                   //�洢����˺ͱ��˵�ip���˿ڵ���Ϣ�ṹ��
		uint16 serverport=0;
		uint8 rc=0;
		uint8 Rx_msgnum=0;
		msgform msg_Rxque;
		cJSON *root;
	    char *out=NULL;

		serverport=SERVER_PORT;

		cfd = socket(AF_INET, SOCK_STREAM, 0);           //����socket ʹ����������TCP������
		if(cfd == -1 )
		{
			printf("server socket fail ! \r\n");
			return -1;
		}
		printf("server socket ok !\r\n");

		bzero(&s_add,sizeof(struct sockaddr_in));        //����������˵�ip�Ͷ˿���Ϣ������ṹ����Բ�����
		s_add.sin_family=AF_INET;
		s_add.sin_addr.s_addr= inet_addr(SERVER_IPADDR);
		s_add.sin_port=htons(serverport);

		printf("server_addr = %#x ,serverport=: %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port); // �����ӡ������С�˺�����ƽʱ���������෴�ġ�

		if((rc=connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))==-1 )// �ͻ������ӷ���������������Ϊsocket�ļ�����������ַ��Ϣ����ַ�ṹ��С
		{
			printf("server connect fail !\r\n");
			return -1;
		}
		else
		{
		  printf("server socket connect ok ! rc=%d\r\n",rc);

		//  if(server_heart_flag==enable)
		  if(0)
		    {
			  server_heart_flag=disable;
			  printf("send server heart beat\n");
			  root=cJSON_CreateObject();

			  cJSON_AddNumberToObject(root,"MsgType", HeartReportMsg);
			  cJSON_AddNumberToObject(root,"Sn",ServerHeart_sn);
			  out=cJSON_PrintUnformatted(root);
			  cJSON_Delete(root);
			  printf("out=%s\n",out);
			  send(cfd,out,strlen(out),MSG_DONTWAIT);
			  //send(cfd,msg,sizeof(msg),MSG_DONTWAIT);
			 }

		   Rx_msgnum=msgrcv(MsgserverTxId,&msg_Rxque,sizeof(msgform),0,0);
		   printf("server msg_Rxque.mtype=%04x\n",msg_Rxque.mtype);
		   printf("server Rx_msgnum=%d\n",Rx_msgnum);
		   if(Rx_msgnum!=0)
		 	{
		 		 package_json_server(&msg_Rxque,Rx_msgnum,cfd);

		    }


		  memset(buffer,0,MAXBUF);
          recbytes = recv(cfd, buffer, MAXBUF,0);
          DBG_PRINT("recbytes= %d\n",recbytes);
		  if(recbytes==-1)
		  {
			printf("server read data fail !\r\n");
			return -1;
		  }
         else if(recbytes>0)
		  {
            //DBG_PRINT("3333\n");
		    printf("server rxbuffer=%s\r\n",buffer);
		    if((buffer[0]=='{')&&(buffer[recbytes-1]=='}'))
		    { parse_json_server(buffer,recbytes);}
		    else
		    {printf("receive server data error\n");}
       	  }

	   }
		//close(cfd);
		return 0;
}
#endif
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
	for(i = 0; i<MAX_CLIENT_NUM; i++){
		if(connect_host[i] == client_fd){

			set_heart_beat(i, HEARTBEAT_OK);
		}
	}
}
/*
 * 检测心跳单元
 * */
void *check_client_heartbeat()
{
	int i;
	while(1){
		if(client_flag == enable){
			client_flag = disable;
			for(i=0; i<MAX_CLIENT_NUM; i++)
			{
				if(client_num<=0)
					break;
				if(connect_host[i] != -1)
				{
					if(connect_host_online[i] == HEARTBEAT_NOT_OK){
						printf("Didn't receive the heartbeat packets within the regulation time:clolse socket%d\n",connect_host[i]);
						close_client(i);
					}
					else{
						set_heart_beat(i, HEARTBEAT_NOT_OK);
					}
				}
			}
		}
	}
}
/*
 * 客户端连接断开，进行关闭操作
 * */
static void close_client(int i)
{
	close(connect_host[i]);
	pthread_mutex_lock(&mutex);
	connect_host[i] =-1;
	client_num--;
	pthread_mutex_unlock(&mutex);
}
/*
 * 建立客户端连接单元
 * */
void *handle_connect(void *argv)
{
	int s_s = *((int *)argv);
	struct sockaddr_in from;
	socklen_t len = sizeof(from);
	for(;;)
	{
		int i = 0;
		int s_c = accept(s_s, (struct sockaddr*)&from, &len);
		printf("a client connect,from: %s,sock:%d \n",inet_ntoa(from.sin_addr),s_c);
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
void *handle_request(void *argv)
{
	char buff[MAXBUF];
	char respondBUff[1024];
	int n=0;
	int maxfd =-1;
	fd_set scanfd;
	struct timeval timeout;
	timeout.tv_sec =1;
	timeout.tv_usec =0;
	int i =0;
	int err = -1;
	int __buffType;

	for(;;)
	{
		maxfd =-1;
		FD_ZERO(&scanfd);
		for(i=0;i<MAX_CLIENT_NUM;i++)  //resolve scanfd and maxfd
		{
			if(connect_host[i] !=-1)
			{
				FD_SET(connect_host[i], &scanfd);
				if(maxfd< connect_host[i])
				{
					maxfd = connect_host[i];
				}
			}
		}
		//select wait socket
		err = select(maxfd+1, &scanfd, NULL, NULL, &timeout);
		switch(err)
		{
			case 0:	break;		//timeout
			case -1: break;		//error happen
			default:
				if(client_num<=0)
					break;
				for(i=0; i<MAX_CLIENT_NUM; i++)
				{
					if(connect_host[i] !=-1)
					{
						if(FD_ISSET(connect_host[i], &scanfd))
						{
							memset(buff, 0, MAXBUF);
							n = recv(connect_host[i], buff, MAXBUF, 0);
							if(n>0)
							{
								//printf("something recv ,size: %d, connect file:%d \n",n,connect_host[i]);
								//printf("buff[0]=%d,buff[n-1]=%d \n",buff[0],buff[n-1]);
								//printf("sever recieve data:%s \n", buff);
//									memcpy(client_list[i].client_buff,buff,n);
							   // printf("**client_buff=%s ****\r\n",buff);
//									client_list[i].client_buff_len=n;
								__buffType = detach_interface_msg_client(buff, n);//detach interface
								switch(__buffType)
								{
									case  DETACH_PRASE_ERROR:
										memcpy(respondBUff,"data format error:", 18);
										memcpy(respondBUff+18,buff, n);
										send(connect_host[i],respondBUff, n+18, 0);
										printf("received client message,but prase json error\n");
										break;
									case  DETACH_MSGTYPE_ERROR:
										client_msg_handle_in_msgtype_error(buff, n, connect_host[i]);
										break;
/*这个case处理节能的客户端信息*/			case  DETACH_BELONG_ENERGY:
										//parse_json_client(buff, n, connect_host[i]);
										break;
/*这个case处理安防的客户端信息*/			case  DETACH_BELONG_SECURITY:
										//printf("received client message >>for security >>");
										client_msg_handle_security(buff, n, connect_host[i]);
										break;
									case DETACH_BELONG_IN_COMMON:
										client_msg_handle_in_common(buff, n, connect_host[i]);
										break;
									default:break;
								}
								//send(connect_host[i],buff,strlen(buff),0);
								memset(buff, 0, MAXBUF);
							}
							else if(n==0)
							{
								printf("socket%d disconneted!\n",connect_host[i]);
								connect_host[i] = -1;
								client_num --;
								close(connect_host[i]);
							}
							else if(n==-1)
							{
								printf("socket%d disconneted!\n",connect_host[i]);
								connect_host[i] = -1;
								client_num--;
								close(connect_host[i]);
							}
						}
					}
				}
			break;
		}
	}
	//free
	for(i=0;i<MAX_CLIENT_NUM;i++)  //resolve scanfd and maxfd
	{
		if(connect_host[i]!=-1)
		{
			close(connect_host[i]);
		}
	}
	return NULL;
}
//int handle_request()
#if 0
void *handle_request(void *argv)
{
	char receive_buf[MAXBUF]={0};
	int Recvsize_2 = 0,i=0;

  while(1)
 {
	for(i=0;i<MAX_CLIENT_NUM;i++)
	{
		if(client_list[i].client_socket>=0)
		{
			Recvsize_2 = recv(client_list[i].client_socket, receive_buf,sizeof(receive_buf), 0);

	        printf("***Recvsize_2=%d ****\r\n",Recvsize_2);
	        printf("***client_socket=%02x ****\r\n",client_list[i].client_socket);
	       // printf("***client_num=%d ****\r\n",client_num);
	        //if(receive_buf != NULL)
	        if(Recvsize_2 > 0)
	        {

	          //if((receive_buf[0]=='{')&&(receive_buf[Recvsize_2-1]=='}'))
	        	if(receive_buf[0]=='{')
	            {printf("receive json=%s\r\n",receive_buf);
	           // printf("***receive client json ****\r\n");

	            memcpy(client_list[i].client_buff,receive_buf,Recvsize_2);

	            printf("**client_buff=%s ****\r\n",client_list[i].client_buff);

				client_list[i].client_buff_len=Recvsize_2;

	            parse_json_client(client_list[i].client_buff,client_list[i].client_buff_len,client_list[i].client_socket);
	          }

	          else
	          {
	        	  printf("receive client data error\n");
	        	  //goto ERR;

	          }


	        }
	        else
	        {

	            printf("**** client socket no receive data !!!!\r\n");
	            //goto ERR;
	        }

	     }
	  }
		//break;
	 }
	 return NULL;

}
#endif
void sig_process(int signo)
{
	int i;
	for(i=0;i<MAX_CLIENT_NUM;i++)  //resolve scanfd and maxfd
	{
		if(connect_host[i]!=-1)
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
//发送给所有在线客户端   //add yanly141230
void send_msg_to_all_client(char *text, int text_size)
{
	int i;
	for(i=0; i<MAX_CLIENT_NUM; i++)
	{
		if(client_num<0)
		{
			printf("send msg to all client error: no client is connected!\n");
			break;
		}
		if(connect_host[i] != -1)
		{
			if(send(connect_host[i], text, text_size, 0)<=0)
			{
//				connect_host[i] =-1;
//				client_num--;
//				close(connect_host[i]);
				close_client(i);
			}
			//printf("send msg to all client,this is socket%d>>",connect_host[i]);
		}
	}
	printf("send msg to all client\n");
}
//发送给one客户端
void send_msg_to_client(char *text, int text_size, int fd)
{
	int i;
	if(send(fd,text,text_size,0)<=0)
	{
		//send error handle
		for(i=0;i<MAX_CLIENT_NUM;i++)
		{
			if(connect_host[i] == fd)
			{
//				connect_host[i] = -1;
//				client_num --;
				close_client(i);
				break;
			}
		}
		//close(fd);
		printf("sock%d send error ,may be disconneted\n",fd);
	}
}
//发送给server
void send_msg_to_server(char *text, int text_size, int fd)
{
	if(send(fd,text,text_size,0)<=0)
	{
		close(fd);
		printf("server sock%d send error ,may be disconneted\n",fd);
	}
}
/*
 * 网关连接到服务器后，发送安防表配置信息给服务器
 * */
void send_all_security_config_to_server(int fd)
{
	char **q_data;
	char **ieee;
	int ieee_row,ieee_col;
	int q_row,q_col;int i,j;
	int index_ieee,index_q;
	char *sql_req_config = "select ieee,nwkaddr,type,subtype,num,info,operator from stable where nwkaddr not null";
	char *sql_req_ieee = "SELECT  DISTINCT ieee,nwkaddr FROM stable where nwkaddr not null";

	cJSON *sroot;
	cJSON *NodeList_array, *NodeList_item;
	cJSON *SubNodeItem, *SubNodeArray;
    int opt ;
    char *out=NULL;
    printf("when connect server, send all security config to server\n");
//query database:
    q_data = sqlite_query_msg(&q_row, &q_col, sql_req_config);
    if(q_data ==NULL)
    {
    	printf("query db error\n");
    	sqlite_free_query_result(q_data);
    	return;
    }
    ieee = sqlite_query_msg(&ieee_row, &ieee_col,sql_req_ieee);
    if(ieee == NULL)
    {
    	printf("query db error\n");
    	sqlite_free_query_result(ieee);
    	return;
    }
    opt = sqlite_query_global_operator();
    if(opt ==-1)
    {
    	printf("query db error\n");
    	return;
    }
//organize json package
    sroot = cJSON_CreateObject();
	cJSON_AddNumberToObject(sroot, "MsgType", SERVER_UPLOAD_ALL_SECURITY_CONFIG_MSG);
	cJSON_AddNumberToObject(sroot, "Sn", 10);
	cJSON_AddNumberToObject(sroot, "GlobalOpt", opt);
	cJSON_AddItemToObject(sroot, "NodeList", NodeList_item =cJSON_CreateArray());
	for(i=0;i<ieee_row;i++)
	{
		index_ieee = i*ieee_col+ieee_col;//ieee data的每行头
		cJSON_AddItemToArray(NodeList_item, NodeList_array=cJSON_CreateObject());
		cJSON_AddStringToObject(NodeList_array, "SecurityNodeID", ieee[index_ieee]);
		cJSON_AddStringToObject(NodeList_array, "Nwkaddr", ieee[index_ieee+1]);
		cJSON_AddItemToObject(NodeList_array, "SubNode", SubNodeItem =cJSON_CreateArray());
		for(j=0;j<q_row;j++)
		{
			index_q = j*q_col + q_col;//data的每行头
			if(strcmp(q_data[index_q], ieee[index_ieee])==0)
			{
				cJSON_AddItemToArray(SubNodeItem, SubNodeArray=cJSON_CreateObject());
				cJSON_AddNumberToObject(SubNodeArray, "Type", atoi(q_data[index_q+2]));
				cJSON_AddNumberToObject(SubNodeArray, "SubType", atoi(q_data[index_q+3]));
				cJSON_AddNumberToObject(SubNodeArray, "Num", atoi(q_data[index_q+4]));
				cJSON_AddStringToObject(SubNodeArray, "Info", q_data[index_q+5]);
				cJSON_AddNumberToObject(SubNodeArray, "OperatorType", atoi(q_data[index_q+6]));
			}
			else
			{}
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
