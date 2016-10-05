#include <mega128.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <delay.h>
#include "uart.h"
#include "debug.h"
#include "xbeehandler.h"
#include "xbeequeue.h"
#include "timer.h"

struct xbeequeue xbee_waitQueue[XBEE_MAX_WAIT_QUEUE];
char   xbee_waitQCount = 0;         


/*============================================================*/
int xbee_addWaitQueue(char id, char *pktptr, int len, int timeout) {

    int slot;
    
    slot = xbee_findFreeSlotWaitQueue();
    if(slot < 0) {                         
        printDebug("\r\n\r\n<XBEE_addWaitQueue> WARNING : Wait queue FULL\r\n\r\n");
        return -1;
    }                                                              
    
    xbee_waitQueue[slot].packetData = (char *) malloc(len);
    if(xbee_waitQueue[slot].packetData == NULL) {
        printDebug("<XBEE_addWaitQueue> malloc(%d) failed\r\n",len);
        return -1;
    }             
    
    memcpy( xbee_waitQueue[slot].packetData, pktptr, len ); 
    
    xbee_waitQueue[slot].packetLen   = len;
    xbee_waitQueue[slot].packetID    = id;    
    xbee_waitQueue[slot].retryCount  = 0;    
    xbee_waitQueue[slot].timeout     = timeout; 
                                           
    TIMER_setTimer(&(xbee_waitQueue[slot].retryTime), xbee_waitQueue[slot].timeout);
    
    xbee_waitQCount++;                              
    printDebug("                                     Added (%02X) to xbee_waitQueue[%d]\r\n", id, slot);         
    //print_payload(eth_waitQueue[slot].packetData,eth_waitQueue[slot].packetLen);
    return xbee_waitQCount;
    
}
/*============================================================*/
int xbee_removeWaitQueue(char id) {
 
    char i;                             
    for(i=0;i<XBEE_MAX_WAIT_QUEUE;i++) {
        if(id == xbee_waitQueue[i].packetID) {        // scan id in eth_waitQueue        
            //printDebug("                                 Removing (%02X) from eth_waitQueue\r\n", id);     
            //print_payload(eth_waitQueue[i].packetData,eth_waitQueue[i].packetLen);      
            //memset( eth_waitQueue[i].packetData, '\0', ETH_MAX_PKT_LEN );      
            free(xbee_waitQueue[i].packetData);                    
            xbee_waitQueue[i].packetData  = NULL;
            xbee_waitQueue[i].packetLen   = 0;
            xbee_waitQueue[i].packetID    = 0;    
            xbee_waitQueue[i].retryCount  = 0;    
            xbee_waitQueue[i].timeout     = 0;    
            xbee_waitQueue[i].retryTime   = 0;    
            xbee_waitQCount--;                                       
            //printDebug("Removed\r\n");                       
            printDebug("                                     Removed (%02X) to xbee_waitQueue[%d]\r\n", id, i);  
            return xbee_waitQCount;                  
        }
    }                 
    printDebug("\r\n\r\nWARNING : xbee_removeWaitQueue(%02X) not found!!\r\n\r\n",id);
    return -1;
    
}
/*============================================================*/
void xbee_initWaitQueue(void) {
 
    char i;      
                                  
    for(i=0;i<XBEE_MAX_WAIT_QUEUE;i++){           
        //memset( xbee_waitQueue[i].packetData, '\0', ETH_MAX_PKT_LEN );      
        free(xbee_waitQueue[i].packetData);  
        xbee_waitQueue[i].packetData  = NULL;
        xbee_waitQueue[i].packetLen   = 0;
        xbee_waitQueue[i].packetID    = 0;    
        xbee_waitQueue[i].retryCount  = 0;    
        xbee_waitQueue[i].timeout     = 0;    
        xbee_waitQueue[i].retryTime   = 0;    
    }                                  
    return;
                          
}             
/*============================================================*/
char xbee_findFreeSlotWaitQueue(void) {

    char i;
    for(i=0;i<XBEE_MAX_WAIT_QUEUE;i++){
        if(xbee_waitQueue[i].packetLen == 0){
            return i;    
        }
    }          
    return -1; 
    
}
/*============================================================*/
char xbee_isIdExist(char id) {

    char i;      
                                  
    for(i=0;i<XBEE_MAX_WAIT_QUEUE;i++){
        if(id == xbee_waitQueue[i].packetID){
            return 1;                           // found id in xbee_waitQueue
        }
    }          
    return 0;
    
}
/*============================================================*/
char xbee_generateId(void) { 
                 
    char id;
          
    do{
        id = (rand() % 256) & 0xFF;
    }while(xbee_isIdExist(id) || (id == 0) || (id == 0xFF));  
    
    return id;
    
} 
/*============================================================*/

