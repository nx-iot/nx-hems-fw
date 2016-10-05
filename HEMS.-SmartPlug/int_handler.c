#include <mega128a.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <delay.h>
#include "debug.h"
#include "uart.h"
#include "xbee.h"
#include "xbeehandler.h"
#include "int_handler.h"
#include "int_protocol.h"
#include "int_dataqueue.h"
#include "initial_system.h"

char _hb_res = 0;
char _hb_stat = 0xFF;
char _joinreq_res = 0;
char _joinreq_stat = 0xFF;
char _pired = 0;

/* ================================================================================= */ 
int8_t INT_processRMDCTRLC(char *buf, int len) {

    unsigned char cid;
    unsigned char value[4];
    
    cid = buf[10];
    memcpy(value, &buf[11], 4);
    
    if((cid&0xF0) == 0x20) {
        printDebug("    - Digital Output Drive : ");
        if(value[0] == 0x01) {
            SWITCH = TURN_ON;
            POWER_RELAY_ON;
            LED_STAT_ON;
            SAVE_DEVICE_STAT = TURN_ON;
            printDebug("ON.\r\n");    
        }else if(value[0] == 0x00) { 
            SWITCH = TURN_OFF;
            POWER_RELAY_OFF;
            LED_STAT_OFF;
            SAVE_DEVICE_STAT = TURN_OFF;
            CURRENT_VOLT = 0.0;
            CURRENT_AMP = 0.0;
            printDebug("OFF.\r\n");
        }
    }
     return 0;
     
}
/* ================================================================================= */ 
int8_t INT_processNETMAINA(char *buf, int len) {

    unsigned char cmd;
    unsigned char status;
    
    cmd = buf[2];
    status = buf[3];    

    switch(cmd) { 
       
        case HEARTBEAT :                                                             // -- 0x11 Heartbeat 
            printDebug("    - Heartbeat Status : 0x%02X\r\n", status);
            _hb_res = 1;
            _hb_stat = status;
            break;
        case JOINREQ :                                                               // -- 0x30 Join Request 
            printDebug("    - Join Request Status : 0x%02X\r\n", status);
            _joinreq_res = 1;
            _joinreq_stat = status;
            break;    
        
        default:                      
            printDebug("\r\nINT_processNETMAINA> Command not define yet ( %02X )!!\r\n", cmd);                
            return -1;
            break;
     }
     return 0;
     
}
/* ================================================================================= */ 
// Put INT_COMM_PROTOCOL Packet in Xbee Tx Packet and Send its.
int8_t INT_sendPacket(char *recvAddr, char *data, int dataLen, int timeout) {       
                                                 
    char coordinatorAddr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    char destNetAddr[] = {0xFF, 0xFE};
    char *sendPacket;
    int packetLen;
    char frameID;
    char frameLen;
    int res;
    int index = 0;
    
    // Start+Len+FrameType+FrameID+destAddr+destNetAddr+BroadcastRadius+Options+  ProtoVer+SendAddr+RecvAddr+  RFData+CheckSum   Byte.
    packetLen = 1+2+1+1+8+2+1+1+1+8+8+dataLen+1;    
    frameLen = packetLen - 4;
    frameID = data[0];
                          
    sendPacket  = (char *)malloc(packetLen);        
    if(sendPacket == NULL){                     
        printDebug("WARNING : INT_sendPacket malloc failed\r\n"); 
        return -1;
    } 
    
    sendPacket[index++] = 0x7E;                                                            // Start Delimeter
    sendPacket[index++] = (frameLen >> 8) & 0xFF;                                          // Xbee Packet Length  - msb
    sendPacket[index++] = frameLen & 0xFF;                                                 //                     - lsb
    sendPacket[index++] = 0x10;                                                            // Frame Type   - ZigBee Transmit Request
    sendPacket[index++] = frameID;                                                         // Frame ID
    memcpy(&sendPacket[index], coordinatorAddr, 8);                                        // 64 bit Destination Address
    index += 8;
    memcpy(&sendPacket[index], destNetAddr, 2);                                            // 16 bit Destination Address
    index += 2;
    sendPacket[index++] = 0x00;                                                            // BroadcastRadius
    sendPacket[index++] = 0x00;                                                            // Options
    /* ---------- Part of INT_COMM_PROTOCOL ---------- */
    sendPacket[index++] = PROTO_VERSION;                                                   // Protocal Version
    memcpy(&sendPacket[index], SENDER_ADDRESS, 8);                                         // Sender Address
    index += 8;
    memcpy(&sendPacket[index], recvAddr, 8);                                               // Receiver Address
    index += 8;
    memcpy(&sendPacket[index], data, dataLen);                                             // Data
    index += dataLen;
    /* ----------------------------------------------- */
    sendPacket[index] = xbee_checksum(&sendPacket[3], packetLen-3);                         // Checksum
     
    if(timeout > 0) {
        res = INT_addWaitQueue(frameID, sendPacket, packetLen, timeout);    
        if(res < 0) {               
            free(sendPacket); 
            return -1;
        }               
    }
    
    printDebug("\r\nPay ---\r\n");   
    print_payload(sendPacket, packetLen);  
    printDebug("-------\r\n");
    
    write_uart1(sendPacket, packetLen);       
    free(sendPacket);
    return 0;
    
}
/* ================================================================================= */ 

