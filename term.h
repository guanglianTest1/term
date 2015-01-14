#ifndef _TERM_H
#define _TERM_H

#include"sysinit.h"




#define VAL_PARA					0x33

#define AMMTER_GET_PWVAL_REQ		0x43C3/*查询电量*/      //?
//#define AMMTER_GET_PWVAL_RES		0x43C3/*查询电量响应*/   //?
#define AMMTER_GET_PWVAL_RES		0xc343/*查询电量响应*/   //?

//#define AMMTER_GET_PWVAL_REQ		0xC343/*查询电量*/
//#define AMMTER_GET_PWVAL_RES		0xC343/*查询电量响应*/

#define AMMTER_BCGET_ADDR_REQ		0x65F3/*广播搜寻设备*/
#define AMMTER_GET_ADDR_RES			0x65F3/*搜寻设备回响应*/


#define MASTER_STATION_TXCMD        0x00
#define SLAVE_STATION_TXACK			0x01

#define SAVE_STATION_OKACK			0x00
#define SAVE_STATION_EXCACK			0x01

#define NEXT_NO_DATAFRM				0x00
#define NEXT_HAS_DATAFRM			0x01

#define READ_AMME_DATA				0x01/*读数据*/
#define READ_AMME_NEXTDATA			0x02/*读后续数据*/
#define READ_AMME_REDATA			0x03/*重读数据*/

#define WRITE_AMME_DATA				0x04/*写数据*/

#define BC_AMME_CLOCK				0x08 /*广播校时*/

#define WRITE_AMME_DEVADR			0x0A/*写设备地址*/

#define MODIY_AMME_DEVRATE			0x0C/*更改通信速率*/

#define MODIY_AMME_LGKEY			0x0F/*修改密码*/

#define BZERO_AMME					0x09/*总清零*/

#define OPEN_WATER_VALVE			0x05/*水表开阀请示帧*/
#define OPEN_LBWATER_REQMSG			0xC520/*打开郎贝水表请求*/

#define CLOSE_WATER_VALVE			0x06/*水表关阀请示帧*/
#define CLOSE_LBWATER_REQMSG		0xC521/*关闭郎贝水表请求*/
/******************************************************************************************************/
//define security serial protocol
#define	SENSOR_STATUS_ALARM						1
#define SENSOR_STATUS_NORMAL					2
#define	SENSOR_DEV_MAX_IN_ONE_NODE				5

#define SWITCH_STATUS_OPENED					1
#define SWITCH_STATUS_CLOSED					2
#define SWITCH_STATUS_INVERT					3
#define SWITCH_STATUS_MAX						4
#define	SWITCH_DEV_MAX_IN_ONE_NODE				4

#define	OPERATER_OPEN							1    //布防
#define OPERATER_CLOSE							2    //撤防
#define OPERATER_MAX							3


#define SECURITY_SERIAL_RESPOND_STATUS_SUCCESS	0

/******************************************************************************************************/
typedef struct{
	uint8 frmhead[3];/*固定填写FE FE FE*/
	uint8 frmStar;/*固定填写0x68*/
	uint8 devid[TERM_LEN];/*低字节在前*/
	uint8 frmEnd;/*固定填写0x68*/
	union
    {
		uint8 val;
		struct
		{
	    	uint8 command:5;
	    	uint8 nextdata:1;/*0无后续数据帧		    D5=1：有后续数据帧*/
	    	uint8 ack:1;/*0从站正确应答 1：从站对异常信息的应答*/
	    	uint8 rtxflg:1;/*传输方向 0由主站发起的命令帧 1由从站发出的应答帧*/
		} bita;
    } cmd; /*控制字节*/

    uint8 msglen;
    uint16 msgType;
/*	UINT8 msgdata[10];*/
}AmmeteMsgHead_T;


typedef struct{
	uint8 frmhead[4];/*固定填写FE FE FE*/
	uint8 frmStar;/*固定填写0x68*/
	uint8 devid[TERM_LEN];/*低字节在前*/
	uint8 frmEnd;/*固定填写0x68*/
	union 
    {
		uint8 val;
		struct
		{
			uint8 command:5;
			uint8 nextdata:1;/*0无后续数据帧		    D5=1：有后续数据帧*/
			uint8 ack:1;/*0从站正确应答 1：从站对异常信息的应答*/
			uint8 rtxflg:1;/*传输方向 0由主站发起的命令帧 1由从站发出的应答帧*/
		} bita;
    } cmd; /*控制字节*/
	
    uint8 msglen;
    uint16 msgType;
/*	UINT8 msgdata[10];*/
}AmmeteRxMsgHead_T;

typedef struct{
	    uint8 frmhead[3];/*固定填写FE FE FE*/
		uint8 frmStar;/*固定填写0x68*/
		uint8 devid[TERM_LEN];/*低字节在前*/
		uint8 frmEnd;/*固定填写0x68*/
		union
	    {
			uint8 val;
			struct
			{
				uint8 command:5;
				uint8 nextdata:1;/*0无后续数据帧		    D5=1：有后续数据帧*/
				uint8 ack:1;/*0从站正确应答 1：从站对异常信息的应答*/
				uint8 rtxflg:1;/*传输方向 0由主站发起的命令帧 1由从站发出的应答帧*/
			} bita;
	    } cmd; /*控制字节*/

	    uint8 msglen;
	    uint16 msgType;
	/*	UINT8 msgdata[10];*/
}WaterRxMsgHead_T;

