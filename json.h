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

#define online 1
#define offline 0

#define client_heart_count 30
//#define client_heart_count 5
//#define MAX_RXBUFFLEN 1024

#define  TERM_MSGTYPE  22

#define  HEART_MSGTYPE  1

/*client detach interface ***********************************************************///add yanly141229
#define	DETACH_PRASE_ERROR					-1
#define DETACH_MSGTYPE_ERROR				-2
#define DETACH_IEEE_NOT_FOUND_ERROR			-3
#define	DETACH_BELONG_ENERGY				0
#define	DETACH_BELONG_SECURITY				1
#define DETACH_BELONG_IN_COMMON				2
/************************************************************************************/

uint8 parse_json_client(char *text,uint8 textlen,int tmp_socket);
//void parse_json_client(client_status client_list);
void package_json_client(uint8 sendmsgtype,int msg_sn,uint16 tmp_socket);
void parse_json_node(char *text,uint8 textlen);
void package_json_server(msgform *msg_Rxque ,uint8 Rx_msgnum,uint16 socketflg);
void parse_json_server(char *text,uint8 textlen);

extern int parse_json_node_security(char *text,int textlen);
extern int detach_interface_msg_client(char *text,int textlen);//add yanly141229
extern int client_msg_handle_security(char *buff, int size, int fd);
extern int client_msg_handle_in_common(char *buff, int size, int fd);
extern int detach_5002_message22(char *text, int textlen);
#endif
