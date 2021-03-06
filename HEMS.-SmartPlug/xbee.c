#include <mega128a.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <delay.h>
#include "uart.h"
#include "xbee.h"
#include "debug.h"
#include "xbeehandler.h"
#include "node_address.h"
#include "int_protocol.h"

#define XBEE_RESET PORTE.2
char sr_addr[8];
/* ================================================================================= */ 
int (*funcProcessZTS)(char*) = NULL;                                                      // Zigbee Transmit Status
/* ================================================================================= */ 
int (*funcProcessZRPKT)(char*, int) = NULL;                                               // Zigbee Receive Packet
/* ================================================================================= */ 
int (*funcProcessMDS)(char*) = NULL;                                                      // Modem Status
/* ================================================================================= */ 
int (*funcProcessATCMR)(char*, int) = NULL;                                               // AT Command Response
/* ================================================================================= */ 
//int (*funcProcessRCRES)(char*, int) = NULL;                                               // Remote Command Response
/* ================================================================================= */ 
//int (*funcProcessZIORXI)(char*, int) = NULL;                                              // ZigBee IO Data Sample Rx Indicator
/* ================================================================================= */ 
//int (*funcProcessNI_IND)(char*, int) = NULL;                                              // Node Identification
/* ================================================================================= */ 
// Read data buffer UART0
// Process buffer
void xbee_receivePacket(void) {

    unsigned char *recvPacket;           
    int size = 0;           
            
    // -- for packet validation -- //
    int offset = 0;
    int len = 0x0000;
    int total = 0;
    
    // --------------- Process Xbee Data ---------------- // UART0
    do{
        recvPacket = (char *) malloc (RX_BUFFER_SIZE1);     
        if(recvPacket == NULL){                
            printDebug("<xbee_receivePacket> malloc..FAILED..\r\n");
        }
    }while(recvPacket == NULL);
          
    size = read_uart1(recvPacket, RX_BUFFER_SIZE1-1, 5);       
     
    
    if(size <= 0) {
        free(recvPacket);
        return;
    }
     printDebug("rawpacket\r\n");
            printDebug("\r\nXBee Receive Packet ---\r\n");   
            print_payload(recvPacket, size);  
            printDebug("-------\r\n"); 
    do{
        offset += strcspn(&recvPacket[offset], "\x7E");                                   // seach for header
        if(offset == size){                 
            //printDebug("invalid coordinator packet");                                   // not found Start Delimiter 0x7E
            break;
        }
        len = (recvPacket[offset+1] & 0xffff) << 8;                                       // check packet length (MSB)
        len |= recvPacket[offset+2];                                                      // check packet length (LSB)
        //printDebug("\r\nlen=%d(%04x)\r\n", len, len);
         
        total += len;                                                                     // update total analyse 
        if (total > size){                                                                //check length validation
            offset++;
            total = offset;                                                               // roll back total analyse                          
            printDebug("\r\nXBEE> invalid length!!\r\n");
            continue;
        }                             
          
        if(xbee_checksum(&recvPacket[offset+3], len) != recvPacket[offset+3+len]){        // checksum error detection                  
            printDebug("XBEE> checksum error\r\n");
            offset++;
            total = offset;                                                               // roll back total analyse          
        }
        else{                                                                             // got a valid packet 
            printDebug("checksum correct\r\n");
            printDebug("\r\nXBee Receive Packet ---\r\n");   
            print_payload(&recvPacket[offset+3], len);  
            printDebug("-------\r\n");        
            xbee_processPacket(&recvPacket[offset+3], len);                               // analyse API-specific Structure 
            offset += 3+len;
        }            
	}while(total < size);
    
    free(recvPacket);

}

