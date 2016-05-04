#ifndef _HttpModule_H
#define _HttpModule_H
#include"sysinit.h"



#define CHILD_NUM 20
#define	MAX_RESPONSE	65536
#define MAX_RESULT 65436
#define MAX_URL_LEN 4096

int child_perform_http_request(char *url, char *result);
void child_doit(uint8 *curl,uint8 curlen,uint8 *nodeid);
int child_response(uint16 status, uint8 *result_str, uint8 *response_str);
size_t write_function( char *ptr, size_t size, size_t nmemb, void *userdata);

void http_ctrl_iasWarningDeviceOperation(char *ieee);//add yan150112
void http_get_localIASCIEOperation(int *status);	//add yan150512
void http_ctrl_start_alarm();//add yan150703

#endif

