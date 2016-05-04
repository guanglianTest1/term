/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: cloudProxyHttpModule.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/03/17
  Description: cloud proxy http module
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/03/17     1.0     build this moudle  
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include<unistd.h>
#include <string.h>

#include"json.h"
#include"cJSON.h"
#include"net.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"

/***************************************************************************
  Function: get_json_str
  Description: get substring between first '{' and last '}'
  Input:  raw_str
  Output: json_str
  Return: 0:OK / -1:Fail
  Others: none
***************************************************************************/
static int get_json_str(char* json_str, char* raw_str)
{
    char* s_begin;
    char* s_end;
    int len;
    s_begin = strchr(raw_str, '{');
    s_end = strrchr(raw_str, '}');

    if (!s_begin || !s_end)
    {
        return -1;
    }

    len = s_end - s_begin + 1;
	if (len <= 0)
	{
		return -1;
	}
    strncpy(json_str, s_begin, len);
    json_str[len] = '\0';
    return 0;
}
void http_ctrl_iasWarningDeviceOperation(char *ieee)
{
	int rc;
	char url_str[MAXBUF];
	char result_str[MAXBUF] = {0};

	sprintf(url_str,"http://127.0.0.1/cgi-bin/rest/network/iasWarningDeviceOperation.cgi?"
			"ep=01&ieee=%s&param1=%s&param2=0&param3=0&operatortype=%s&callback=1234"
			"&encodemethod=NONE&sign=AAA", ieee,WARNING_TIME,WARNING_TYPE);
//	printf("bao jing :%s\n",url_str);
//	exit(1);
	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK)
	{
		//printf("child curl_global_init error %ld\n", (uint32)getpid());
		printf("child curl_global_init error\n");
		exit(1);
	}


	rc = child_perform_http_request(url_str,result_str);
	if(rc!=0)
	{
        printf("child curl_set error rt=%d\n",rc);
	}
	curl_global_cleanup();
}
const char getzbnode[]= "http://127.0.0.1/cgi-bin/rest/network/getZBNode.cgi?callback=1234&encodemethod=NONE&sign=AAA";
#if 1   //采用多进程方式执行  by yanly
void http_ctrl_start_alarm()
{
	pid_t	pid;
	if ((pid = fork()) > 0) {
		waitpid(-1,NULL,0);
		return;				//parent process
	}
	if ((pid = fork()) > 0) {
		exit(0);					//child process
		return;
	}
									//grandson process
	int res=0;
	int rt;
	CURLcode rc;
	char		result_str[MAX_RESULT] = {0};
	char		response_json_str[MAX_RESULT] = {0};
	cJSON 		*response_json;

	//开始zigbee报警器报警:
	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK) {
		printf("curl_global_init error\n");
		res = -1;
		exit(1);
	}

	rt = child_perform_http_request(getzbnode, result_str);
	if(rt <0) {
		printf("invoke api failed\n");
		res =-1;
		exit(1);
	}
	get_json_str(response_json_str, result_str);
	response_json = cJSON_Parse(response_json_str);
	if (!response_json)
	{
		printf("json parse Error before: (%s)\n", cJSON_GetErrorPtr());
		res= -2;
		exit(1);
	}
    cJSON* json_array;
    cJSON* json_node;
    cJSON* json_tmp;

    json_array = cJSON_GetObjectItem(response_json, "response_params");
    if (!json_array)
    {
        cJSON_Delete(response_json);
		res= -2;
		exit(1);
    }
    int array_size = cJSON_GetArraySize(json_array);
    if (array_size == 0)
    {
        cJSON_Delete(response_json);
		res= -2;
		exit(1);
    }

    char* ieee;
    char* modeid;
    char request[200] = {0};
    int i = 0;
    for (i = 0; i < array_size; ++i)
    {
        // 对于其中某个设备操作失败直接忽略
        json_tmp = cJSON_GetArrayItem(json_array, i);
        if (!json_tmp)
        {
            continue;
        }
        json_node = cJSON_GetObjectItem(json_tmp, "node");
        if (!json_node)
        {
            continue;
        }
        json_tmp = cJSON_GetObjectItem(json_node, "model_id");
        if (!json_tmp)
        {
            continue;
        }
        modeid = json_tmp->valuestring;
        if(memcmp(modeid, "Z602A", 5) !=0)
        {
        	continue;
        }
        json_tmp = cJSON_GetObjectItem(json_node, "ieee");
        if (!json_tmp)
        {
            continue;
        }
        ieee = json_tmp->valuestring;
        //发送报警api
        snprintf(request,sizeof(request),"http://127.0.0.1/cgi-bin/rest/network/iasWarningDeviceOperation.cgi?"
    			"ep=01&ieee=%s&param1=%s&param2=0&param3=0&operatortype=%s&callback=1234"
    			"&encodemethod=NONE&sign=AAA", ieee,WARNING_TIME,WARNING_TYPE);
//        snprintf(request, sizeof(request), "http://127.0.0.1/cgi-bin/rest/network/iasWarningDeviceOperation.cgi?ep=01&ieee=%s&param1=0&param2=0&param3=0&operatortype=0&callback=1234&encodemethod=NONE&sign=AAA", ieee);
    	rt = child_perform_http_request(request, result_str);
    	if(rt <0) {
    		printf("invoke api failed\n");
    	}
    }
	cJSON_Delete(response_json);
	curl_global_cleanup();
	exit(0);
}
#else
void http_ctrl_start_alarm()
{
	int res=0;
	int rt;
	CURLcode rc;
	char		result_str[MAX_RESULT] = {0};
	char		response_json_str[MAX_RESULT] = {0};
	cJSON 		*response_json;

	//开始zigbee报警器报警:
	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK) {
		printf("curl_global_init error\n");
		res = -1;
		exit(1);
	}

	rt = child_perform_http_request(getzbnode, result_str);
	if(rt <0) {
		printf("invoke api failed\n");
		res =-1;
		exit(1);
	}
	get_json_str(response_json_str, result_str);
	response_json = cJSON_Parse(response_json_str);
	if (!response_json)
	{
		printf("json parse Error before: (%s)\n", cJSON_GetErrorPtr());
		res= -2;
		exit(1);
	}
    cJSON* json_array;
    cJSON* json_node;
    cJSON* json_tmp;

    json_array = cJSON_GetObjectItem(response_json, "response_params");
    if (!json_array)
    {
        cJSON_Delete(response_json);
		res= -2;
		exit(1);
    }
    int array_size = cJSON_GetArraySize(json_array);
    if (array_size == 0)
    {
        cJSON_Delete(response_json);
		res= -2;
		exit(1);
    }

    char* ieee;
    char* modeid;
    char request[200] = {0};
    int i = 0;
    for (i = 0; i < array_size; ++i)
    {
        // 对于其中某个设备操作失败直接忽略
        json_tmp = cJSON_GetArrayItem(json_array, i);
        if (!json_tmp)
        {
            continue;
        }
        json_node = cJSON_GetObjectItem(json_tmp, "node");
        if (!json_node)
        {
            continue;
        }
        json_tmp = cJSON_GetObjectItem(json_node, "model_id");
        if (!json_tmp)
        {
            continue;
        }
        modeid = json_tmp->valuestring;
        if(memcmp(modeid, "Z602A", 5) !=0)
        {
        	continue;
        }
        json_tmp = cJSON_GetObjectItem(json_node, "ieee");
        if (!json_tmp)
        {
            continue;
        }
        ieee = json_tmp->valuestring;
        //发送报警api
        snprintf(request,sizeof(request),"http://127.0.0.1/cgi-bin/rest/network/iasWarningDeviceOperation.cgi?"
    			"ep=01&ieee=%s&param1=%s&param2=0&param3=0&operatortype=%s&callback=1234"
    			"&encodemethod=NONE&sign=AAA", ieee,WARNING_TIME,WARNING_TYPE);
//        snprintf(request, sizeof(request), "http://127.0.0.1/cgi-bin/rest/network/iasWarningDeviceOperation.cgi?ep=01&ieee=%s&param1=0&param2=0&param3=0&operatortype=0&callback=1234&encodemethod=NONE&sign=AAA", ieee);
    	rt = child_perform_http_request(request, result_str);
    	if(rt <0) {
    		printf("invoke api failed\n");
    	}
    }
	cJSON_Delete(response_json);
	curl_global_cleanup();
}
#endif
/*
 * 获取全局布撤防状态
 * */
