#include "protocol.h"
#include "print_debug.h"
#include "uart2_manag.h"
#include "xbee.h"
#include "handler.h"

extern unsigned char  Read_SW[16];
int INT_sendNetworkMaintenanceCmd(char cmd, char *specData, int specDataLen, int timeout) {

    char frameData[256];
    int frameLen;   
    int index;
    int res;
    
    frameLen = 1+1+1+specDataLen;     // frameID+ frameType+ Cmd+ specData     byte.
    index = 0;
    
    //frameData = (char *) malloc (frameLen);     
//    if(frameData == NULL) {                
//        print_debug(0,"\r\nWARNING : <INT_sendNetworkMaintenanceCmd> malloc..FAILED (%d byte).\r\n", frameLen);
//        return -1;
//    }
   
//    if(cmd == HEARTBEAT) { 
//        _hb_res = 0;      
//    }else if(cmd == JOINREQ) {
//        _joinreq_res = 0;
//    }                
    frameData[index++] = 0xFF;                                             // Frame ID (Fixed 0xFF)
    frameData[index++] = NETMAINC;                                         // Frame Type                             
    frameData[index++] = cmd;                                              // CMD
    if(specDataLen > 0){
        memcpy(&frameData[index], specData , specDataLen);                  // Specific Structure
        index += specDataLen;  
    }
    
    print_debug(0,"<<< [INT_sendNetworkMaintenanceCmd [0x%02X]] Sending..\r\n", cmd);
    print_debug(0,"TimeOut : %d\r\n", timeout);		
		res = INT_sendPacket(RECEIVER_ADDRESS, frameData, frameLen, timeout);     
//    if(res < 0) {
//        free(frameData);                
//        return -1;
//    }  
//    free(frameData);
    return 0;
}
/* ================================================================================= */
int INT_sendEventReportCmd(unsigned long int timeStamp, char recordType, char eventID, char *status, int timeout) {

    uint8_t frameData[256];
    int frameLen;   
    int res, i;
    int index = 0;
    
    frameLen = 1+1+4+8+1+1+4;    // frameID+ frameType+ TIM+ SID+ RTYP+ EventID+ Status     byte.

//    if((recordType != RLT) && (recordType != LOG) && (recordType != SMART_PLUG) && (recordType != SMART_THERMO)) {                 // Record type 0x20 Fixed for HEMS Project.          
//        print_debug(0,"\r\nWARNING : <INT_sendEventReportCmd> Wrong recordType 0x%02X.\r\n", recordType);
//        free(frameData);
//        return -1;
//    }
     
    frameData[index++] = 0xFF;                                 // Frame ID
    frameData[index++] = EVENREPC;                                         // Frame Type
    memcpy(&frameData[index], &timeStamp, sizeof(timeStamp));              // Time Stamp
    index += sizeof(timeStamp);
    
    for(i = 0; i < 8; i++){                                                // Source ID
        frameData[index++] = SENDER_ADDRESS[i];
    }
                                                                         
    frameData[index++] = recordType;                                       // Record Type
    frameData[index++] = eventID;                                          // Event ID
    memcpy(&frameData[index], status, 4);                                  // Status
    index += 4;
    
    print_debug(0,"<<< [INT_sendEventReportCmd] Sending..\r\n");
    print_payload(0,frameData,index);
    res = INT_sendPacket(RECEIVER_ADDRESS, (char*)frameData, frameLen, timeout);     
    
    return 0;
}
/* ================================================================================= */
int INT_sendDataReportCmd(unsigned long int timeStamp, char recordType, char numSet, DATASET *data, int timeout) {

    uint8_t frameData[256];
    int frameLen;   
    int res, i;
    int index;
    
    //frameLen = 1+1+4+8+1+1+(sizeof(DATASET)*numSet);    // frameID+ frameType+ TIM+ SID+ RTYP+ NUM_D+ SetData*NumSet     byte.
		frameLen = 1+1+4+8+1+1+(6*(int)numSet); 
    index = 0;
     
    frameData[index++] = 0xFF;                                 // Frame ID
    frameData[index++] = DATAREPC;                                         // Frame Type
    memcpy(&frameData[index], &timeStamp, sizeof(timeStamp));              // Time Stamp
    index += sizeof(timeStamp);
    
    for(i = 0; i < 8; i++){                                                // Source ID
        frameData[index++]  = SENDER_ADDRESS[i];
    }
                                                                         
    frameData[index++] = recordType;                                       // Record Type
    frameData[index++] = numSet;                                           // Number of data set
    
    for(i = 0; i < numSet; i++) {                                          // Data Set
        frameData[index++] = data[i].dataID;                               // - Data ID
        frameData[index++] = data[i].dataType;                             // - Data Type
        memcpy(&frameData[index], &data[i].value, sizeof(data[i].value));  // - Data 
        index += sizeof(data[i].value);
    }
    
    print_debug(0,"<<< [INT_sendDataReportCmd] Sending..\r\n");
    //print_payload(0,frameData, index);
    res = INT_sendPacket(RECEIVER_ADDRESS, (char*)frameData, frameLen, timeout);     
    return 0;
}
/* ================================================================================= */
int INT_sendStatusReportCmd(unsigned long int timeStamp, char accm, char *param, int paramLen, int timeout) {

    uint8_t frameData[256];
    int frameLen;   
    int res, i;
    int index;
    
    frameLen = 1+1+4+8+1+paramLen;     // frameID+ frameType+ TIM+ SID+ ACCM+ parameterLen     byte.
    index = 0;
    
    frameData[index++] = 0xFF;                                 						 // Frame ID
    frameData[index++] = STATREPC;                                         // Frame Type
    memcpy(&frameData[index], &timeStamp, sizeof(timeStamp));              // Time Stamp
    index += sizeof(timeStamp);
    
    for(i = 0; i < 8; i++){                                                // Source ID
        frameData[index++]  = SENDER_ADDRESS[i];
    }
    
    frameData[index++] = accm;                                             // Access Method
    // !!! not use memcpy(.., &param, ..);
    // can use param or &param[0]  
    memcpy(&frameData[index], param, paramLen);                            // Parameter Response
    index += paramLen;

    print_debug(0,"<<< [INT_sendStatusReportCmd [%02X]] Sending..\r\n", accm);
    print_payload(0,frameData, index);
    res = INT_sendPacket(RECEIVER_ADDRESS, (char*)frameData, frameLen, timeout);     
    return 0;
}

