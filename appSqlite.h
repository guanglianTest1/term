/*
 * appSqlite.h
 *
 *  Created on: Jan 5, 2015
 *      Author: yanly
 */

#ifndef APPSQLITE_H_
#define APPSQLITE_H_

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


extern int db_init();
extern void sqlite_insert_security_config_table(char *ieee, char *addr, subsecurityConfig_t *data, int icnt);
extern void sqlite_updata_global_operator(char op);
extern char** sqlite_query_msg(int *row, int *col, char *sql);
extern int sqlite_query_global_operator();
extern void sqlite_free_query_result(char **data);
extern char** sqlite_query_ieee(char **data, int *row, int *col);
#endif /* APPSQLITE_H_ */
