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




uint8 parse_json_client(char *text,uint8 textlen,int tmp_socket);
//void parse_json_client(client_status client_list);
void package_json_client(uint8 sendmsgtype,int msg_sn,uint16 tmp_socket);
void parse_json_node(char *text,uint8 textlen) ;
void package_json_server(msgform *msg_Rxque ,uint8 Rx_msgnum,uint16 socketflg);
void parse_json_server(char *text,uint8 textlen);

#endif
