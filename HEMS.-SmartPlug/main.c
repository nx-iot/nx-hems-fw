/*****************************************************
Project         : HEMS - Smart_Plug/Smart_Switch/Smart_Breaker
Date            : 13/06/2013
Author          : Tawan Srisang, Sirichakorn
Company         : Intelenics Co.,Ltd.
Comments        : 
Version Format  :

Chip type       : ATmega128
Program type    : Application
Frequency       : 11.059200 MHz
*****************************************************/
#include <mega128a.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <delay.h>
#include <i2c.h>
#include <sleep.h>
#include <io.h>
#include <math.h>
#include "initial_system.h"
#include "int_dataqueue.h"
#include "int_protocol.h"
#include "int_handler.h"
#include "xbeehandler.h"
#include "uart.h"
#include "debug.h"
#include "xbee.h"
#include "timer.h"
#include "adc.h"
#include "node_address.h"

uint8_t SWITCH = TURN_OFF;

float SENSOR_SENSITIVE;
float AMP_ADJ_ZERO;
eeprom float ADJ0_SENSOR5A = 0.090;
eeprom float ADJ0_SENSOR20A = 0.11;
eeprom float ADJ0_SENSOR30A = 0.11;
eeprom uint8_t SAVE_DEVICE_STAT = TURN_OFF;
float CURRENT_VOLT = 0.0;
float CURRENT_AMP = 0.0;

int8_t read_dSwitch() {
    int8_t read1, read2;     
    read1 = D_SW1_PIN | (D_SW2_PIN<<1) | (D_SW3_PIN<<2) | (D_SW4_PIN<<3); 
    delay_ms(100); 
    read2 = D_SW1_PIN | (D_SW2_PIN<<1) | (D_SW3_PIN<<2) | (D_SW4_PIN<<3);
    if(read1 == read2) {
        return read1;
    }else {
        return -1;
    }
}

