/***************************************************************************
  Copyright (C),  2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  File name:      json.h
  Description:    定义封装和解析json API
  Author:         jiang   
  Version:        1.0       
  Date:           2014-03-19
  History:        
                  
    1. Date:
       Author:
       Modification:
    2. ...
***************************************************************************/
#ifndef _JSON_H
#define _JSON_H
#include"sysinit.h"
#include"timer.h"
#include"cJSON.h"
#define online 1
#define offline 0

#define client_heart_count 30
//#define client_heart_count 5
//#define MAX_RXBUFFLEN 1024

#define  TERM_MSGTYPE  						22
#define  ANNCE_CALLBACK						41   //add yan
#define  GLOBAL_ARM							33   //ADD YAN

#define  HEART_MSGTYPE  1

/*client detach interface ***********************************************************///add yanly141229
#define	DETACH_PRASE_ERROR					-1
#define DETACH_MSGTYPE_ERROR				-2
#define DETACH_IEEE_NOT_FOUND_ERROR			-3
#define	DETACH_BELONG_ENERGY				0
#define	DETACH_BELONG_SECURITY				1
#define DETACH_BELONG_IN_COMMON				2

/*json error ***********************************************************/
#define JSON_OK								0
#define JSON_KEY_ERROR						-5
#define JSON_VALUE_ERROR					-6
#define JSON_MSGTYPE_ERROR					-4
#define JSON_INSERT_DATABASE_ERROR			-7


#define MALLOC_ERROR						1


/************************************************************************************/
//extern int msghandle_ConfigQueAckMsg(cJSON *root,int tmp_socket);
int msghandle_ConfigQueAckMsg(int msg_sn,uint16 tmp_socket);
//int parse_json_client(char *text,uint8 textlen,int tmp_socket);
int parse_json_client(int msgtype,cJSON *root,int tmp_socket);
//void parse_json_client(client_status client_list);
//int ConfigQueAckMsg(uint8 sendmsgtype,int msg_sn,uint16 tmp_socket);
//int package_json_client(uint8 sendmsgtype,int msg_sn,int tmp_socket);
void parse_json_node(char *text,uint8 textlen);
void package_json_server(msgform *msg_Rxque ,uint8 Rx_msgnum,uint16 socketflg);
void parse_json_server(char *text,uint8 textlen);
int client_msg_handle_energy(char *text,int textlen,int tmp_socket);

extern int parse_json_node_security(char *text,int textlen);
extern int detach_interface_msg_client(char *text,int textlen);//add yanly141229
extern int client_msg_handle_security(char *buff, int size, int fd);
extern int client_msg_handle_in_common(char *buff, int size, int fd);
extern int detach_5002_message22(char *text, int textlen);
extern void client_msg_handle_in_msgtype_error(char *buff, int size, int fd);
extern int parse_received_server_msg(char *text);
extern void origin_callback_handle(char *text,int textlen);
extern void json_msgsnd(int id, int type, char *buff, int buff_size);

extern void json_msgsndclient(int id, int type, char *buff, int buff_size);
void set_global_opertor_status(char status);

#endif
