#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include "debug.h"
#include "uart.h"
#include "adc.h"
#include "initial_system.h"

//flash unsigned char cmdSetVref[]        = "$CMDSVR";            /*-- Set Vref measure from multimeter --*/ 
flash unsigned char cmdSetAdj0Sen5A[]   = "$CMDADJ05";          /*-- Set Adj 0 Amp. Current Sensor 5 Amp. --*/
flash unsigned char cmdSetAdj0Sen20A[]  = "$CMDADJ20";          /*-- Set Adj 0 Amp. Current Sensor 20 Amp. --*/
flash unsigned char cmdSetAdj0Sen30A[]  = "$CMDADJ30";          /*-- Set Adj 0 Amp. Current Sensor 30 Amp. --*/
//flash unsigned char cmdSetWorkingMode[] = "$CMDSWM";            /*-- Set Working Mode. --*/
/* ================================================================================= */ 
//static void DEBUG_setRealVref(void);
static void DEBUG_setADJ0CurrentSensor(uint8_t sensor);
//static void DEBUG_setWorkingMode(void);
/* ================================================================================= */ 
//static void DEBUG_setRealVref(void) {
//
//    unsigned char i;
//    unsigned char rxStrSetVREF[6];
//                    
//    for(i = 0; i <= 5; i++) {
//        rxStrSetVREF[i] = rxPackage0[i+8];  
//    }
//    rxStrSetVREF[5] = '\0';
//    printDebug("<DEBUG_setRealVref> get set Vref<string> %s\r\n", rxStrSetVREF);
//    _adcVref = (atof(rxStrSetVREF));
//    printDebug("<DEBUG_setRealVref> After convert = %f\r\n", _adcVref);
//     
//}
/* ================================================================================= */ 
static void DEBUG_setADJ0CurrentSensor(uint8_t sensor) {

    unsigned char i;
    unsigned char rxStrSetADJ[6];
                    
    for(i = 0; i <= 5; i++) {
        rxStrSetADJ[i] = rxPackage0[i+10];  
    }
    rxStrSetADJ[5] = '\0';
    printDebug("<DEBUG_setADJ0CurrentSensor> get set ADJ0 Current Sensor<string> %s\r\n", rxStrSetADJ);
    switch(sensor) { 
        case 5:
            ADJ0_SENSOR5A = (atof(rxStrSetADJ));
            printDebug("<DEBUG_setADJ0CurrentSensor> After convert (Sensor 5A.) = %0.4f\r\n", ADJ0_SENSOR5A);
            break;
        case 20:
            ADJ0_SENSOR20A = (atof(rxStrSetADJ));
            printDebug("<DEBUG_setADJ0CurrentSensor> After convert (Sensor 20A.) = %0.4f\r\n", ADJ0_SENSOR20A);
            break;
        case 30:
            ADJ0_SENSOR30A = (atof(rxStrSetADJ));
            printDebug("<DEBUG_setADJ0CurrentSensor> After convert (Sensor 30A.) = %0.4f\r\n", ADJ0_SENSOR30A);
            break;
    }
     
}
/* ================================================================================= */ 
//static void DEBUG_setWorkingMode(void) {
//    unsigned char rxParam[2]; 
//    unsigned char tmp;
//                    
//    rxParam[0] = rxPackage0[8];
//    rxParam[1] = '\0';
//    printDebug("<DEBUG_setWorkingMode> get set Working Mode %s\r\n", rxParam);
//    tmp = (atoi(rxParam));  
//    if(tmp == 1) {
//        WORKING_MODE = CURRENT_MONITOR_MODE; 
//        printDebug("<DEBUG_setWorkingMode> Working Mode: CURRENT_MONITOR_MODE\r\n");
//    }else {
//        WORKING_MODE = NORMAL_MODE;
//        printDebug("<DEBUG_setWorkingMode> Working Mode: NORMAL_MODE\r\n");
//    }   
//}
/* ================================================================================= */ 
void debug_req(void) {

    _FlagPackageTerminate0 = 0;
    printDebug("\r\nGet Debug Req : \r\n");    
                                  
//    if(!strncmpf(rxPackage0, cmdSetVref, 7)) {
//        DEBUG_setRealVref();
//    }else 
    if(!strncmpf(rxPackage0, cmdSetAdj0Sen5A, 9)) {
        DEBUG_setADJ0CurrentSensor(5);
    }else if(!strncmpf(rxPackage0, cmdSetAdj0Sen20A, 9)) {
        DEBUG_setADJ0CurrentSensor(20);
    }else if(!strncmpf(rxPackage0, cmdSetAdj0Sen30A, 9)) {
        DEBUG_setADJ0CurrentSensor(30);
    }
//    else if(!strncmpf(rxPackage0, cmdSetWorkingMode, 7)) {
//        DEBUG_setWorkingMode();
//    }  
    
    return;         
}
/* ================================================================================= */ 
void printDebug(flash char *fmtstr, ...)
{
    char textBuffer[256];
    va_list argptr; 
    
    va_start(argptr, fmtstr);
    vsprintf(textBuffer,fmtstr,argptr);     
    send_uart(0, textBuffer);   
    va_end(argptr);        
    
    return;        
}
/* ================================================================================= */ 
 void print_payload(const unsigned char *payload, int len) {

    int len_rem = len;
    int line_width = 16;            // number of bytes per line //
    int line_len;
    int offset = 0;                    // zero-based offset counter //
    const unsigned char *ch = payload;

    if (len <= 0)
        return;

    // data fits on one line //
    if (len <= line_width) {
        print_hex_ascii_line(ch, len, offset);
        return;
    }
    // data spans multiple lines //
    for ( ;; ) {
        // compute current line length //
        line_len = line_width % len_rem;
        // print line //
        print_hex_ascii_line(ch, line_len, offset);
        // compute total remaining //
        len_rem = len_rem - line_len;
        // shift pointer to remaining bytes to print //
        ch = ch + line_len;
        // add offset //
        offset = offset + line_width;
        // check if we have line width chars or less //
        if (len_rem <= line_width) {
            // print last line and get out //
            print_hex_ascii_line(ch, len_rem, offset);
            break;
        }
    }
    return;
    
}                                 
/* ================================================================================= */ 
void print_hex_ascii_line(const unsigned char *payload, int len, int offset) {

    int i;
    int gap;
    const unsigned char *ch;

    // offset //                      
    printDebug("%05d   ", offset);               
    
    
    // hex //                                                                                                      
    ch = payload;
    for(i = 0; i < len; i++) {                            
        printDebug("%02X ", *ch);                    
        
        ch++;
        // print extra space after 8th byte for visual aid //
        if (i == 7){                            
            printDebug(" ");                                     
            
        }
    }
    // print space to handle line less than 8 bytes //
    if (len < 8){                            
        printDebug(" ");                                              
        
    }
    
    // fill hex gap with spaces if not full line //
    if (len < 16) {
        gap = 16 - len;
        for (i = 0; i < gap; i++) {
            printDebug("   ");                                           
            
        }
    }
    printDebug("   ");                                                      
    
    
    // ascii (if printable) //
    ch = payload;
    for(i = 0; i < len; i++) {
        if (isprint(*ch)){
            printDebug("%c", *ch);                                           
            
        }
        else{
            printDebug(".");                                                 
            
        }
        ch++;
    }

    printDebug("\r\n");                                                        
    return;
    
}
/* ================================================================================= */