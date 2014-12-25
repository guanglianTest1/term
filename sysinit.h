#ifndef sysinit_H_
#define sysinit_H_

#include <sys/msg.h>

typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;
typedef signed char int8;
typedef unsigned char uint8;



#define MAXBUF 1024

typedef struct {
	  long mtype;
	  char mtext[MAXBUF];
 }msgform;

//#define __ARM_BOARD
#define __DEBUG
//#define __HAS_MSG_THREAD
//#define __HAS_TEST_THREAD

#ifdef __DEBUG
#define DBG_PRINT(format, ...) printf(format, ## __VA_ARGS__)
#define DBG_STR(__STR__) printf("%s:%d:%s\n",__FUNCTION__,__LINE__,__STR__)
#else
#define DBG_PRINT(format,...)
#define DBG_STR(__STR__)
#endif


void sysInit() ;


#endif 
