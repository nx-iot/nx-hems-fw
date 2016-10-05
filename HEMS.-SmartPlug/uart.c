#include <mega128a.h> 
#include <stdio.h>
#include <stdint.h>
#include <delay.h>
#include <string.h>
#include "uart.h"
#include "debug.h"

// UART 1
uint8_t rx_buffer1[RX_BUFFER_SIZE1];                                       // USART0 Receiver buffer
uint16_t rx_wr_index1, rx_rd_index1, rx_counter1;
bit rx_buffer_overflow1;                                                // This flag is set on USART0 Receiver buffer overflow

// UART 0                                                                        
flash uint8_t charHeader = '$';
flash uint8_t charTerminate = '#';
uint8_t _FlagPackageTerminate0 = 0;
uint8_t rxPackageIndex0;
uint8_t rxPackage0[RX_BUFFER_SIZE0];
uint8_t rxPackageIndexCount0;


/* ================================================================================= */
/*************************************************************************************/
/******************************** UART 0 (Print Debug) *******************************/
/*************************************************************************************/ 
/* ================================================================================= */
// USART0 Receiver interrupt service routine
interrupt [USART0_RXC] void usart0_rx_isr(void) {

    uint8_t status, data;
    status = UCSR0A;
    data = UDR0;
    
    if((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN)) == 0) {
    
        putchar0(data);
        // Detect Package Header
        if(data == charHeader) {
            rxPackageIndex0 = 0;
        }
        
        rxPackage0[rxPackageIndex0++] = data;         
        
        if(rxPackageIndex0 >= RX_BUFFER_SIZE0) { 
            rxPackageIndex0 = 0;                
        }
        
        if(data == charTerminate) {
            rxPackageIndexCount0 = (rxPackageIndex0 - 1);
            rxPackageIndex0 = 0;
            _FlagPackageTerminate0 = 1;
        }
            
    }
    
}
/* ================================================================================= */
// Write a character to the USART0 Transmitter
#pragma used+
void putchar0(uint8_t c) {

    while((UCSR0A & DATA_REGISTER_EMPTY) == 0);
    UDR0 = c;
    
}
#pragma used-
/* ================================================================================= */
/*************************************************************************************/
/*********************************** UART 1 (XBee) ***********************************/
/*************************************************************************************/ 
/* ================================================================================= */ 
// USART1 Receiver interrupt service routine
interrupt [USART1_RXC] void usart1_rx_isr(void) {

    uint8_t status, data;
    status = UCSR1A;
    data = UDR1;
    
    if((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN)) == 0) {
        rx_buffer1[rx_wr_index1++] = data;
         //printDebug("%02X ",data);
        if (rx_wr_index1 == RX_BUFFER_SIZE1) 
            rx_wr_index1 = 0;
        if (++rx_counter1 == RX_BUFFER_SIZE1) {
            printDebug("WARNING : UART0 BUFFER OVERFLOW %d\r\n", rx_counter1);
            rx_counter1 = 0;
            rx_buffer_overflow1 = 1;            
        }
    }
    
}
/* ================================================================================= */ 
#ifndef _DEBUG_TERMINAL_IO_
// Get a character from the USART1 Receiver buffer
#define _ALTERNATE_GETCHAR_       
#pragma used+
char getchar(void) {

    uint8_t data;
    while (rx_counter1 == 0);
    data = rx_buffer1[rx_rd_index1++];
    if(rx_rd_index1 == RX_BUFFER_SIZE1) 
        rx_rd_index1 = 0;
    #asm("cli")
    --rx_counter1;
    #asm("sei")
    return data;
    
}
/* ================================================================================= */ 
#pragma used-
// Write a character to the USART1 Transmitter
#pragma used+
void putchar1(uint8_t c) {

    while((UCSR1A & DATA_REGISTER_EMPTY) == 0);
    UDR1 = c;
    
}
#pragma used-
#endif
/* ================================================================================= */ 
uint16_t read_uart1(uint8_t *readbuf, uint16_t maxread, uint16_t interchar) {
                      
    uint8_t data;  
    uint16_t len = 0;
    
    while(rx_counter1 > 0) {
        data = rx_buffer1[rx_rd_index1++];
        if (rx_rd_index1 == RX_BUFFER_SIZE1) {
            rx_rd_index1 = 0;                                
        }
        UCSR1B &= 0x7F;          
        --rx_counter1;                         
        UCSR1B |= 0x80;              
        memcpy(readbuf++, &data, 1);       
        len++;
        if(len == maxread) {
            break;
        }        
        delay_ms(interchar);
    }
    return len;
    
}
/* ================================================================================= */ 
uint16_t write_uart1(uint8_t writebuf[], uint16_t len) {

    uint16_t i;
    for(i = 0; i < len; i++) {
        putchar1(writebuf[i]);           
    }           
    return i;
    
}    
/* ================================================================================= */ 
void xbee_clearRcvBuffer0(void) {

    memset(rx_buffer1, '\0', RX_BUFFER_SIZE1);
    
}
/* ================================================================================= */
/*************************************************************************************/
/************************************* Alternate *************************************/
/*************************************************************************************/ 
/* ================================================================================= */
void send_uart(uint8_t port, uint8_t *buffer) {

    uint8_t i = 0;
    
    switch(port) {
        case 0: 
            while(buffer[i] != 0) {
                putchar0(buffer[i]);
                i++;
            }
            break;
        case 1: 
            while(buffer[i] != 0) {
                putchar1(buffer[i]);
                i++;
            }
            break;
    }
        
}
/* ================================================================================= */
void init_uart(uint8_t channel, uint32_t baud) {

    switch(channel) {
    case 0:
        // USART0 initialization
        // FOSC = 11.0592 MHz
        // Communication Parameters: 8 Data, 1 Stop, No Parity
        // USART0 Receiver: On
        // USART0 Transmitter: On
        // USART0 Mode: Asynchronous
        // USART0 Baud Rate: 9600
        UCSR0A=0x00;
        UCSR0B=0x98;
        UCSR0C=0x06;
        UBRR0H=0x00;
        switch (baud) {  
        case 2400:
            UBRR0L = 0x1F;
            break;
        case 9600:
            UBRR0L = 0x47;
            break;
        case 14400:
            UBRR0L = 0x2F;
            break;
        case 19200:
            UBRR0L = 0x23;
            break;
        case 38400:
            UBRR0L = 0x11;
            break;    
        case 57600:
            UBRR0L = 0x0B;
            break;    
        case 115200:
            UBRR0L = 0x05;
            break; 
        default:  
            UBRR0L = 0x47;       // default baudrate is 9600
        }      
        break;
    case 1:
        // USART1 initialization
        // FOSC = 11.0592 MHz
        // Communication Parameters: 8 Data, 1 Stop, No Parity
        // USART1 Receiver: On
        // USART1 Transmitter: On
        // USART1 Mode: Asynchronous
        // USART1 Baud Rate: 9600
        UCSR1A=0x00;
        UCSR1B=0x98;
        UCSR1C=0x06;
        UBRR1H=0x00;
        switch ( baud ) { 
        case 2400:
            UBRR1L = 0x1F;
            break;
        case 9600:
            UBRR1L = 0x47;
            break;
        case 14400:
            UBRR1L = 0x2F;
            break;
        case 19200:
            UBRR1L = 0x23;
            break;
        case 38400:
            UBRR1L = 0x11;
            break;    
        case 57600:
            UBRR1L = 0x0B;
            break;    
        case 115200:
            UBRR1L = 0x05;
            break; 
        default:  
            UBRR1L = 0x47;       // default baudrate is 9600
        }     
        break;

    }

}
/* ================================================================================= */







