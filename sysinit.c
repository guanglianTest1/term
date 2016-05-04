/*
 * sysInit.c
 *
 *  Created on: Mar 19, 2014
 *      Author: ema
 */

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include"json.h"
#include"cJSON.h"
#include"net.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"
#include "appSqlite.h"

#define MSG_QUE_ID             0x12345
#define MSG_RXQUE_ID           0x54321
#define MSG_SERVERQUE_ID       0x11223
#define MSG_TERM_ID            0x33221

int MsgtermTxId;
int MsgtermRxId;
int MsgserverTxId;
int MsgTermNativeId;
void sysInit()
{

	// buf_init();

	if ((MsgtermTxId = msgget(MSG_QUE_ID, 0)) < 0)
	{
		if ((MsgtermTxId = msgget(MSG_QUE_ID, IPC_CREAT | 0666)) < 0)
			DBG_PRINT(" no MsgtermTxId\n");
		//return 1;
	}

	if ((MsgtermRxId = msgget(MSG_RXQUE_ID, 0)) < 0)
	{
		if ((MsgtermRxId = msgget(MSG_RXQUE_ID, IPC_CREAT | 0666)) < 0)
			DBG_PRINT(" no MsgtermRxId\n");
		//return 2;
	}

	if ((MsgserverTxId = msgget(MSG_RXQUE_ID, 0)) < 0)
	{
		if ((MsgserverTxId = msgget(MSG_SERVERQUE_ID, IPC_CREAT | 0666)) < 0)
			DBG_PRINT(" no MsgserverTxId\n");
		//return 3;
	}

	if ((MsgTermNativeId = msgget(MSG_TERM_ID, 0)) < 0)
	{
		if ((MsgTermNativeId = msgget(MSG_TERM_ID, IPC_CREAT | 0666)) < 0)
			DBG_PRINT(" no MsgTermNativeId\n");
		//return 1;
	}

	//getConfig();   //�������ļ����������ݴ��޸��
	//openterm();    //ȷ���Ƿ���Ҫ�����ܱ�Ĳ���?
}

