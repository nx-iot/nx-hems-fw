#ifndef INT_DATAQUEUE_H
#define INT_DATAQUEUE_H   

#include "timer.h"

#define INT_MAX_WAIT_QUEUE  10
#define INT_MAX_PKT_LEN     255
#define INT_MAX_RETRY       1
                     
struct INTQUEUE {                                
    char            retryCount;    
    TIMER           retryTime;
    int             timeout;          
    unsigned char   packetLen;     
    char            packetID; 
    char            *packetData;
};

extern struct INTQUEUE INT_waitQueue[INT_MAX_WAIT_QUEUE];

int INT_retransmit(void);
int INT_addWaitQueue(char id, char *pktptr, int len, int timeout);
int INT_removeWaitQueue(char id);
void INT_initWaitQueue(void);
char INT_findFreeSlotWaitQueue(void);
char INT_isIdExist(char id);
char INT_generateId(void);

#endif 