/* ================================================================================= */ 
// Select Hadler Function                       
int xbee_processPacket(char *buf, int len) {                       

    char frameType;   
    int res, i;                                          
    
    frameType = buf[0];            
    
    switch(frameType) { 
       
        case ATCMR:                                                                       // AT Command Response
            if(len < 5) {                                    
                // shoudn't reach here since checksum valid
                return -1;
            }  
            printDebug("ATCMR : 0x%X\r\n", ATCMR); 
            res = funcProcessATCMR(buf, len);
            if(buf[2]== 0x53 && buf[3] == 0x48){
                sr_addr[0] = buf[5];
                sr_addr[1] = buf[6];
                sr_addr[2] = buf[7];
                sr_addr[3] = buf[8];
                memcpy(&SENDER_ADDRESS[0],&sr_addr[0],4);
            }
            if(buf[2]== 0x53 && buf[3] == 0x4C){
               sr_addr[4] = buf[5];
               sr_addr[5] = buf[6];
               sr_addr[6] = buf[7];
               sr_addr[7] = buf[8];
               memcpy(&SENDER_ADDRESS[4],&sr_addr[4],4);
               nodeAddress_write(SENDER_ADDRESS);
               //memcpy(&specData[0], &SENDER_ADDRESS[0], 8); 
            }
            
            break;  
        case MDS:                                                                         // Modem Status        
            if(len < 2) {            
                // shoudn't reach here since checksum valid
                return -1;
            }        
            printDebug("MDS : 0x%X ", MDS);                             
            res = funcProcessMDS(buf);
            
            break;   
        case ZTS:                                                                         // ZigBee Transmit Status    
            if(len < 7) {                                   
                // shoudn't reach here since checksum valid
                return -1;
            }    
            printDebug("ZTS : 0x%X [%02X]\r\n", ZTS, buf[5]);
            res = funcProcessZTS(&buf[0]);
            break;  
        case ZRPKT:                                                                       // ZigBee Receive packet  
            if(len < 12) {                                  
                // shoudn't reach here since checksum valid
                return -1;
            }          
            //printDebug("ZRPKT : 0x%X\r\n", ZRPKT);                       
            res = funcProcessZRPKT(&buf[0], len);                                         // TNT_COMM_PROTOCOL PACKET
            if (res < 0){   return -1;  }                 
            
            break; 
        case ZERI:                                                                        // ZigBee Explicit Rx Indicator
            // ------ reserved ------ //         
            return 0;
            break;
        case ZIORXI:                                                                      // ZigBee IO Data Sample Rx Indicator
             if(len < 15) {                                  
                // shoudn't reach here since checksum valid
                return -1;
            } 
            printDebug("ZIORXI : 0x%X\r\n", ZIORXI);
            //res = funcProcessZIORXI(&buf[0], len);       
            return 0;
            break; 
        case NI_IND:                                                                      // Node Identification Indicator
            printDebug("NI_IND : 0x%X\r\n", NI_IND);
            //res = funcProcessNI_IND(&buf[0], len); 
            for(i = 0; i < len; i++){
                printDebug("%02X ", buf[i]);
            }   
            printDebug("\r\n");
            return 0;
            break;
        case RCRES:                                                                       // Remote Command Response  
            if(len < 15) {                                  
                // shoudn't reach here since checksum valid
                return -1;
            } 
            printDebug("RCRES : 0x%X\r\n", RCRES);
            //res = funcProcessRCRES(&buf[0], len);
            return 0;
            break;  
                             
        default:                      
            printDebug("\r\nUnknown XBee Frame Type ( %02x )!!\r\n", frameType);                
            return -1;
            break;  
        
    }     
    return res;        
}
/* ================================================================================= */ 
int xbee_checksum(char buf[],int len) {

    int i;
    char sum = 0;                                          
    //print_payload(buf,len);
    for (i = 0; i < len; i++) {
        sum += buf[i];
    }                 
    return (0xff - (sum & 0xff));

}
/* ================================================================================= */ 
// Send API Packet
int xbee_sendAPI(char *frame, int lenght, int timeout) {       
                                                 
    char *sendPacket;                                                                    
    int packetLen = 3+lenght+1;
    //char frameID = frame[1];
    //int res;
                          
    sendPacket  = (char *)malloc(packetLen);        
    if(sendPacket == NULL){                     
        printDebug("WARNING : xbee_sendAPI malloc failed\r\n"); 
        return -1;
    } 
    
    sendPacket[0] = 0x7E;                                                               // Start Delimeter
    sendPacket[1] = (lenght >> 8) & 0xFF;                                               // Xbee Packet Length  - msb
    sendPacket[2] = lenght & 0xFF;                                                      //                     - lsb
    memcpy(&sendPacket[3], frame, lenght);
    sendPacket[packetLen-1] = xbee_checksum(&sendPacket[3],lenght);
    
    /*
    if(timeout > 0) {
        res = xbee_addWaitQueue(frameID, &sendPacket[0], packetLen, timeout);    
        if(res < 0) {               
            free(sendPacket); 
            return -1;
        }               
    }
    */
     
    printDebug("\r\n----------- Send ------------\r\n");
    print_payload(sendPacket, packetLen);
    printDebug("-----------------------------\r\n");
    
    write_uart1(sendPacket, packetLen);       
    free(sendPacket);
    return 0;
    
}
/* ================================================================================= */ 
// Send AT Command, AT Command Q Packet
// frameType use : ATCMD or ATCMQ only
int xbee_sendATCommand(char frameType, int atc, char *param, int paramLen) {       
                                                 
    char *frame;
    int frameLen = 1+1+2+paramLen;       // FrameType+FrameID+ATCommand+ParamLen   Byte.
                          
    frame  = (char *) malloc( frameLen );        
    if(frame == NULL){                     
        printDebug("WARNING : xbee_sendATCommand malloc failed\r\n"); 
        return -1;
    } 
    
    if((frameType != 0x08) && (frameType != 0x09)) {  
        printDebug("WARNING : Frame type is not AT command (%02X)\r\n", frameType);
        return -1;
    }
    frame[0] = frameType;
    frame[1] = 0x01; //xbee_generateId();              // frame ID
    frame[2] = (atc >> 8) & 0xFF;
    frame[3] = atc & 0xFF;      
    if(paramLen > 0){
        memcpy(&frame[4], param, paramLen);  
    }   
    
    xbee_sendAPI(frame, frameLen, XBEE_TIMEOUT);
          
    free(frame);    
    return 0;
}
/* ================================================================================= */ 
// Send Zigbee Transmit Request Packet
int xbee_sendZTR(char *destAddr, char *destNetAddr, char *data, int dataLen) {       
                                                 
    char *frame;
    int frameLen = 1+1+8+2+1+1+dataLen;       // FrameType+FrameID+destAddr+destNetAddr+BroadcastRadius+Options+RFData   Byte.
                          
    frame  = (char *) malloc( frameLen );        
    if(frame == NULL){                     
        printDebug("WARNING : xbee_sendZTR malloc failed\r\n"); 
        return -1;
    } 
    
    frame[0] = 0x10;
    frame[1] = 0x01; //xbee_generateId();
    memcpy(&frame[2], destAddr, 8);  
    memcpy(&frame[10], destNetAddr, 2);
    frame[12] = 0x00;     // BroadcastRadius
    frame[13] = 0x00;     // Options
    memcpy(&frame[14], data, dataLen);
    
    xbee_sendAPI(frame, frameLen, XBEE_TIMEOUT);
          
    free(frame);
    return 0;
    
}
/* ================================================================================= */ 
void xbee_reset(void) {

    printDebug("XBEE> Resetting XBee Module, please wait.\r\n");
    XBEE_RESET = 0;
    delay_ms(500);
    XBEE_RESET = 1;  
    delay_ms(1);                 
    _xbee_join = 0;
    _xbee_Assoc = 0xFF;
    xbee_sendATCommand(ATCMQ, NR, "\x00", 1); 
    
}                                                        
/* ================================================================================= */ 
// Read Serial Number
void xbee_readSerialNumber(void) {       
    printDebug("XBEE> Get Adress Nathapong...\r\n");                                                    
    xbee_sendATCommand(ATCMQ, SH, NULL, 0);
    delay_ms(100);
    xbee_sendATCommand(ATCMQ, SL, NULL, 0);
    delay_ms(100);
    
}
/* ================================================================================= */ 
void xbee_leaveNetwork(void) {

    printDebug("XBEE> Leaving Network...\r\n");              
    _xbee_join = 0;
    _xbee_Assoc = 0xFF;
    xbee_sendATCommand(ATCMQ, NR, "\x00", 1);
    delay_ms(500); 
    
}                                                        
/* ================================================================================= */ 
// use Association Indication (AI Command)
void xbee_checkAssociation(void) {       
                                                         
    _xbee_chkAsso_res = 0;
    xbee_sendATCommand(ATCMQ, AI, NULL, 0);
    
}
/* ================================================================================= */ 
// For End-Device only
// Join scaning will enable after reset module
void xbee_disableJoinScan(void) {

    printDebug("XBEE> Disable Join scaning.\r\n");                 
    //_xbee_join = 0;
    xbee_sendATCommand(ATCMQ, DJ, "\x01", 1); 
    
}                                                        
/* ================================================================================= */ 

