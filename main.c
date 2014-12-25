
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>


#include"net.h"
#include"json.h"
#include"cJSON.h"
#include"timer.h"
#include"HttpModule.h"
#include"term.h"
#include"sysinit.h"

 pthread_t time_thread;
 pthread_t server_thread;
 pthread_t term_thread;
 pthread_t node_thread;
//static pthread_t adapter_thread;
 pthread_t client_thread;
 pthread_t client_type;



int main()
{
	
//this is test
	sysInit();
	
	ConnectClient();
   
	
	pthread_create(&client_thread,NULL,client_msg_thread,NULL);   //���������ͨ���߳�

    
    pthread_create(&client_type,NULL,client_type_thread,NULL);   //���ͻ�������
    
	
	pthread_create(&time_thread,NULL,time_t_thread,NULL);          //��ʱ���߳�
	

	pthread_create(&term_thread,NULL,term_msg_thread,NULL);       //���ͳ�����Ϣ�߳�
		
	
	pthread_create(&node_thread,NULL,node_msg_thread,NULL);       //���ܽڵ��豸��Ϣ�߳�

		 		
	pthread_create(&server_thread,NULL,server_msg_thread,NULL);   //���������ͨ���߳�
			


	while (1)
		{


		//DBG_PRINT("1111\n");
        }
	return (0);
}
	


