#include "xbee.h"
#include "print_debug.h"
#include "uart2_manag.h"
#include "xbeehandler.h"
#include "handler.h"
#include "protocol.h"
char _hb_res = 0;
char _hb_stat = 0xFF;
char _joinreq_res = 0;
char _joinreq_stat = 0xFF;
char _pired = 0;


/* ================================================================================= */ 
//int8_t INT_processRMDCTRLC(char *buf, int len) {

//    unsigned char cid;
//    unsigned char value[4];
//    
//    cid = buf[10];
//    memcpy(value, &buf[11], 4);
//    
//    if((cid&0xF0) == 0x20) {
//        printDebug("    - Digital Output Drive : ");
//        if(value[0] == 0x01) {
//            SWITCH = TURN_ON;
//            POWER_RELAY_ON;
//            LED_STAT_ON;
//            SAVE_DEVICE_STAT = TURN_ON;
//            printDebug("ON.\r\n");    
//        }else if(value[0] == 0x00) { 
//            SWITCH = TURN_OFF;
//            POWER_RELAY_OFF;
//            LED_STAT_OFF;
//            SAVE_DEVICE_STAT = TURN_OFF;
//            CURRENT_VOLT = 0.0;
//            CURRENT_AMP = 0.0;
//            printDebug("OFF.\r\n");
//        }
//    }
//     return 0;
//     
//}
///* ================================================================================= */ 
int8_t INT_processNETMAINA(char *buf, int len) {

    unsigned char cmd;
    unsigned char status;
    
    cmd = buf[2];
    status = buf[3];    

    switch(cmd) { 
         
				case HEARTBEAT :                                                             // -- 0x11 Heartbeat 
            print_debug(0,"    - Heartbeat Status : 0x%02X\r\n", status);
            _hb_res = 1;
            _hb_stat = status;
            break;
        case JOINREQ :                                                               // -- 0x30 Join Request 
            print_debug(0,"    - Join Request Status : 0x%02X\r\n", status);
            _joinreq_res = 1;
            _joinreq_stat = status;
            break;    
        
        default:                      
            print_debug(0,"\r\nINT_processNETMAINA> Command not define yet ( %02X )!!\r\n", cmd);                
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
    uint8_t sendPacket[256];
    int packetLen;
    char frameID;
    char frameLen;
    int res;
    int index = 0;
    
    // Start+Len+FrameType+FrameID+destAddr+destNetAddr+BroadcastRadius+Options+  ProtoVer+SendAddr+RecvAddr+  RFData+CheckSum   Byte.
    packetLen = 1+2+1+1+8+2+1+1+1+8+8+dataLen+1;    
    frameLen = packetLen - 4;
    frameID = data[0];
                          
    
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
		
		//print_debug(0,"\r\n dataLen--- %d\r\n index---- %d \r\n\r\n",dataLen,index); 
    memcpy(&sendPacket[index], data, dataLen);                                             // Data
    index += dataLen;
		
		
		
		
		//print_payload(0,&sendPacket[3],index-3);
    /* ----------------------------------------------- */
    sendPacket[index] = xbee_checksum((char*)&sendPacket[3],index-3);                         // Checksum	
//    if(timeout > 0) {
//        res = INT_addWaitQueue(frameID, sendPacket, packetLen, timeout);    
//        if(res < 0) {               
//            free(sendPacket); 
//            return -1;
//        }               
//    }
    
//    print_debug(0,"\r\nPay ---\r\n");   
//    print_payload(0,sendPacket, index+1);  
//    print_debug(0,"-------\r\n");
    
    xbee_write(sendPacket, index+1);       
    return 0;
    
}
/* ================================================================================= */ 