/* ================================================================================= */
/*************************************************************************************/
/************************** Process INT_COMM_PROTOCAL PACKET *************************/
/*************************************************************************************/
/* ================================================================================= */ 
int INT_processPacket(char *buf, int len) {

    // Zigbee Protocol Variable
    unsigned char srcAddr[9];
    unsigned int srcNetAddr = 0x0000;
    //unsigned char recvOption;
    unsigned int dataLen;           // TNT_COMM_PROTOCOL PACKET Length
    
    // INT_COMM_PROTOCOL Variable
    //unsigned char protoVersion;
    unsigned char senderAddr[8];
    unsigned char recvAddr[8];
    unsigned char frameType;
    unsigned char frameID;
    int8_t res;
    // - Status Request Cmd
    //unsigned int interval;
    //unsigned char accm;
    //unsigned char listID;
    // - Remote Device Control
    char command;
    
    // Define Zigbee Protocol Variable
    memcpy(srcAddr, &buf[1], 8); 
    srcNetAddr  = (buf[9]&0xFFFF) << 8;                   
    srcNetAddr |= buf[10];  
    //recvOption  = buf[11];
      
    // Define INT_COMM_PROTOCOL Variable
    memcpy(senderAddr, &buf[13], 8);        // Sender Address in Header of INT_COMM_PROTOCAL
    memcpy(recvAddr, &buf[21], 8);          // Receiver Address in Header of INT_COMM_PROTOCAL
    //protoVersion    = buf[12];
    frameID         = buf[29]; 
    frameType       = buf[30];
    
    // Define data length of INT_COMM_PROTOCOL PACKET
    dataLen = len-1-8-2-1;              // Data Lenght = Len- FrameType- SrcAddr- SrcNetAddr- ReceiveOption     Byte.
    
//    //----- Save address of coordinator in first times ----//
//    if(!_flag_saveRecvAddr) {
//        _flag_saveRecvAddr = 1;
//        memcpy(_receiveAddress, senderAddr, 8);
//        //printDebug("SAVED RECEIVER_ADDRESS\r\n");
//    }
     
     switch(frameType) { 
        /*------------------ Command Packet ------------------*/
//        case STATREQC :                                                       // -- 0x1A Status Request COMMAND 
//            if(dataLen < 30){                                    
//                // shoudn't reach here since checksum valid
//                INT_sendProtocolErrorFeedbackAck(frameID, INVALID_LEN);
//                return -1;
//            }  
//            print_debug(0,">>> STATUS_REQUEST_CMD : 0x%X\r\n", STATREQC);
//            //accm = buf[39];
//            //interval = (buf[40]&0xFFFF) << 8;
//            //interval |= buf[41];
//            //listID = accm & 0x0F;
//            
//            
//            break;
//        case NETMAINC :                                                       // -- 0x7A Network Maintenance COMMAND
//            if(dataLen < 20){                                    
//                // shoudn't reach here since checksum valid
//                INT_sendProtocolErrorFeedbackAck(frameID, INVALID_LEN);
//                return -1;
//            }  
//            print_debug(0,">>> NETWORK_MAINTENANCE_CMD : 0x%X\r\n", NETMAINC);
//            break;
//        case RMDCTRLC :                                                       // -- 0x8A Remote Device Control COMMAND  
//            if(dataLen < 32){                                    
//                // shoudn't reach here since checksum valid
//                INT_sendProtocolErrorFeedbackAck(frameID, INVALID_LEN);
//                return -1;
//            }  
//            print_debug(0,">>> REMOTE_DEVICE_CONTROL_CMD : 0x%X\r\n", RMDCTRLC);
//            command = buf[39];
//            res = funcProcessRMDCTRLC(&buf[29], dataLen);
//            if(res < 0) {
//                INT_sendRemoteDeviceControlAck(frameID, command, 0x01, 0);
//                return -1;
//            }
//            INT_sendRemoteDeviceControlAck(frameID, command, 0x00, 0);
//            break;
//        //------------------ Acknowledgment Packet ------------------//
        case STATREPA :                                                        // -- 0x2B Status Report ACKNOWLEDGE
            if(dataLen < 23){                                    
                // shoudn't reach here since checksum valid
                //INT_sendProtocolErrorFeedbackAck(frameID, INVALID_LEN);
                return -1;
            }  
            print_debug(0,">>> STATUS_REPORT_ACK : 0x%X\r\n", STATREPA);
            //INT_removeWaitQueue(frameID);
            //INT_setTimeSync(&buf[31]);
            break;
        case EVENREPA :                                                        // -- 0x3B Event Report ACKNOWLEDGE
            if(dataLen < 19){                                    
                // shoudn't reach here since checksum valid
                //INT_sendProtocolErrorFeedbackAck(frameID, INVALID_LEN);
                return -1;
            }  
            print_debug(0,">>> EVENT_REPORT_ACK : 0x%X\r\n", EVENREPA);
            //INT_removeWaitQueue(frameID);
            break;
        case DATAREPA :                                                        // -- 0x5B Data Report ACKNOWLEDGE
            if(dataLen < 19){                                    
                // shoudn't reach here since checksum valid
                //INT_sendProtocolErrorFeedbackAck(frameID, INVALID_LEN);
                return -1;
            }  
            print_debug(0,">>> DATA_REPORT_ACK : 0x%X\r\n", DATAREPA);
            //INT_removeWaitQueue(frameID);
            break;  
        case NETMAINA :                                                        // -- 0x7B Network Maintenance ACKNOWLEDGE       
            if(dataLen < 21){            
                // shoudn't reach here since checksum valid
                //INT_sendProtocolErrorFeedbackAck(frameID, INVALID_LEN);
                return -1;
            }        
            print_debug(0,">>> NETWORK_MAINTENANCE_ACK : 0x%X\r\n", NETMAINA);
            if(buf[31] == HEARTBEAT || buf[31] == JOINREQ) {
                if(buf[32] == 0x00) {            // Ack OK
                    memcpy(&RECEIVER_ADDRESS[0], &senderAddr[0], 8);
                    //printDebug("SAVED RECEIVER_ADDRESS\r\n");    
                }
            }
            
            res = INT_processNETMAINA(&buf[29], dataLen);
            if(res < 0) {
                return -1;
            }                                 
            break;
        case PROTOERROR :                                                      // -- 0xEB Protocol Error Feedback        
            if(dataLen < 20){            
                // shoudn't reach here since checksum valid
                //INT_sendProtocolErrorFeedbackAck(frameID, INVALID_LEN);
                return -1;
            }        
            print_debug(0,"\r\n>>> PROTOCOL_ERROR_FEEDBACK_ACK : FRAME ID[0x%02X] ERROR ID[0x%02X]\r\n\r\n", frameID, buf[31] );                                 
            break;
				case RMDCTRLC :
						Remote_Relay(buf);
						break;
        default:                      
            print_debug(0,"\r\n>>> RECEIVE INT_COMM> Frame Type not define yet (0x%02X)!!\r\n\r\n", frameType);
            //INT_sendProtocolErrorFeedbackAck(frameID, INVALID_FTYPE);                
            return -1;
            break;
     }
                       
    /*printDebug("ZigBee Receive Packet\r\n");
    printDebug("Source Addr : "); 
    for(i = 1; i < 9; i++) {
          printDebug("%02X ", buf[i]);            
    }    
    printDebug("\r\nNetwork Addr =(%04X)\r\n", srcNetAddr);    
    printDebug("Receive Optiobs =(%02X)\r\n", recvOption);
    printDebug("Data : ");  
    for(i = 0; i < dataLen; i++) {
          printDebug("%02X ", data[i]);            
    }  
    printDebug("\n");*/ 
    return 0;     

}