void http_get_localIASCIEOperation(int *status)
{
	int rc;
	char url_str[MAXBUF];
	char result_str[MAXBUF] = {0};
	char response_json_str[MAXBUF];
	cJSON *response_json, *json_temp, *json_opt;
	int opt;

	sprintf(url_str,"http://127.0.0.1/cgi-bin/rest/network/localIASCIEOperation.cgi?"
			"operatortype=5&param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA");

	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK)
	{
		GDGL_DEBUG("child curl_global_init error\n");
		exit(1);
	}
	rc = child_perform_http_request(url_str,result_str);
	if(rc!=0)
	{
		GDGL_DEBUG("child curl_set error rt=%d\n",rc);
		exit(1);
	}
	//赋值全局布撤防状态
	get_json_str(response_json_str, result_str);
	GDGL_DEBUG("get global opertor respond:%s\n", response_json_str);
	response_json = cJSON_Parse(response_json_str);
	if (!response_json)
	{
		GDGL_DEBUG("http_get_localIASCIEOperation: parse failed\n");
	}
	else
	{
		json_temp = cJSON_GetObjectItem(response_json, "response_params");
        if(json_temp)
		{
        	json_opt = cJSON_GetObjectItem(json_temp, "param1");
        	if(json_opt)
        	{
        		opt = json_opt->valueint;
        		if(opt ==0)					//撤防
        		{
        			*status = 2;
        		}
        		else if(opt ==3)			//布防
        		{
        			*status = 1;
        		}
        		else
        		{
        			*status = 2;
        		}
        	}
        	else
        	{
            	GDGL_DEBUG("json string not param1\n");
            	cJSON_Delete(response_json);
            	curl_global_cleanup();
            	exit(1);
        	}
		}
        else
        {
        	GDGL_DEBUG("json string not response_params\n");
        	cJSON_Delete(response_json);
        	curl_global_cleanup();
        	exit(1);
        }
	}
	cJSON_Delete(response_json);
	curl_global_cleanup();
}
#if 1
//采用多进程方式执行 ZBSendRawMessageToComPort.cgi 命令，不等待api的返回结果，节省时间。//add by yanly
void child_doit(uint8 *curl,uint8 curlen,uint8 *nodeid)
{
	pid_t	pid;
	if ((pid = fork()) > 0) {
		waitpid(-1,NULL,0);
		return;				//parent process
	}
	if ((pid = fork()) > 0) {
		exit(0);					//child process
		return;
	}
									//grandson process

	uint16	rt,rc;
	char  url_str[MAXBUF], result_str[MAXBUF] = {0};
	uint8 i;
	uint8 buffer[MAXBUF]={0};

	for (i=0;i<curlen;i++)
		sprintf(buffer+i*2,"%02X",curl[i]);
	//printf("[%s]\n",buffer);


    //sprintf(url_str,"http://%s/cgi-bin/rest/network/ZBSendRawMessageToComPort.cgi?ep=0A&nwk_addr=%s&data=%s&callback=1234&encodemethod=NONE&sign=AAA",GATEWAY_IPADDR,nodeid,buffer);//
	sprintf(url_str,"http://127.0.0.1/cgi-bin/rest/network/ZBSendRawMessageToComPort.cgi?ep=0A&nwk_addr=%s&data=%s&callback=1234&encodemethod=NONE&sign=AAA", nodeid, buffer);//modify by yan150112
	DBG_PRINT("url_str=%s\n",url_str);                                                                                                             //ALL_IDAWY 也可以
	// global libcURL init	
	rc = curl_global_init( CURL_GLOBAL_ALL );
	if (rc != CURLE_OK) 
	{
		//printf("child curl_global_init error %ld\n", (uint32)getpid());
		GDGL_DEBUG("child curl_global_init error\n");
		exit (1);
	}	


	rt = child_perform_http_request(url_str,result_str);
	if(rt!=0)
	{
		GDGL_DEBUG("child curl_set error rt=%d\n",rt);
	}
	
	//DBG_PRINT("result_str=%s\n",result_str);
	
	curl_global_cleanup();
	exit(0);   //执行完成后退出
}
#else
void child_doit(uint8 *curl,uint8 curlen,uint8 *nodeid)
{
	uint16	rt,rc;
	char  url_str[MAXBUF], result_str[MAXBUF] = {0};
	uint8 i;

	uint8 buffer[MAXBUF]={0};

	for (i=0;i<curlen;i++)
	sprintf(buffer+i*2,"%02X",curl[i]);
	//printf("[%s]\n",buffer);


    //sprintf(url_str,"http://%s/cgi-bin/rest/network/ZBSendRawMessageToComPort.cgi?ep=0A&nwk_addr=%s&data=%s&callback=1234&encodemethod=NONE&sign=AAA",GATEWAY_IPADDR,nodeid,buffer);//
	sprintf(url_str,"http://127.0.0.1/cgi-bin/rest/network/ZBSendRawMessageToComPort.cgi?ep=0A&nwk_addr=%s&data=%s&callback=1234&encodemethod=NONE&sign=AAA", nodeid, buffer);//modify by yan150112
	DBG_PRINT("url_str=%s\n",url_str);                                                                                                             //ALL_IDAWY 也可以
	// global libcURL init
	rc = curl_global_init( CURL_GLOBAL_ALL );
	if (rc != CURLE_OK)
	{
		//printf("child curl_global_init error %ld\n", (uint32)getpid());
		GDGL_DEBUG("child curl_global_init error\n");
		exit (1);
	}


	rt = child_perform_http_request(url_str,result_str);
	if(rt!=0)
	{
		GDGL_DEBUG("child curl_set error rt=%d\n",rt);
	}

	//DBG_PRINT("result_str=%s\n",result_str);

	curl_global_cleanup();

}
#endif
int child_perform_http_request(char *url, char *result)
{
    
	    CURL *ctx;
		CURLcode rc;
		pid_t c_pid;

		c_pid = getpid();

		// create a context, sometimes known as a handle.
		ctx = curl_easy_init();
		if (NULL == ctx) {
			DBG_PRINT("child %ld curl_easy_init Error\n", (long)c_pid);
			return 1;
		}

		//curl_easy_setopt( ctx , CURLOPT_VERBOSE, 1 );

		// target url:
		rc = curl_easy_setopt( ctx , CURLOPT_URL,  url );
		if (CURLE_OK != rc) {
			DBG_PRINT("child %ld curl_easy_setopt CURLOPT_URL Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
			curl_easy_cleanup( ctx );
			return 2;
		}

		// no progress bar:
		rc = curl_easy_setopt( ctx , CURLOPT_NOPROGRESS , 1 );
		if (CURLE_OK != rc) {
			DBG_PRINT("child %ld curl_easy_setopt CURLOPT_NOPROGRESS Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
			curl_easy_cleanup( ctx );
			return 3;
		}

		// include the header in the body output:
		rc = curl_easy_setopt( ctx , CURLOPT_HEADER,  1 );
		if (CURLE_OK != rc) {
			DBG_PRINT("child %ld curl_easy_setopt CURLOPT_HEADER Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
			curl_easy_cleanup( ctx );
			return 4;
		}

		rc = curl_easy_setopt( ctx , CURLOPT_PROTOCOLS,  CURLPROTO_HTTP );
		if (CURLE_OK != rc) {
			DBG_PRINT("child %ld curl_easy_setopt CURLOPT_PROTOCOLS Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
			curl_easy_cleanup( ctx );
			return 5;
		}

	    rc = curl_easy_setopt( ctx , CURLOPT_WRITEFUNCTION , write_function );
		if (CURLE_OK != rc) {
			DBG_PRINT("child %ld curl_easy_setopt CURLOPT_WRITEFUNCTION Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
			curl_easy_cleanup( ctx );
			return 7;
		}

		rc = curl_easy_setopt( ctx , CURLOPT_WRITEDATA , (void *)result );
		if (CURLE_OK != rc) {
			DBG_PRINT("child %ld curl_easy_setopt CURLOPT_WRITEDATA Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
			curl_easy_cleanup( ctx );
			return 8;
		}

		rc = curl_easy_perform( ctx );
		if (CURLE_OK != rc) {
			DBG_PRINT("child %ld curl_easy_perform Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
			curl_easy_cleanup( ctx );
			return 9;
		}
		//DBG_PRINT("result=%s\n",result);//modify yan
		curl_easy_cleanup( ctx );
		return 0;
}


size_t write_function( char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t write_num;
	unsigned int len;

    write_num = size * nmemb;
	len = strlen(userdata);
	if ( (write_num + len) >= MAX_RESULT)
		return 0;

	memcpy(userdata + len, ptr, write_num);
	*((char *)userdata + len + write_num) = '\0';
	
	return write_num;
}


#if 0

int child_response(uint16 status, uint8 * result_str, uint8 * response_str)
{
    uint8 *out;
	//cJSON *response_json;
	uint16 re;
	pid_t c_pid;

	c_pid = getpid();

	if ( (status > 0) && (!result_str) ) {
		printf("child %ld status OK but result_str is NULL\n", (uint32)c_pid);
		return -1;
	}

	response_json = cJSON_CreateObject();
	if (!response_json) {
		printf("child %ld Create response_json failed\n", (uint32)c_pid);
		return -2;
	}
	cJSON_AddNumberToObject(response_json, "status", status);
	if (status > 0) {
		cJSON_AddStringToObject(response_json, "result", result_str);
	}

	out = cJSON_PrintUnformatted(response_json);
	cJSON_Delete(response_json);
	if (!out) {
		printf("child %ld cJSON_PrintUnformatted response_json failed\n", (uint32)c_pid);
		return -3;
	}
	re = snprintf(response_str, MAX_RESPONSE, "%s", out);
	free(out);

	return (re+1); //including null
}
#endif
