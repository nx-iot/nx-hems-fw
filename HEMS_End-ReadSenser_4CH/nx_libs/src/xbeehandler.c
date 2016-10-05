#include "xbeehandler.h"
#include "print_debug.h"
#include "xbee.h"

char _xbee_join = 0;
char _xbee_Assoc = 0xFF;     // 0x00 = Successfully joined a network       
char _xbee_chkAsso_res = 0;
/* ================================================================================= */ 
int xbee_processMDS(char *buf) {

    unsigned char status;
    status = buf[1]; 
    //printDebug("Modem Status\r\n");
    //printDebug("Status =(%02X)\r\n", status);
    
    switch (status) {
        case HWRST:
            print_debug(0,"         Hardware Reset\r\n");     
            // should set some flag here
            break;
        case WDRST:                                                               
            print_debug(0,"         Watchdog Timer Reset\r\n");  
            break;
        case JOINNET:                                                                         
            print_debug(0,"         Joined to network\r\n");    
            //_xbee_join = 1;
            break;
        case DISAS:                                                                      
            print_debug(0,"         Network disassociated\r\n");       
            //_xbee_join = 0;
            break;
        case COSTART:                                    
            print_debug(0,"         Start Coordinator\r\n");   
            break;
        case KEYUPDT:                                                                     
            print_debug(0,"         Security key was updated\r\n");       
            break;
        case VOLTEXC:                                                                     
            print_debug(0,"         Voltage exceeded\r\n");       
            break;
        case MODCHNG:                                                                     
            print_debug(0,"         Modem config change\r\n");       
            break;
        case STCKERR:                                                                     
            print_debug(0,"         Modem Stack error\r\n");     
            break;
            
        default:
            break; 
    }           
    return 0;
}
/* ================================================================================= */ 
 
int xbee_processZTS(char *buf) {

    //unsigned char frameID;
    unsigned int netAddr = 0x0000;
    //unsigned char retryCount;
    unsigned char delivStatus;
    //unsigned char discovStatus;
    
    //frameID = buf[1];
    netAddr = (buf[2] & 0xffff) << 8;                   
    netAddr |= buf[3];   
    //retryCount = buf[4];                              
    delivStatus = buf[5];
    //discovStatus = buf[6];
    
    if(delivStatus == 0x22) {    // Not Joined to Network
        _xbee_join = 0;    
    }                 
          
    /*printDebug("ZigBee Transmit Status Packet\r\n");  
    printDebug("Frame ID =(%02X)\r\n",frameID);  
    printDebug("Network Addr =(%04X)\r\n",netAddr);   
    printDebug("Retry Count=(%d)\r\n",retryCount);  
    printDebug("Delivery Status =(%02X)\r\n",delivStatus);  
    printDebug("Discovery Status =(%02X)\r\n",discovStatus);*/  
    
    return 0;  

}
/* ================================================================================= */ 
// AT Command Response (0x88)
int xbee_processATCMR(char *buf, int len) {

    //unsigned char frameID;
    unsigned int atCmd = 0x0000;
    unsigned char cmdStatus;
    int dataLen;
    int res;
    
    //frameID = buf[1];
    atCmd = (buf[2] & 0xffff) << 8;                   
    atCmd |= buf[3];   
    cmdStatus = buf[4];                 
          
    //printDebug("AT Command Response Packet\r\n");  
    //printDebug("Frame ID = (%02X)\r\n",frameID);  
    //printDebug("AT Command = (%04X)\r\n",atCmd);   
    //printDebug("Status = (%02X)\r\n",cmdStatus);
    
    switch(cmdStatus) {
        case ATCMDOK:                                                               
            //printDebug("ATCmd OK\r\n");  
            break;
        case ATCMDERR:                                                               
            //printDebug("ATCmd Error\r\n");         
            break;
        case BADCMD:                                                                 
            //printDebug("ATCmd Invalid Command\r\n");    
            break;
        case BADPARAM:                                                               
            //printDebug("ATCmd Invalid Parameter\r\n");
            break;
        case TXFAIL:                                                                 
            //printDebug("ATCmd Tx Failure\r\n");     
            break;
        default:                                                                     
            //printDebug("Unknown Error\r");
            break;
    }                                                                        
    if(cmdStatus) {
        return -1;
    }                       
    dataLen = len - 5; 
    res = process_cmdData(atCmd, &buf[5], dataLen); 
    if(res < 0) {
        return -1;
    }                       
    return 0;  

}
/* ================================================================================= */ 
// Process command data of AT Command Response Packet
int process_cmdData(unsigned int atCmd, char *cmdData, int len) { 

    char i;                                                     
    switch(atCmd) {
                                                  
        case AI:                                                                    
            _xbee_chkAsso_res = 1;
            if (len != 1) {             
                print_debug(0,"XBEE_ASSOC> Invalid response length\r\n");
                return -1;
            }                                                   
            _xbee_Assoc = cmdData[0];
            if(_xbee_Assoc == 0) {
                _xbee_join = 1;   
                print_debug(0,"XBEE_ASSOC>\t OK.\r\n");
            }else {
                _xbee_join = 0;     
                print_debug(0,"XBEE_ASSOC> Association Indication [0x%02X]\r\n", _xbee_Assoc);           
            }                                             
            return 0;
            break;
        case SH:                                                                    
						memcpy(&SENDER_ADDRESS[0],cmdData,4);
            break;
        case SL:                                                                    
						memcpy(&SENDER_ADDRESS[4],cmdData,4);
						nodeAddress_write((uint8_t*)SENDER_ADDRESS);
            break;
            
        default:
            break;
    }
    return 0;
         
}
/* ================================================================================= */ 