void main(void) {

    uint8_t error;
    uint8_t joinState;
    TIMER hb_retrytime          = 0;            // For check HeartBeat timeout
    TIMER joinreq_retrytime     = 0;            // For check JoinRequest timeout
    TIMER dataRep_retrytime     = 0;            // For check Data Report timeout
    TIMER statusRep_retrytime   = 0;            // For check Status Report timeout
    uint8_t retryCount;
    uint8_t flag1stRound;  
    int8_t read_D_SW;
    /* specData[8] = DEVICE_TYPE */
    uint8_t specData[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};   // Specific Stucture of Join Req packet
    uint8_t statusRepParam[13]; // Parameter of status report
    
    // -- Electrical Variable -- //
    float total     = 0.0;
    float avg       = 2500.0;
    float value     = 0.0;
    float Viout     = 0.0; 
    float Vdif      = 0.0;
    float Vsq_avg   = 0.0;
    float volt      = 0.0;
    float amp       = 0.0;
    float power     = 0.0;
    float whour     = 0.0;
    float Vsum      = 0.0;
    float Isum      = 0.0;
    float Psum      = 0.0;
    float WHsum     = 0.0;
    float Vavg      = 0.0;
    float Iavg      = 0.0;
    float Pavg      = 0.0; 
    uint16_t countSampling  = 0;
    uint16_t adcValue       = 0; 
    uint16_t number         = 0;                       
    
    uint32_t timeStamp = 0;
    //char resParam[20];
    
    //char param[]  = {0xD0, 0xB0, 0xA0, 0xA1}; 
    char stat_on[]  = {0x01, 0x00, 0x00, 0x00};
    char stat_off[]  = {0x00, 0x00, 0x00, 0x00};
    DATASET data[4];
  
    //////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////
     
    /*=============== System Initialize ===============*/
    do{ error = initial_system(); }while(error); 
    printDebug("\r\n++++++++++ Nathapong ++++++++++\r\n"); 
    
    /*=============== Buffer Initialize ===============*/   
    xbee_clearRcvBuffer0();
    INT_initWaitQueue();   
    
    /*=============== Select Device Type ===============*/
   
    do { 
        read_D_SW = read_dSwitch(); 
        if(read_D_SW == 0x0F) {
            SENSOR_SENSITIVE = SENSOR5A;
            if((ADJ0_SENSOR5A > 0.0) && (ADJ0_SENSOR5A < 0.12)) {
                AMP_ADJ_ZERO = ADJ0_SENSOR5A;
            }else {AMP_ADJ_ZERO = 0.09;}
            specData[8] = TYPE_SMART_SWITCH;                             // Device Type  
            printDebug("\r\n++++++++++ Smart Switch ++++++++++\r\n");
            printDebug("\r\nFirmware Version : %0.1f\r\n", FIRMWARE_VERSION); 
            printDebug("Current Sensor   : ACS712ELCTR-05B-T (%0.1f mV/Amp.)\r\n", SENSOR_SENSITIVE); 
        }else if(read_D_SW == 0x0E) {
            SENSOR_SENSITIVE = SENSOR20A;
            if((ADJ0_SENSOR20A > 0.0) && (ADJ0_SENSOR20A < 0.14)) {
                AMP_ADJ_ZERO = ADJ0_SENSOR20A;
            }else {AMP_ADJ_ZERO = 0.11;}
            specData[8] = TYPE_SMART_PLUG;                               // Device Type
            printDebug("\r\n++++++++++ Smart Plug ++++++++++\r\n");
            printDebug("\r\nFirmware Version : %0.1f\r\n", FIRMWARE_VERSION);  
            printDebug("Current Sensor   : ACS712ELCTR-20A-T (%0.1f mV/Amp.)\r\n", SENSOR_SENSITIVE);  
        }else if(read_D_SW == 0x0D) {
            SENSOR_SENSITIVE = SENSOR30A;
            if((ADJ0_SENSOR30A > 0.0) && (ADJ0_SENSOR30A < 0.15)) {
                AMP_ADJ_ZERO = ADJ0_SENSOR30A;
            }else {AMP_ADJ_ZERO = 0.12;}
            specData[8] = TYPE_SMART_BREAKER;                            // Device Type
            printDebug("\r\n++++++++++ Smart Breaker ++++++++++\r\n");
            printDebug("\r\nFirmware Version : %0.1f\r\n", FIRMWARE_VERSION); 
            printDebug("Current Sensor   : ACS712ELCTR-30A-T (%0.1f mV/Amp.)\r\n", SENSOR_SENSITIVE);  
        }else if(read_D_SW < 0) {
            printDebug("Read Dip-Switch ERROR!\r\n");
        }else {
            SENSOR_SENSITIVE = SENSOR5A;
            AMP_ADJ_ZERO = ADJ0_SENSOR5A;
            specData[8] = TYPE_SMART_SWITCH; 
            printDebug("\r\n++++++++++ Default Type : Smart Switch ++++++++++\r\n");
            printDebug("\r\nFirmware Version : %0.1f\r\n", FIRMWARE_VERSION);
            printDebug("Current Sensor   : ACS712ELCTR-05B-T (%0.1f mV/Amp.)\r\n", SENSOR_SENSITIVE); 
        }
    }while(read_D_SW < 0);  
    
    /*=============== Current Measurement ===============*/
    printDebug("Current Measure  : > %0.2f Amp.\r\n", AMP_ADJ_ZERO);
    
    /*=============== Read Node Address ===============*/ 
    
    //do{ error = nodeAddress_write(SENDER_ADDRESS); }while(error);
    //do{ error = nodeAddress_read(SENDER_ADDRESS); }while(error);
    //memcpy(&specData[0], &SENDER_ADDRESS[0], 8);
    
    /*=============== ADC Vref ===============*/
//    printDebug("ADC Vref         : %0.2f Volt.\r\n", _adcVref);
    
    /*=============== Working Mode ===============*/ 
//    if(WORKING_MODE == CURRENT_MONITOR_MODE) {
//        printDebug("Working Mode     : Current Monitoring Mode\r\n");
//    }else {
//        printDebug("Working Mode     : Normal Mode\r\n");
//    }
    
    #asm("sei")    // Global enable interrupts  
    
    printDebug("\r\n-------- Initial Complete --------\r\n");
    delay_ms(5000);
    
    /*=============== Set Present on/off Status from Previous on/off status ===============*/
    if(SAVE_DEVICE_STAT == TURN_ON) {
        SWITCH = TURN_ON; 
        POWER_RELAY_ON;
        printDebug("SWITCH ON!\r\n");
    }else {
        SWITCH = TURN_OFF; 
        POWER_RELAY_OFF;
        printDebug("SWITCH OFF!\r\n");
    }
     
    /*=============== CURRENT_MONITOR_MODE ===============*/
//
//        while(1) { 
//            
//            #asm("wdr");
//           
//            /*=============== ADC1 Sampling every 1 ms. ===============*/
//            if(_Flag0001INT && (SWITCH == TURN_ON)) {
//             
//                _Flag0001INT = 0;
//                adcValue = read_adc(ADC1); 
//                //printDebug("ADC = %d\r\n", adcValue);
//                value = adcValue * ((_adcVref*1000) / 1023.0);
//                avg = (499.0*avg + value) / 500.0;
//                
//                if(value > avg) {
//                    Vdif = value - avg;
//                    total += (Vdif*Vdif);              
//                }else if(value < avg) { 
//                    Vdif = avg - value;
//                    total += (Vdif*Vdif);
//                }
//                countSampling++;                        
//              
//            }
//              
//            /*=============== Electrical Measurement ===============*/
//            if(_Flag05INT) { 
//                
//                _Flag05INT = 0; 
//                
//                printDebug("%d   ", countSampling);
//                
//                /*---------- Voltage ----------*/
//                adcValue = read_adc(ADC0);
//                //printDebug("%d   ", adcValue);
//                volt = (((adcValue * _adcVref)/1023.0)/0.01) + 9.0;
//                printDebug("%0.4f   ", volt);
//                Vsum += volt;
//                        
//                /*---------- Current ----------*/
//                Vsq_avg = total / countSampling;
//                Viout = sqrt(Vsq_avg);
//                amp = Viout / SENSOR_SENSITIVE;
//                
//                //printDebug("%0.4f   ", value); 
//                //printDebug("%0.4f   ", avg);
//                //printDebug("%d   ", countSampling);
//                //printDebug("%0.4f   ", total);
//                //printDebug("%0.4f   ", Viout);
//                printDebug("%0.4f   ", amp);
//                
//                /* Adjust Current to 0 */ 
//                if(amp < AMP_ADJ_ZERO) {
//                    amp = 0.0;                                            
//                }
//    //            if((SENSOR_SENSITIVE == SENSOR5A) && (amp < 0.060)) {
//    //                amp = 0.0;  
//    //            }else if((SENSOR_SENSITIVE == SENSOR20A) && (amp < 0.080)) {
//    //                amp = 0.0;
//    //            }else if((SENSOR_SENSITIVE == SENSOR30A) && (amp < 0.10)) {
//    //                amp = 0.0;
//    //            }
//                
//                printDebug("%0.4f   ", amp); 
//                
//                total = 0.0;
//                countSampling = 0;
//                Isum += amp;
//                        
//                /*---------- Power ----------*/
//                power = volt*amp;
//                printDebug("%0.4f   ", power);
//                Psum += power;
//                        
//                /*----------  Watt-hour ----------*/
//                whour = power*(0.5/3600.0);
//                printDebug("%0.4f\r\n", whour);
//                WHsum += whour;
//                number++;        
//                //printDebug("------------------------------------------\r\n");
//                        
//            }
//            
//            /*=============== Check Button Switch ===============*/
//            if(SW_BUTTON_PIN == 0) {
//                _FlagBT = 1;
//                                
//            }else{
//                _FlagBT = 0;
//            } 
//            if((SW_BUTTON_PIN == 1) && (pressedBTCounter != 0)) {
//                //printDebug("\r\nCounter = %d\r\n", pressedBTCounter);
//                if(pressedBTCounter < 6) {
//                    if(SWITCH == TURN_OFF) { 
//                        SWITCH = TURN_ON;         
//                        POWER_RELAY_ON;
//                        LED_STAT_ON;
//                        SAVE_DEVICE_STAT = TURN_ON;
//                        printDebug("SWITCH ON!\r\n"); 
//                    }else {  
//                        SWITCH = TURN_OFF;                         
//                        POWER_RELAY_OFF;
//                        LED_STAT_OFF;
//                        SAVE_DEVICE_STAT = TURN_OFF;
//                        printDebug("SWITCH OFF!\r\n");
//                    }   
//                }
//                pressedBTCounter = 0;    
//            }
//            
//            /*=============== Received Debug Request ===============*/
//            if(_FlagPackageTerminate0) {    
//                debug_req();     
//            }
//              
//        }
    
    /*=============== Pairing Operation ===============*/ 
    START:
        
    _xbee_Assoc = 0xFF;     // 0x00 = Successfully joined a network       
    _xbee_chkAsso_res = 0;
        
    flag1stRound = 1;
    hb_retrytime = 0; 
    joinreq_retrytime = 0;
    dataRep_retrytime = 0;
    statusRep_retrytime = 0;
    number = 0;
    Vsum = 0;
    Isum = 0;
    Psum = 0;
    WHsum = 0;
        
    joinState = 1;
    retryCount = 0;
    _pired = 0;
     xbee_readSerialNumber();
    do {
        #asm("wdr");
        switch(joinState) {
            case 1:
                printDebug("PAIRING OPERATION> Start..\r\n");
                memcpy(&specData[0], &SENDER_ADDRESS[0], 8);
                xbee_checkAssociation();
                _BlinkLED_1Hz = 1;
                joinState = 2;
                break;
            case 2:                  
                if(!_xbee_join) { 
                    if(_xbee_chkAsso_res && _xbee_Assoc) { 
                        xbee_checkAssociation();
                    }
                    xbee_receivePacket();     // Process Xbee Data UART0        
                    //delay_ms(1000);
                }else {                
                    printDebug("PAIRING OPERATION> XBee Joined.\r\n");
                    joinState = 3;
                    break;
                }
                break;
            case 3:
                if(_xbee_join) {
                    /*-------- Received HeartBeat Ack --------*/
                    xbee_receivePacket();                                     // Process Xbee Data UART0 
                    if(_hb_res) {
                        if(_hb_stat == 0x00) {                                // Ack OK -> Paired 
                            joinState = 7;
                            _hb_res = 0;
                            _hb_stat = 0xFF;
                            break;
                        }else if(_hb_stat == 0x01) {                          // Ack Kill -> Leave Network
                            xbee_leaveNetwork();
                            delay_ms(100);
                            joinState = 1;
                            _hb_res = 0;
                            _hb_stat = 0xFF;
                            retryCount = 0;
                            break;    
                        }
                    }
                    /*-------- Send HeartBeat Cmd --------*/ 
                    if(TIMER_checkTimerExceed(hb_retrytime) && !_hb_res) {
                        if(retryCount >= 3) {
                            xbee_leaveNetwork();
                            retryCount = 0;
                            joinState = 1;
                            break;
                        }else {
                            printDebug("RETRANSMIT %d times\r\n", retryCount);
                            INT_sendNetworkMaintenanceCmd(HEARTBEAT, NULL, 0, 0);      
                            TIMER_setTimer(&hb_retrytime, 10);
                            retryCount++;
                        }
                    }                
                }else {
                    joinState = 1;
                    break;   
                } 
                break;
            case 4:
                xbee_leaveNetwork();
                delay_ms(5);
                xbee_checkAssociation();
                joinState = 5;
                break;
            case 5:  
                if(!_xbee_join) { 
                    if (_xbee_chkAsso_res && _xbee_Assoc){ 
                        xbee_checkAssociation();
                    }
                    xbee_receivePacket();    // Process Xbee Data UART0      
                    //delay_ms(1000);
                }else {    
                    printDebug("PAIRING OPERATION> Found new parent.\r\n");
                    delay_ms(1000);
                    joinState = 6;
                    break;
                } 
                break;
            case 6:
                if(_xbee_join) {
                    /*-------- Received Join Request Ack --------*/
                    xbee_receivePacket();                                      // Process Xbee Data UART0
                    if(_joinreq_res) {
                        if(_joinreq_stat == 0x00) {                            // Ack Accepted -> Paired 
                            joinState = 7;
                            _joinreq_res = 0;
                            _joinreq_stat = 0xFF;
                            break;
                        }else if(_joinreq_stat == 0x01) {                      // Ack Deny -> Leave Network
                            xbee_leaveNetwork();
                            delay_ms(100);
                            retryCount = 0;
                            joinState = 1;
                            _joinreq_res = 0;
                            _joinreq_stat = 0xFF;
                            break;    
                        }
                    }
                    /*-------- Send Join Request Cmd --------*/
                    if(TIMER_checkTimerExceed(joinreq_retrytime) && !_joinreq_res) {
                        if(retryCount >= 3) {
                            retryCount = 0;
                            joinState = 4;
                            break;
                        }else {
                            printDebug("RETRANSMIT %d times\r\n", retryCount);
                            INT_sendNetworkMaintenanceCmd(JOINREQ, specData, sizeof(specData), 0);       
                            TIMER_setTimer(&joinreq_retrytime, 10);
                            retryCount++;
                        }
                    } 
                }else {
                    joinState = 1; 
                    break;   
                }
                break;
            case 7:
                printDebug("PAIRING OPERATION> Pairing Success.\r\n");
                _pired = 1; 
                _BlinkLED_1Hz = 0;
                LED_STAT_ON;
                joinState = 0;
                delay_ms(3000);
                LED_STAT_OFF;
                delay_ms(500);
                if(SWITCH == TURN_ON) {
                   LED_STAT_ON;
                }else {LED_STAT_OFF;} 
                break;
                     
            default :
                break; 
                
        }
            
        /*=============== Received Debug Request ===============*/
        if(_FlagPackageTerminate0) {    
            debug_req();     
        }
                
        /*=============== Check Button Switch ===============*/
        if(SW_BUTTON_PIN == 0) {
            _FlagBT = 1;
                            
        }else {
            _FlagBT = 0;
        }           
        if((SW_BUTTON_PIN == 1) && (pressedBTCounter != 0)) {
            printDebug("%d  ", pressedBTCounter);
            if((joinState == 3) && (pressedBTCounter >= 6)) {
                printDebug("PAIRING OPERATION> Pressed Button.\r\n");
                delay_ms(1000);
                retryCount = 0;
                joinState = 4;     
            }else { 
                if(SWITCH == TURN_OFF) {
                    SWITCH = TURN_ON; 
                    POWER_RELAY_ON; 
                    SAVE_DEVICE_STAT = TURN_ON;
                    printDebug("SWITCH ON!\r\n");
                }else { 
                    SWITCH = TURN_OFF;
                    POWER_RELAY_OFF; 
                    SAVE_DEVICE_STAT = TURN_OFF;
                    printDebug("SWITCH OFF!\r\n");
                }   
            }
            pressedBTCounter = 0;    
        }
              
    }while(joinState > 0);
        
    while(1) {
        /*================================================================================================*/
        /*======================================== WORK IN ONLINE ========================================*/
        /*================================================================================================*/
        #asm("wdr");
        if(_xbee_join) {
             
            /*---------- Received Debug Request ----------*/
            if(_FlagPackageTerminate0) {    
                debug_req();     
            }
                
            /*---------- Check Button Switch ----------*/
            if(SW_BUTTON_PIN == 0) {
                _FlagBT = 1;                
            }else {
                _FlagBT = 0;
            } 
            if((SW_BUTTON_PIN == 1) && (pressedBTCounter != 0)) {
                //printDebug("\r\nCounter = %d\r\n", pressedBTCounter);
                if(pressedBTCounter < 6) {
                    if(SWITCH == TURN_OFF) {
                        SWITCH = TURN_ON;
                        POWER_RELAY_ON;
                        LED_STAT_ON;  
                        SAVE_DEVICE_STAT = TURN_ON;
                        printDebug("SWITCH ON!\r\n"); 
                        INT_sendEventReportCmd(timeStamp, 0x20, 0xA0, stat_on, TIMEOUT);        // Event output high
                    }else { 
                        SWITCH = TURN_OFF;
                        POWER_RELAY_OFF;
                        LED_STAT_OFF;
                        CURRENT_VOLT = 0.0;
                        CURRENT_AMP = 0.0;
                        SAVE_DEVICE_STAT = TURN_OFF;
                        printDebug("SWITCH OFF!\r\n");
                        INT_sendEventReportCmd(timeStamp, 0x20, 0xA0, stat_off, TIMEOUT);        // Event output low
                    }   
                }
                pressedBTCounter = 0;    
            }
                
            /*---------- ADC1 Sampling (AC Current Sensor) every 1 ms. ----------*/
            if(_Flag0001INT) {
                 
                _Flag0001INT = 0;
                adcValue = read_adc(ADC1); 
                //printDebug("ADC = %d\r\n", adcValue);
                value = adcValue * (5000.0 / 1023.0);
                    
                // Keep track of the moving average 
                // See more : http://jeelabs.org/2011/09/15/power-measurement-acs-code/
                avg = (499.0*avg + value) / 500.0;
                    
                if(value > avg) {
                    Vdif = value - avg;
                    total += (Vdif*Vdif);              
                }else if(value < avg) { 
                    Vdif = avg - value;
                    total += (Vdif*Vdif);
                }
                countSampling++;                        
                  
            }
                  
            /*---------- Electrical Measurement ----------*/
            if(_Flag05INT) { 
                    
                _Flag05INT = 0;
                /*---------- Voltage ----------*/
                adcValue = read_adc(ADC0);
                //printDebug("%d   ", adcValue);
                volt = (((adcValue*5.0)/1023.0)/0.01); 
                if(volt < 223.0) { 
                    volt += 9.0;
                }else if(volt > 233.0) {
                    volt -= 4.0;
                }
                //printDebug("Volt = %f\r\n", volt);
                Vsum += volt;
                            
                /*---------- Current ----------*/
                // V-rms
                // See more : http://www.electronics-tutorials.ws/blog/rms-voltage.html
                Vsq_avg = total / countSampling;
                Viout = sqrt(Vsq_avg);
                amp = Viout / SENSOR_SENSITIVE;                  // ACS712 +-5 or +-20 or +-30 Amp.
                     
                /* Adjust Current to 0 */
                if(amp < AMP_ADJ_ZERO) {
                    amp = 0.0;                                            
                } 
                    
                total = 0.0;
                countSampling = 0;
                Isum += amp;
                            
                /*---------- Power ----------*/
                power = volt*amp;
                //printDebug("Power = %f\r\n", power);
                Psum += power;
                            
                /*----------  Watt-hour ----------*/
                whour = power*(0.5/3600.0);
                //printDebug("Watt-Hour = %f\r\n", whour);
                WHsum += whour;
                number++;        
                //printDebug("------------------------------------------\r\n");
                            
            }
                
            /*==================== NETWORK COMMUNICATION ====================*/
                
            /*---------- DATA_REPORT_CMD ----------*/
            if(flag1stRound) {
                flag1stRound = 0;     
                TIMER_setTimer(&dataRep_retrytime, 30);
            }else {
                if(TIMER_checkTimerExceed(dataRep_retrytime)) {
                    
                    if(SWITCH == TURN_ON) {
                        Vavg = Vsum/number;      // Voltage
                        Iavg = Isum/number;      // Current
                        Pavg = Psum/number;      // Power 
                            
                        CURRENT_VOLT = Vavg;
                        CURRENT_AMP = Iavg; 
                            
                        printDebug("\r\n======================================================\r\n");
                        printDebug("Vsum = %0.4f  ", Vsum); printDebug("Isum = %0.4f  ", Isum); printDebug("Psum = %0.4f\r\n", Psum);
                        printDebug("Vavg = %0.4f   ", Vavg); printDebug("Iavg = %0.4f   ", Iavg); printDebug("Pavg = %0.4f\r\n", Pavg);
                        printDebug("Watt-Hour Sum = %0.4f\r\n", WHsum);
                        printDebug("Number Sampling = %d\r\n", number);
                        printDebug("======================================================\r\n\r\n");
                            
                        data[0].dataID = 0x01;
                        data[0].dataType = VOLT;
                        data[0].value = Vavg;
                        data[1].dataID = 0x01;
                        data[1].dataType = AMP;
                        data[1].value = Iavg*1000.0;    // mA
                        data[2].dataID = 0x01;
                        data[2].dataType = PWR;
                        data[2].value = Pavg;
                        data[3].dataID = 0x01;
                        data[3].dataType = KWH;
                        data[3].value = WHsum;
                            
                        INT_sendDataReportCmd(timeStamp, 0x20, 0x04, data, TIMEOUT); 
                             
                        /* Safety Current Sensor */
                        if((SENSOR_SENSITIVE == SENSOR5A) && (Iavg > 4.9)) {
                            SWITCH = TURN_OFF;
                            POWER_RELAY_OFF;
                            LED_STAT_OFF;
                            CURRENT_VOLT = 0.0;
                            CURRENT_AMP = 0.0;
                            printDebug("Current Exceed --SWITCH OFF!\r\n");
                            INT_sendEventReportCmd(timeStamp, 0x20, 0xA0, stat_off, TIMEOUT);        // Event output low 
                        }else if((SENSOR_SENSITIVE == SENSOR20A) && (Iavg > 19.9)) {
                            SWITCH = TURN_OFF;
                            POWER_RELAY_OFF;
                            LED_STAT_OFF;
                            CURRENT_VOLT = 0.0;
                            CURRENT_AMP = 0.0;
                            printDebug("Current Exceed --SWITCH OFF!\r\n");
                            INT_sendEventReportCmd(timeStamp, 0x20, 0xA0, stat_off, TIMEOUT);        // Event output low 
                        }else if((SENSOR_SENSITIVE == SENSOR30A) && (Iavg > 29.9)) {
                            SWITCH = TURN_OFF;
                            POWER_RELAY_OFF;
                            LED_STAT_OFF;
                            CURRENT_VOLT = 0.0;
                            CURRENT_AMP = 0.0;
                            printDebug("Current Exceed --SWITCH OFF!\r\n");
                            INT_sendEventReportCmd(timeStamp, 0x20, 0xA0, stat_off, TIMEOUT);        // Event output low 
                        }
                            
                        /* Reset value */
                        number = 0;
                        Vsum = 0.0;
                        Isum = 0.0; 
                        Psum = 0.0;
                        WHsum = 0.0;
                            
                    }       
                    TIMER_setTimer(&dataRep_retrytime, 60);
                }
            }
                   
            /*---------- STATUS_REPORT_CMD ----------*/ 
            if(TIMER_checkTimerExceed(statusRep_retrytime)) {
                
                statusRepParam[0] = SWITCH;
                statusRepParam[1] = 0x00;
                statusRepParam[2] = 0x00;
                statusRepParam[3] = 0x00;
                statusRepParam[4] = 0x00;                    
                memcpy(&statusRepParam[5], &CURRENT_VOLT, 4);          
                memcpy(&statusRepParam[9], &CURRENT_AMP, 4);      
                        
                INT_sendStatusReportCmd(timeStamp, 0x20, statusRepParam, 13, TIMEOUT);   
                TIMER_setTimer(&statusRep_retrytime, 60);
            }
                
            /*---------- RECEIVE AND RETRANSMIT PACKET ----------*/
            INT_retransmit();
            xbee_receivePacket();
                    
        }else {
            goto START;    
        }
            
    }
    
}


