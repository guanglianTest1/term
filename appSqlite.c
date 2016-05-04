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
#include"sysinit.h"
#include"json.h"
#include"sysinit.h"

//extern int Term_rcvPeriod;
sqlite3* node_db = NULL;

//node_list *node_table=NULL;
//extern int MsgTermNativeId;

node_native nodetable_native[NODE_NUMM];

uint8 Term_Num[NODE_NUMM]={0};

uint8 nodenum=0;
//extern uint16 term_num;
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
    		GDGL_DEBUG("insert fail:%s\n",errmsg);
    	}

    }
    sqlite3_free(errmsg);
}

void sqlite_delete_allmsg_from_etable()
{
	//int i;
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
		//sprintf(sql,"delete from etable WHERE ieee='%s'",ieee);
		sprintf(sql,"delete from etable ");
	    ret = sqlite3_exec(node_db,sql,NULL,NULL,&errmsg);
	    if(ret != SQLITE_OK){
	        fprintf(stderr,"query SQL error: %s\n",errmsg);
	    }
	    sqlite3_free(errmsg);

}

void sqlite_insert_energy_config_table(char *ieee, char *addr, term_list *data, int icnt)
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
	//sprintf(sql,"delete from etable WHERE ieee='%s'",ieee);
	//sprintf(sql,"delete from etable ");
    //ret = sqlite3_exec(node_db,sql,NULL,NULL,&errmsg);
   // if(ret != SQLITE_OK){
   //     fprintf(stderr,"query SQL error: %s\n",errmsg);
   // }
	//插入数据
    for(i=0;i<icnt;i++)
    {
    	sprintf(sql,"insert into ETable values(NULL,'%s','%s','%s',%d,'%s',%d)",ieee,addr,data[i].TermCode,data[i].TermType,data[i].TermInfo,data[i].TermPeriod);
    	ret = sqlite3_exec(node_db,sql,NULL,NULL,&errmsg);
    	if(ret != SQLITE_OK)
    	{
    		GDGL_DEBUG("insert fail:%s\n",errmsg);
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
		GDGL_DEBUG("insert fail:%s\n",errmsg);
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
			return 0;
		}
	}

	ret = sqlite3_exec(node_db,sql,NULL,NULL,&errmsg);
	if(ret != SQLITE_OK)
	{
		GDGL_DEBUG("updata database fail:%s\n",errmsg);
	}
	sqlite3_free(errmsg);
	return 1;
}
/*
 * node.db initialization
 * */
