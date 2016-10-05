#include "xbee.h"
#include "print_debug.h"
#include "uart2_manag.h"
#include "xbeehandler.h"
#include "handler.h"
#include "protocol.h"

uint8_t sr_addr[8];
char RECEIVER_ADDRESS[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
char SENDER_ADDRESS[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 

///* ================================================================================= */ 
//int (*funcProcessZTS)(char*) = NULL;                                                      // Zigbee Transmit Status
///* ================================================================================= */ 
//int (*funcProcessZRPKT)(char*, int) = NULL;                                               // Zigbee Receive Packet
///* ================================================================================= */ 
//int (*funcProcessMDS)(char*) = NULL;                                                      // Modem Status
///* ================================================================================= */ 
//int (*funcProcessATCMR)(char*, int) = NULL;                                               // AT Command Response
///* ================================================================================= */ 
/*
xbee_api_proc std. => xbee api process
*/
void xbee_api_proc(){

}

void xbee_at_cmd(){

}

int8_t xbee_read(uint8_t *value, uint16_t *len){
	uint16_t lenn = 0;
	int8_t ret = 0;
	ret = uart2_read(value,len,255);
	memcpy(&lenn,len,sizeof(uint16_t));
	//print_payload(0,value,lenn);
	
//	if(len > 0){
//		HAL_UART_Transmit(&huart3,value,*len,10);
//	}
	xbee_flush();
	return ret;
}

int8_t xbee_write(uint8_t *value, uint16_t len){
	int8_t ret = 0;
	print_payload(0,value,len);
	ret = uart2_write(value, len);
	return ret;
}

/* ================================================================================= */
void xbee_receivePacket(void) {

    uint8_t recvPacket[256];           
    int ret = 0;           
            
    // -- for packet validation -- //
    int offset = 0;
    uint16_t size = 0x0000;
		uint16_t len = 0x0000;
    int total = 0;
    
    // --------------- Process Xbee Data ---------------- // UART0
      
    ret = xbee_read(recvPacket, &size);       
    //print_debug(0,"Size %d -------\r\n",size);  
		if(size <= 0 && len <=0) {
        return;
		}
		 if(recvPacket[0] != 0x7E){
			 //recvPacket[0] = 0x7E;
			 return;
		 }
     print_debug(0,"rawpacket\r\n");
            print_debug(0,"\r\nXBee Receive Packet ---\r\n");   
            print_payload(0,recvPacket, size);  
            print_debug(0,"-------\r\n"); 
	
		 while(total < size){
        offset += strcspn((char*)&recvPacket[offset], "\x7E");                                   // seach for header
        if(offset == size){                 
            //printDebug("invalid coordinator packet");                                   // not found Start Delimiter 0x7E
            break;
        }
        len = (recvPacket[offset+1] & 0xffff) << 8;                                       // check packet length (MSB)
        len |= recvPacket[offset+2];                                                      // check packet length (LSB)
        //print_debug(0,"\r\nlen=%d(%04x)\r\n", len, len); 
        total += len;                                                                     // update total analyse
//				if((char)recvPacket[offset+3+len] == 0){
//					return;
//				}
        if (total > size){                                                                //check length validation
            offset++;
            total = offset;                                                               // roll back total analyse                          
            print_debug(0,"\r\nXBEE> invalid length!!\r\n");
					//break;
            continue;
        } 
				if(xbee_checksum((char*)&recvPacket[offset+3], len) == (char)recvPacket[offset+3+len]){        // checksum error detection                   
						print_debug(0,"checksum correct\r\n");
            print_debug(0,"\r\nXBee Receive Packet ---\r\n");   
            print_payload(0,&recvPacket[offset+3], len);  
            print_debug(0,"-------\r\n"); 			
            xbee_processPacket((char*)&recvPacket[offset+3], len);                               // analyse API-specific Structure 
            offset += 3+len;
        }
        else{                                                                             // got a valid packet 
            print_debug(0,"XBEE> checksum error\r\n");
            offset++;
            total = offset;                                                               // roll back total analyse 
        }
	}
	//print_debug(0,"Out : total %d---- size %d---\r\n",total,size); 
}

int8_t xbee_getserial(void) {

    uint8_t recvPacket[256];           
    int ret = 0;           
            
    // -- for packet validation -- //
    int offset = 0;
    uint16_t size = 0x0000;
		uint16_t len = 0x0000;
    int total = 0;
    int count = 0;
    // --------------- Process Xbee Data ---------------- // UART0
      
    ret = xbee_read(recvPacket, &size);       
    //print_debug(0,"Size %d -------\r\n",size);  
		if(size <= 0 && len <=0) {
        return 0;
		}
		 if(recvPacket[0] != 0x7E){
				return 0;
		 }
     print_debug(0,"rawpacket\r\n");
            print_debug(0,"\r\nXBee Receive Packet ---\r\n");   
            print_payload(0,recvPacket, size);  
            print_debug(0,"-------\r\n"); 
		 
    do{
        offset += strcspn((char*)&recvPacket[offset], "\x7E");                                   // seach for header
        if(offset == size){                 
            //printDebug("invalid coordinator packet");                                   // not found Start Delimiter 0x7E
            break;
        }
        len = (recvPacket[offset+1] & 0xffff) << 8;                                       // check packet length (MSB)
        len |= recvPacket[offset+2];                                                      // check packet length (LSB)
        print_debug(0,"\r\nlen=%d(%04x)\r\n", len, len); 
        total += len;                                                                     // update total analyse 
        if (total > size){                                                                //check length validation
            offset++;
            total = offset;                                                               // roll back total analyse                          
            print_debug(0,"\r\nXBEE> invalid length!!\r\n");
						return 0;
            continue;
        } 
				//print_debug(0,"XBEE> checksum Value : %d Value : %d\r\n",xbee_checksum((char*)&recvPacket[offset+3], len),recvPacket[offset+3+len]);
        if(xbee_checksum((char*)&recvPacket[offset+3], len) == (char)recvPacket[offset+3+len]){        // checksum error detection                   
						print_debug(0,"checksum correct\r\n");
            print_debug(0,"\r\nXBee Receive Packet ---\r\n");   
            print_payload(0,&recvPacket[offset+3], len);  
            print_debug(0,"-------\r\n");        
            xbee_processPacket((char*)&recvPacket[offset+3], len);                               // analyse API-specific Structure 
            offset += 3+len;
						count ++;
						if(count == 2){
							return 1;
						}
					
        }
        else{                                                                             // got a valid packet 
						
            print_debug(0,"XBEE> checksum error\r\n");
						return 0;
            offset++;
            total = offset;                                                               // roll back total analyse 

        }            
	}while(total < size);
return 0;
}

/* ================================================================================= */
void xbee_flush(){
	uart2_flush();
}
/* ================================================================================= */
// Select Hadler Function                       
int xbee_processPacket(char *buf, int len) {                       

		uint8_t frameType;   
    int res, i;                                          
    
    frameType = buf[0];            
    
    switch(frameType) { 
       
        case ATCMR:                                                                       // AT Command Response
            if(len < 5) {                                    
                // shoudn't reach here since checksum valid
                return -1;
            }  
            print_debug(0,"ATCMR : 0x%X\r\n", ATCMR); 
            res = xbee_processATCMR(buf, len);
            break;  
        case MDS:                                                                         // Modem Status        
            if(len < 2) {            
                // shoudn't reach here since checksum valid
                return -1;
            }        
            print_debug(0,"MDS : 0x%X ", MDS);                             
            res = xbee_processMDS(buf);
            
            break;   
        case ZTS:                                                                         // ZigBee Transmit Status    
            if(len < 7) {                                   
                // shoudn't reach here since checksum valid
                return -1;
            }    
            print_debug(0,"ZTS : 0x%X [%02X]\r\n", ZTS, buf[5]);
            res = xbee_processZTS(&buf[0]);
            break;  
        case ZRPKT:                                                                       // ZigBee Receive packet  
            if(len < 12) {                                  
                // shoudn't reach here since checksum valid
                return -1;
            }          
            print_debug(0,"ZRPKT : 0x%X\r\n", ZRPKT);                       
						res = INT_processPacket(&buf[0], len);                                         // TNT_COMM_PROTOCOL PACKET
            //print_debug(0,"res : %d\r\n", res);   
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
            print_debug(0,"ZIORXI : 0x%X\r\n", ZIORXI);
            //res = funcProcessZIORXI(&buf[0], len);       
            return 0;
            break; 
        case NI_IND:                                                                      // Node Identification Indicator
            print_debug(0,"NI_IND : 0x%X\r\n", NI_IND);
            //res = funcProcessNI_IND(&buf[0], len); 
            for(i = 0; i < len; i++){
                print_debug(0,"%02X ", buf[i]);
            }   
            print_debug(0,"\r\n");
            return 0;
            break;
        case RCRES:                                                                       // Remote Command Response  
            if(len < 15) {                                  
                // shoudn't reach here since checksum valid
                return -1;
            } 
            print_debug(0,"RCRES : 0x%X\r\n", RCRES);
            //res = funcProcessRCRES(&buf[0], len);
            return 0;
            break;  
                             
        default:                      
            print_debug(0,"\r\nUnknown XBee Frame Type ( %02x )!!\r\n", frameType);                
            return -1;
            break;  
        
    }     
    return res;      
}
/* ================================================================================= */
uint8_t nodeAddress_write(uint8_t *addr){
    uint8_t i = 0;
    print_debug(0,"XBEE SENDER_ADDRESS   : ");
    for(i = 0; i < 8; i++)  {
        print_debug(0,"%02X ", addr[i]);    
    }
    print_debug(0,"\r\n");
    return 0;
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
                                                 
    uint8_t sendPacket[256];                                                                    
    int packetLen = 3+lenght+1;
    //char frameID = frame[1];
    //int res;
                              
    
    sendPacket[0] = 0x7E;                                                               // Start Delimeter
    sendPacket[1] = (lenght >> 8) & 0xFF;                                               // Xbee Packet Length  - msb
    sendPacket[2] = lenght & 0xFF;                                                      //                     - lsb
    memcpy(&sendPacket[3], frame, lenght);
    sendPacket[packetLen-1] = xbee_checksum((char*)&sendPacket[3],lenght);
    
    /*
    if(timeout > 0) {
        res = xbee_addWaitQueue(frameID, &sendPacket[0], packetLen, timeout);    
        if(res < 0) {               
            free(sendPacket); 
            return -1;
        }               
    }
    */
     
    print_debug(0,"\r\n----------- Send ------------\r\n");
    print_payload(0,sendPacket, packetLen);
    print_debug(0,"-----------------------------\r\n");
    
    xbee_write(sendPacket, packetLen);
		//uart2_write(sendPacket, packetLen);
    return 0;
    
}
/* ================================================================================= */
// Send AT Command, AT Command Q Packet
// frameType use : ATCMD or ATCMQ only
int xbee_sendATCommand(char frameType, int atc, char *param, int paramLen) {       
                                                 
    char frame[256];
    int frameLen = 1+1+2+paramLen;       // FrameType+FrameID+ATCommand+ParamLen   Byte.
                          
    
    if((frameType != 0x08) && (frameType != 0x09)) {  
        print_debug(0,"WARNING : Frame type is not AT command (%02X)\r\n", frameType);
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
          
    return 0;
}
/* ================================================================================= */ 


// use Association Indication (AI Command)
void xbee_checkAssociation(void) {       
                                                         
    //_xbee_chkAsso_res = 0;
    xbee_sendATCommand(ATCMQ, AI, NULL, 0);
    
}
/* ================================================================================= */ 
// Read Serial Number
void xbee_readSerialNumber(void) {       
    print_debug(0,"XBEE> Get Adress Nathapong...\r\n");                                                    
    xbee_sendATCommand(ATCMQ, SH, NULL, 0);
    HAL_Delay(100);
    xbee_sendATCommand(ATCMQ, SL, NULL, 0);
    HAL_Delay(100);
    
}
/* ================================================================================= */
