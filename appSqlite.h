/*
 * appSqlite.h
 *
 *  Created on: Jan 5, 2015
 *      Author: yanly
 */
#include "user_config.h"
#ifndef APPSQLITE_H_
#define APPSQLITE_H_
/**********************************************************/  //serity table: database column field define
#define		SECURITY_SENSOR_TYPE			1
#define		SECURITY_SWITCH_TYPE			2

#define   ERROR  -1
#define   OK      0
typedef struct
{
	char type;
	char subtype;
	char num;
	char *info;
	char operator;
}subsecurityConfig_t;

typedef struct
{
	char *ieee;
	char *addr;
	char globalOpt;
	subsecurityConfig_t subNode_config;
}securityConfig_t;


#if 1


#define  INFOLEN 128


typedef struct {
	 char  TermCode_native[INFOLEN];      //智能表的编号
	 char  TermType_native;      //智能表的类型
	 char  TermInfo_native[INFOLEN];      //智能表的描述
	 char  TermPeriod_native;    //智能表的上报周期
	}term_native;


typedef struct {
	 char IEEE_native[INFOLEN];
     char Nwkaddr_native[INFOLEN];
     term_native termtable_native[TERM_NUMM];
	}node_native;

#endif


typedef struct {

	 char *TermCode;      //智能表的编号
	 char  TermType;      //智能表的类型
	 char *TermInfo;      //智能表的描述
	 char  TermPeriod;     //智能表的上报周期

	}term_list;

#if 1
typedef struct {
	 char *IEEE;
     char *Nwkaddr;
     term_list termtable[];
	}node_list;
#endif

#if 0
typedef struct {
		 char *IEEE;
	     char *Nwkaddr;
	     term_list termtable;
	}node_list;
#endif
//extern node_list node_table[];


extern int db_init();
extern void sqlite_insert_security_config_table(char *ieee, char *addr, subsecurityConfig_t *data, int icnt);
extern void sqlite_insert_energy_config_table(char *ieee, char *addr, term_list *data, int icnt);
extern void sqlite_updata_global_operator(char op);
extern char** sqlite_query_msg(int *row, int *col, char *sql);
extern int sqlite_query_global_operator();
extern void sqlite_free_query_result(char **data);
extern char** sqlite_query_ieee(char **data, int *row, int *col);
extern int sqlite_updata_msg(char *sql);
extern int sqlite_query_to_native();
extern void sqlite_delete_allmsg_from_etable();
#endif /* APPSQLITE_H_ */
