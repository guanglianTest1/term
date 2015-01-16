/*
 * appSqlite.c
 *数据库处理
 *  Created on: Jan 5, 2015
 *      Author: yanly
 */
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include "appSqlite.h"
#include "user_config.h"

sqlite3* node_db = NULL;


//int check_ieee_config(void *params,int n_column,char **column_value,char **column_name)
//{
//    int i;
//    //printf("column=%d\n",n_column);
//    for(i=0;i<n_column;i++){
//        //printf("\t%s",column_value[i]);
//    }
//    //printf("\n");
//    return 0;
//}

/*
 *insert security configuration in stable
 * */
void sqlite_insert_security_config_table(char *ieee, char *addr, subsecurityConfig_t *data, int icnt)
{
	int i;
	int ret;
	char sql[1024];
	char* errmsg;

	if(node_db == NULL)
	{
		if(db_init()<0)
		{
			return;
		}
	}
	//判断IEEE在不在表内,在表内删除表内数据
	sprintf(sql,"delete from stable WHERE ieee='%s'",ieee);
    ret = sqlite3_exec(node_db,sql,NULL,NULL,&errmsg);
    if(ret != SQLITE_OK){
        fprintf(stderr,"query SQL error: %s\n",errmsg);
    }
	//插入数据
    for(i=0;i<icnt;i++)
    {
    	sprintf(sql,"insert into STable values(NULL,'%s','%s',%d,%d,%d,'%s',%d)",ieee,addr,data[i].type,data[i].subtype,data[i].num,data[i].info,data[i].operator);
    	ret = sqlite3_exec(node_db,sql,NULL,NULL,&errmsg);
    	if(ret != SQLITE_OK)
    	{
    		printf("insert fail:%s\n",errmsg);
    	}

    }
    sqlite3_free(errmsg);
}
/*
 * updata global operator in stable
 * */
void sqlite_updata_global_operator(char op)
{
	int ret;
	char sql[512];
	char *errmsg;
	sprintf(sql,"delete from stable WHERE ieee='%s'",GLOBAL_OPERATOR_IN_IEEE_NAME);
    ret = sqlite3_exec(node_db,sql,NULL,NULL,&errmsg);
	sprintf(sql,"INSERT INTO STable VALUES (NULL,'%s',null,null,null,null,null,%d)",GLOBAL_OPERATOR_IN_IEEE_NAME,op);
	ret = sqlite3_exec(node_db,sql,NULL,NULL,&errmsg);
	if(ret != SQLITE_OK)
	{
		printf("insert fail:%s\n",errmsg);
	}
	sqlite3_free(errmsg);
}
/*
 * updata msg in node.db
 * */
int sqlite_updata_msg(char *sql)
{
	int ret;
	char *errmsg;
	if(node_db == NULL)
	{
		if(db_init()<0)
		{
			return NULL;
		}
	}

	ret = sqlite3_exec(node_db,sql,NULL,NULL,&errmsg);
	if(ret != SQLITE_OK)
	{
		printf("updata database fail:%s\n",errmsg);
	}
	sqlite3_free(errmsg);
	return 0;
}
/*
 * node.db initialization
 * */
int db_init()
{
    if (SQLITE_OK != sqlite3_open(DATABASE_PATH, &node_db)){//打开数据库
        printf("open db fail \n");
        sqlite3_close(node_db);
        return -1;
    }
    else
    	printf("open node.db \n");
    return 0;
}
/*
 * 查询配置表
 * */
char** sqlite_query_msg(int *row, int *col, char *sql)
{
	int ret;
	char *errmsg;
	char **dbresult;
	int j,i;
	int nrow,ncol,index;

	if(node_db == NULL)
	{
		if(db_init()<0)
		{
			return NULL;
		}
	}

    ret = sqlite3_get_table(node_db,sql,&dbresult,row,col,&errmsg);
    if(ret == SQLITE_OK)
    {
    	nrow = *row;ncol = *col;
        //printf("query %i records.\n",nrow);
        index=ncol;
#if 1
        for(i=0;i<nrow;i++)
        {
            for(j=0;j<ncol;j++)
            {
                //printf("%s",dbresult[index]);
                index++;
            }
            //printf("\n");
        }
#endif
    }
    else
    {
    	printf("query fail:%s\n",errmsg);
    	return NULL;

    }
    if((nrow ==0)||(ncol ==0))
    {
    	printf("database not message\n");
    	return NULL;
    }
    return dbresult;
    //sqlite3_free_table(data);
}
/********************************************************************************/
//char** sqlite_query_ieee(char **data, int *row, int *col)
//{
//	int ret;
//	char *errmsg;
//	char **dbresult;
//	int j,i,nrow,ncol,index;
//    ret = sqlite3_get_table(node_db,"SELECT  DISTINCT ieee,nwkaddr FROM stable where nwkaddr not null;",&dbresult,row,col,&errmsg);
//    if(ret == SQLITE_OK)
//    {
//    	nrow = *row;ncol = *col;
//        printf("query %i records.\n",nrow);
//        index=ncol;
//        for(i=0;i<nrow;i++)
//        {
//            for(j=0;j<ncol;j++)
//            {
//                printf("%s",dbresult[index]);
//                index++;
//            }
//            printf("\n");
//        }
//    }
//    else
//    {
//    	return -1;
//    	printf("query fail:%s\n",errmsg);
//    }
//    return dbresult;
//    //sqlite3_free_table(data);
//}
/*
 * query global operator configuration table
 * */
int sqlite_query_global_operator()
{
	int ret;
	int opt;
	char *errmsg;
	char **data;
	int j,i,nrow,ncol,index;
    ret = sqlite3_get_table(node_db,"select operator from stable where ieee = 'global operator'",&data,&nrow,&ncol,&errmsg);
    if(ret == SQLITE_OK)
    {
        //printf("query %i records.\n",nrow);
        index=ncol;
        for(i=0;i<nrow;i++)
        {
            for(j=0;j<ncol;j++)
            {
                //printf("%s",data[index]);
                index++;
            }
            //printf("\n");
        }
    }
    else
    {
    	return -1;
    	printf("query fail:%s\n",errmsg);
    }
    if(ncol ==0)
    {
    	return -1;
    }
    opt = atoi(data[ncol]);
    //printf("opt=%d\n",opt);
    sqlite3_free_table(data);
    return opt;
}
/*
 * release query result
 * */
void sqlite_free_query_result(char **data)
{
	sqlite3_free_table(data);
}
/*********************************************************************************************/