void Remote_Relay(char *buf)
{
	
	if(buf[39]==RELAY_CH1)
	{
		if(buf[40]==1)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,1);
		}
		else if(buf[40]==0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,0);
		}
		 //Read_SW[0] = buf[40];
	}
	
	else if(buf[39]==RELAY_CH2)
	{
		if(buf[40]==1)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,1);
		}
		else if(buf[40]==0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,0);
		}
	}
	
	else if(buf[39]==RELAY_CH3)
	{
		if(buf[40]==1)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,1);
		}
		else if(buf[40]==0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,0);
		}
	}
	
	else if(buf[39]==RELAY_CH4)
	{
		if(buf[40]==1)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,1);
		}
		else if(buf[40]==0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,0);
		}
	}
	
	else if(buf[39]==RELAY_CH5)
	{
		if(buf[40]==1)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,1);
		}
		else if(buf[40]==0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,0);
		}
	}
	
	else if(buf[39]==RELAY_CH6)
	{
		if(buf[40]==1)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,1);
		}
		else if(buf[40]==0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,0);
		}
	}
	
	else if(buf[39]==RELAY_CH7)
	{
		if(buf[40]==1)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,1);
		}
		else if(buf[40]==0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,0);
		}
	}
	
	else if(buf[39]==RELAY_CH8)
	{
		if(buf[40]==1)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,1);
		}
		else if(buf[40]==0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,0);
		}
	}
	buf[39]=(buf[39]-0x20)+0x01;
	print_debug(0,"\r\n-------------------------------------------------------\r\n");  
	print_debug(0,"[Remote Channel]  : Relay Channel %d\r\n", buf[39]);   
	if(buf[40]==1)
		print_debug(0,"[Remote Control] : SWITCH ON!\r\n");
	if(buf[40]==0)
		print_debug(0,"[Remote Control] : SWITCH OFF!\r\n");
	print_debug(0,"\r\n-------------------------------------------------------\r\n");  
}