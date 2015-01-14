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

//#include "unp.h"
//#include "unp.h"
#include <stdio.h>
#include <stdlib.h>
//#include "cJSON.h"
//#include <curl/curl.h>
#include <curl/curl.h>
#include<unistd.h>
#include <string.h>
//#include "smartgateway.h"

#include"json.h"
#include"cJSON.h"
#include"net.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"

//char localIp[20]="192.168.1.112";

void http_ctrl_iasWarningDeviceOperation(char *ieee)
{
	int rc;
	char url_str[MAXBUF];
	char result_str[MAXBUF] = {0};

	sprintf(url_str,"http://%s/cgi-bin/rest/network/iasWarningDeviceOperation.cgi?"
			"ep=01&ieee=%s&param1=%s&param2=0&param3=0&operatortype=%s&callback=1234"
			"&encodemethod=NONE&sign=AAA",local_addr,ieee,WARNING_TIME,WARNING_TYPE);
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

void child_doit(uint8 *curl,uint8 curlen,uint8 *nodeid)
{
	uint16	rt,rc;
	char  url_str[MAXBUF], result_str[MAXBUF] = {0};
	uint8 i;

	uint8 buffer[512]={0};

	for (i=0;i<curlen;i++)
	sprintf(buffer+i*2,"%02X",curl[i]);
	//printf("[%s]\n",buffer);


    //sprintf(url_str,"http://%s/cgi-bin/rest/network/ZBSendRawMessageToComPort.cgi?ep=0A&nwk_addr=%s&data=%s&callback=1234&encodemethod=NONE&sign=AAA",GATEWAY_IPADDR,nodeid,buffer);
	sprintf(url_str,"http://%s/cgi-bin/rest/network/ZBSendRawMessageToComPort.cgi?ep=0A&nwk_addr=%s&data=%s&callback=1234&encodemethod=NONE&sign=AAA",local_addr,nodeid,buffer);//modify by yan150112
	//DBG_PRINT("url_str=%s\n",url_str);
	// global libcURL init	
	rc = curl_global_init( CURL_GLOBAL_ALL );
	if (rc != CURLE_OK) 
	{
		//printf("child curl_global_init error %ld\n", (uint32)getpid());
		printf("child curl_global_init error\n");
		exit (1);
	}	


	rt = child_perform_http_request(url_str,result_str);
	if(rt!=0)
	{
        printf("child curl_set error rt=%d\n",rt);
	}
	
	//DBG_PRINT("result_str=%s\n",result_str);
	
	curl_global_cleanup();

}

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
		DBG_PRINT("result=%s\n",result);
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