int db_init()
{
    if (SQLITE_OK != sqlite3_open(DATABASE_PATH, &node_db)){//打开数据库
    	GDGL_DEBUG("open db fail \n");
        sqlite3_close(node_db);
        exit(1);  //打开数据库失败，退出
        return -1;
    }
//    else
//    	printf("open node.db \n");
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
    	GDGL_DEBUG("query fail:%s\n",errmsg);
    	return NULL;

    }
    if((nrow ==0)||(ncol ==0))
    {
    	GDGL_DEBUG("sql is[%s], but database not message\n", sql);
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
    	GDGL_DEBUG("query fail:%s\n",errmsg);
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


#if 1
int sqlite_query_to_native()
{
	    char **q_data=NULL;
		char **ieee=NULL;int ieee_row,ieee_col;
		int q_row,q_col;int i,j,k=0,num=0;
		int index_ieee,index_q;
		char *sql_req_config = "select ieee,nwkaddr,TermCode,Termtype,TermInfo,TermPeriod from etable where nwkaddr not null";
		char *sql_req_ieee = "SELECT  DISTINCT ieee,nwkaddr FROM etable where nwkaddr not null";
		//uint8 k=0;

	    //printf("sqlite_query_to_native\n");
	    q_data = sqlite_query_msg(&q_row, &q_col, sql_req_config);
	    //printf("q_row=%d\n",q_row);
	    if(q_data ==NULL)
	    {
//	    	printf("query db error\n");
	    	sqlite_free_query_result(q_data);
	    	return JSON_VALUE_ERROR;
	    }
	    ieee = sqlite_query_msg(&ieee_row, &ieee_col,sql_req_ieee);
	    if(ieee == NULL)
	    {
//	    	printf("query db error\n");
	    	sqlite_free_query_result(ieee);
	    	return JSON_VALUE_ERROR;
	    }

    nodenum=ieee_row;
    //printf("nodenum=%d\n",nodenum);
    //nodetable_native = (node_native *)malloc(nodenum*sizeof(node_native));//malloc
    memset(nodetable_native,0,sizeof(nodetable_native));
	for(i=0;i<ieee_row;i++)
	{
		index_ieee = i*ieee_col+ieee_col;//ieee data的每行头
		//nodetable_native[i].IEEE_native=ieee[index_ieee];
		//nodetable_native[i].Nwkaddr_native=ieee[index_ieee+1];
		memcpy(nodetable_native[i].IEEE_native,ieee[index_ieee],strlen(ieee[index_ieee]));
		memcpy(nodetable_native[i].Nwkaddr_native,ieee[index_ieee+1],strlen(ieee[index_ieee+1]));
		//printf("IEEE_native[%d]=%s\n",i,nodetable_native[i].IEEE_native);
	    //printf("Nwkaddr_native[%d]=%s\n",i,nodetable_native[i].Nwkaddr_native);

		for(j=0;j<q_row;j++)
		{    //printf("j=%d\n",j);
			index_q = j*q_col + q_col;//data的每行头
			if(strcmp(q_data[index_q], ieee[index_ieee])==0)
			{

				memcpy(nodetable_native[i].termtable_native[num].TermCode_native,q_data[index_q+2],strlen(q_data[index_q+2]));
				//nodetable_native[i].termtable_native[j].Termcode_native=q_data[index_q+2];
				nodetable_native[i].termtable_native[num].TermType_native=atoi(q_data[index_q+3]);
				//nodetable_native[i].termtable_native[j].TermType_native=q_data[index_q+3];
				memcpy(nodetable_native[i].termtable_native[num].TermInfo_native,q_data[index_q+4],strlen(q_data[index_q+4]));
				nodetable_native[i].termtable_native[num].TermPeriod_native=atoi(q_data[index_q+5]);

				k++;
				//printf("TermCode[%d][%d]=%s\n",i,num,nodetable_native[i].termtable_native[num].TermCode_native);
                num++;
			}
			else
			num=0;
		}
		Term_Num[i]=k;
		k=0;
		//printf("termnum[%d]=%d\n",i,Term_Num[i]);


	}
#if 0
	if(nodetable_native[0].termtable_native[0].TermPeriod_native>0)
	 {
		Term_rcvPeriod=60*nodetable_native[0].termtable_native[0].TermPeriod_native;
	    printf("Term_rcvPeriod=%d\n",Term_rcvPeriod);
	 }
   else
	 {
	   Term_rcvPeriod=TERM_PEIOD;
	   printf("Term_rcvPeriod=%d\n",Term_rcvPeriod);
	 }
	ModTimer(Term_rcvPeriod,time2);
#endif
	sqlite_free_query_result(q_data);
	sqlite_free_query_result(ieee);


	return 0;
}
#endif

#if 0

int sqlite_query_to_native()
{
	    char **q_data=NULL;
		char **ieee=NULL;int ieee_row,ieee_col;
		int q_row,q_col;int i,j;
		int index_ieee,index_q;
		char *sql_req_config = "select ieee,nwkaddr,TermCode,Termtype,TermInfo,TermPeriod from etable where nwkaddr not null";
		char *sql_req_ieee = "SELECT  DISTINCT ieee,nwkaddr FROM etable where nwkaddr not null";
		uint8 k=0;
		msgform msg_nativeterm;

	    printf("sqlite_query_to_native\n");

	    q_data = sqlite_query_msg(&q_row, &q_col, sql_req_config);
	    printf("q_row=%d\n",q_row);
	    if(q_data ==NULL)
	    {
	    	printf("query db error\n");
	    	sqlite_free_query_result(q_data);
	    	return JSON_VALUE_ERROR;
	    }
	    ieee = sqlite_query_msg(&ieee_row, &ieee_col,sql_req_ieee);
	    if(ieee == NULL)
	    {
	    	printf("query db error\n");
	    	sqlite_free_query_result(ieee);
	    	return JSON_VALUE_ERROR;
	    }

    nodenum=ieee_row;
    printf("nodenum=%d\n",nodenum);
    node_table = (node_list *)malloc(nodenum*sizeof(node_list));//malloc
    memset(node_table,0,sizeof(node_table));
	for(i=0;i<ieee_row;i++)
	{
		index_ieee = i*ieee_col+ieee_col;//ieee data的每行头
		node_table[i].IEEE=ieee[index_ieee];
		node_table[i].Nwkaddr=ieee[index_ieee+1];

		printf("IEEE_native=%s\n",node_table[i].IEEE);
		printf("Nwkaddr_native=%s\n",node_table[i].Nwkaddr);

		for(j=0;j<q_row;j++)
		{
			index_q = j*q_col + q_col;//data的每行头
			if(strcmp(q_data[index_q], ieee[index_ieee])==0)
			{
				//memcpy(nodetable_native[i].termtable_native[j].Termcode_native,q_data[index_q+2],strlen(q_data[index_q+2]));
				node_table[i].termtable[j].TermCode=q_data[index_q+2];
				printf("TermCode=%s\n",node_table[i].termtable[j].TermCode);
				node_table[i].termtable[j].TermType=atoi(q_data[index_q+3]);
				node_table[i].termtable[j].TermInfo=q_data[index_q+4];
				//memcpy(nodetable_native[i].termtable_native[j].TermInfo_native,q_data[index_q+4],strlen(q_data[index_q+4]));
				node_table[i].termtable[j].TermPeriod=atoi(q_data[index_q+5]);
				k++;

			}

		}
		Term_Num[i]=k;

		printf("k=%d\n",k);
		printf("termnum[%d]=%d\n",i,Term_Num[i]);
		k=0;

	}
	memcpy(msg_nativeterm.mtext,node_table,sizeof(node_list));

    printf("mtext=%s\n",msg_nativeterm.mtext);
    msgsnd(MsgTermNativeId,&msg_nativeterm,sizeof(msgform),0);

	sqlite_free_query_result(q_data);
	sqlite_free_query_result(ieee);


	return 0;
}

#endif


/*********************************************************************************************/


