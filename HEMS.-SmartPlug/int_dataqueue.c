#include <mega128a.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <delay.h>
#include "uart.h"
#include "debug.h"
#include "int_dataqueue.h"
#include "timer.h"

struct INTQUEUE INT_waitQueue[INT_MAX_WAIT_QUEUE];
char   INT_waitQCount = 0;         
/* ================================================================================= */ 
int INT_retransmit(void) {

    char i;
    for(i = 0; i < INT_MAX_WAIT_QUEUE; i++) {
        if(TIMER_checkTimerExceed(INT_waitQueue[i].retryTime) && (INT_waitQueue[i].packetID != 0)) {  
            if(INT_waitQueue[i].retryCount >= INT_MAX_RETRY) {
                printDebug("                                     REJECT PACKET FRAME ID[%02X]\r\n", INT_waitQueue[i].packetID);
                INT_removeWaitQueue(INT_waitQueue[i].packetID);
            }else{                                                       
                INT_waitQueue[i].retryCount++;      // increase retry counter 
                TIMER_setTimer(&(INT_waitQueue[i].retryTime), INT_waitQueue[i].timeout);     
                printDebug("                                     RETRANSMIT(%02X) %d times\r\n", INT_waitQueue[i].packetID, INT_waitQueue[i].retryCount);                          
                //print_payload(waitQueue[i].packetData,waitQueue[i].packetLen);                 
                //printDebug("XBEE_RESEND> Retransmit(%d) ID[%02X]\r\n",xbee_waitQueue[i].retryCount,xbee_waitQueue[i].packetID);
                write_uart1(INT_waitQueue[i].packetData, INT_waitQueue[i].packetLen);   
            }  
        }
    }        
    return 0;      
} 
/* ================================================================================= */ 
int INT_addWaitQueue(char id, char *pktptr, int len, int timeout) {

    int slot;
    
    slot = INT_findFreeSlotWaitQueue();
    if(slot < 0) {                         
        printDebug("\r\nWARNING : <INT_addWaitQueue> Wait queue FULL\r\n\r\n");
        return -1;
    }                                                              
    
    INT_waitQueue[slot].packetData = (char *) malloc(len);
    if(INT_waitQueue[slot].packetData == NULL) {
        printDebug("\r\nWARNING : <INT_addWaitQueue> malloc(%d) failed\r\n\r\n", len);
        return -1;
    }             
    
    memcpy(INT_waitQueue[slot].packetData, pktptr, len); 
    
    INT_waitQueue[slot].packetLen   = len;
    INT_waitQueue[slot].packetID    = id;    
    INT_waitQueue[slot].retryCount  = 0;    
    INT_waitQueue[slot].timeout     = timeout; 
                                           
    TIMER_setTimer(&(INT_waitQueue[slot].retryTime), INT_waitQueue[slot].timeout);
    
    INT_waitQCount++;                              
    printDebug("                                     Added (%02X) to INT_waitQueue[%d]\r\n", id, slot);         
    print_payload(INT_waitQueue[slot].packetData, INT_waitQueue[slot].packetLen);
    return INT_waitQCount;
    
}
/* ================================================================================= */ 
int INT_removeWaitQueue(char id) {
 
    char i;                             
    for(i = 0; i < INT_MAX_WAIT_QUEUE; i++) {
        if(id == INT_waitQueue[i].packetID) {        // scan id in INT_waitQueue    
            print_payload(INT_waitQueue[i].packetData, INT_waitQueue[i].packetLen);      
            //memset( INT_waitQueue[i].packetData, '\0', INT_MAX_PKT_LEN );      
            free(INT_waitQueue[i].packetData);                    
            INT_waitQueue[i].packetData  = NULL;
            INT_waitQueue[i].packetLen   = 0;
            INT_waitQueue[i].packetID    = 0;    
            INT_waitQueue[i].retryCount  = 0;    
            INT_waitQueue[i].timeout     = 0;    
            INT_waitQueue[i].retryTime   = 0;    
            INT_waitQCount--; 
                                                                         
            printDebug("                                     Removed (%02X) INT_waitQueue[%d]\r\n", id, i);  
            return INT_waitQCount;                  
        }
    }                 
    printDebug("\r\nWARNING : INT_removeWaitQueue(%02X) not found!!\r\n\r\n",id);
    return -1;
    
}
/* ================================================================================= */ 
void INT_initWaitQueue(void) {
 
    char i;      
                                  
    for(i = 0; i < INT_MAX_WAIT_QUEUE; i++) {           
        //memset( INT_waitQueue[i].packetData, '\0', INT_MAX_PKT_LEN );      
        free(INT_waitQueue[i].packetData);  
        INT_waitQueue[i].packetData  = NULL;
        INT_waitQueue[i].packetLen   = 0;
        INT_waitQueue[i].packetID    = 0;    
        INT_waitQueue[i].retryCount  = 0;    
        INT_waitQueue[i].timeout     = 0;    
        INT_waitQueue[i].retryTime   = 0;    
    }                                  
    return;
                          
}             
/* ================================================================================= */ 
char INT_findFreeSlotWaitQueue(void) {

    char i;
    for(i = 0; i < INT_MAX_WAIT_QUEUE; i++) {
        if(INT_waitQueue[i].packetLen == 0) {
            return i;    
        }
    }          
    return -1; 
    
}
/* ================================================================================= */ 
char INT_isIdExist(char id) {

    char i;                                    
    for(i = 0; i < INT_MAX_WAIT_QUEUE; i++) {
        if(id == INT_waitQueue[i].packetID) {
            return 1;                           // found id in INT_waitQueue
        }
    }          
    return 0;
    
}
/* ================================================================================= */ 
char INT_generateId(void) { 
                 
    char id;      
    do{
        id = (rand() % 256) & 0xFF;
    }while(INT_isIdExist(id) || (id == 0) || (id == 0xFF));  
    return id;
    
} 
/* ================================================================================= */ 