/*广州郎贝电子485水表*/
typedef struct{
	    uint8 frmhead[2];/*固定填写FE FE FE*/
		uint8 frmStar;/*固定填写0x68*/
		uint8 devid[TERM_LEN];/*低字节在前*/
		uint8 frmEnd;/*固定填写0x68*/
		union
	    {
			uint8 val;
			struct
			{
				uint8 command:5;
				uint8 nextdata:1;/*0无后续数据帧		    D5=1：有后续数据帧*/
				uint8 ack:1;/*0从站正确应答 1：从站对异常信息的应答*/
				uint8 rtxflg:1;/*传输方向 0由主站发起的命令帧 1由从站发出的应答帧*/
			} bita;
	    } cmd; /*控制字节*/

	    uint8 msglen;
	    uint16 msgType;
	/*	UINT8 msgdata[10];*/
}lbWaterRxMsgHead_T;

/*广州郎贝电子485电表*/
typedef struct{
	    uint8 frmhead;/*固定填写FE FE FE*/
		uint8 frmStar;/*固定填写0x68*/
		uint8 devid[TERM_LEN];/*低字节在前*/
		uint8 frmEnd;/*固定填写0x68*/
		union
	    {
			uint8 val;
			struct
			{
				uint8 command:5;
				uint8 nextdata:1;/*0无后续数据帧		    D5=1：有后续数据帧*/
				uint8 ack:1;/*0从站正确应答 1：从站对异常信息的应答*/
				uint8 rtxflg:1;/*传输方向 0由主站发起的命令帧 1由从站发出的应答帧*/
			} bita;
	    } cmd; /*控制字节*/

	    uint8 msglen;
	    uint16 msgType;
	/*	UINT8 msgdata[10];*/
}lbAmmeteRxMsgHead_T;

#define lbAmmeteRxMsgHead_len 13


/*获取电量部分*/
typedef struct{
	AmmeteRxMsgHead_T msgHead;
	uint8 smdata;/*小数部分*/
	uint8 bigdata[3];/*整数部分*/
}GetAmmereValRes_T;
/******************************************************************************************************/
#define HEXBCDTODEC(c)					(((c)>>4)*10+((c)&0x0F))

#define DEV_TYPE_AMMETER				1/*电表*/
#define DEV_TYPE_COLDWATETER			2/*冷水表*/
#define DEV_TYPE_HOTWATETER			3/*热水表*/

/*��ȡ��������*/

#define FLAG_SEC        0x0001
#define FLAG_MIN        0x0002
#define FLAG_HOUR       0x0004
#define FLAG_HALFDAY    0x0008
#define FLAG_DAY        0x0010
#define FLAG_MONTH      0x0020
#define FLAG_YEAR       0x0040
#define FLAG_TIME       0x0100
#define FLAG_DATE       0x0200
#define FLAG_ALPHA      0x1000

typedef struct
{
	uint16	tm_sec;		/* Seconds: 0-59 (K&R says 0-61?) */
	uint16	tm_min;		/* Minutes: 0-59 */
	uint16	tm_hour;	/* Hours since midnight: 0-23 */
	uint16	tm_mday;	/* Day of the month: 1-31 */
	uint16	tm_mon;		/* Months *since* january: 0-11 */
	uint16	tm_year;	/* Years since 1900 */
	uint16	tm_wday;	/* Days since Sunday (0-6) */
	uint16	tm_yday;	/* Days since Jan. 1: 0-365 */
	uint16	tm_isdst;	/* +1 Daylight Savings Time, 0 No DST,
				 * -1 don't know */
}tm;



typedef struct {

    //uint8  *TermCode;      //智能表的编号
	 uint8 TermCode[TERM_LEN];
	 uint8  TermType;      //智能表的类型
	 uint32 ReportData;


	}TxMsg;

#define MsgComNative 1
#define MsgComClient 2

union test {
        uint32 a;
        uint8 b[4];
} test;
/******************************************************************************************************/
//void  SendmsgToclient(TxMsg Tmp_TxMsg);
void  SendmsgToclient(TxMsg TmpBuf);

void *term_msg_thread(void *p);

uint8 GetCheckCS(uint8 *dataBuf, uint8 datalen);

uint8 SendDataToDev(uint8 *termid,uint8 *nodeid);

void termGetValResClient(uint8* msgBuf);

void serialMsgDeal(uint8 *msgBuf,uint8 msg_len);

void unpack_term_msg(uint8 *sdata,uint8 slen);

int digit2time(char *timeStr, int iLen, tm *ptTime);
void term_msg_process();






//extern void getsystm(void);
//extern int prtfBuf(U8_T mod,U8_T *dataBuf, int length,char* info);

/********************************************************************/
extern void send_data_to_dev_security(char *nwkaddr, char *text, int text_len);

#endif/*End of _SHELL_H*/

