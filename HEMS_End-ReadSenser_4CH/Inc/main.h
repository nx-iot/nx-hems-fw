#ifndef MAIN_H
#define MAIN_H

#include "stm32f0xx_hal.h"
#include "uart2_manag.h"
#define FIRMWARE_VERSION        1.0
#define TIMEOUT                 5               // Timeout for retransmit

#define TURN_ON                 1
#define TURN_OFF                0

#define NORMAL_MODE             0
#define CURRENT_MONITOR_MODE    1

// Sensor Sensitive mV/A.
#define SENSOR5A            185.0
#define SENSOR20A           100.0
#define SENSOR30A           66.0

#define TYPE_SMART_PLUG     0x11
#define TYPE_SMART_SWITCH   0x12
#define TYPE_SMART_BREAKER  0x13

/*----- Digital Input -----*/
#define SW_BUTTON_DDR       DDRC.0 = 0
#define SW_BUTTON_PORT      PORTC.0
#define SW_BUTTON_PIN       PINC.0

#define D_SW1_DDR           DDRA.7 = 0  
#define D_SW1_PIN           PINA.7
#define D_SW2_DDR           DDRA.6 = 0  
#define D_SW2_PIN           PINA.6
#define D_SW3_DDR           DDRA.5 = 0  
#define D_SW3_PIN           PINA.5
#define D_SW4_DDR           DDRA.4 = 0  
#define D_SW4_PIN           PINA.4

/*----- Digital Output -----*/
#define LED_STAT_DDR        DDRC.7 = 1
#define LED_STAT_PIN        PINC.7       
#define LED_STAT_ON         PORTC.7 = 0         // Active Low
#define LED_STAT_OFF        PORTC.7 = 1

#define POWER_RELAY_DDR    (DDRG |= 0x04)
#define POWER_RELAY_PIN     PING.2        
#define POWER_RELAY_ON     (PORTG |= 0x04)
#define POWER_RELAY_OFF    (PORTG &= 0xFB)
             
#define XBEE_RESET_DDR      DDRE.2 = 1
#define XBEE_RESET_PIN      PINE.2       
#define XBEE_RESET_HIGH     PORTE.2 = 1
#define XBEE_RESET_LOW      PORTE.2 = 0  

#define XBEE_SLEEP_DDR     (DDRG |= 0x08)
#define XBEE_SLEEP_PIN      PING.3        
#define XBEE_SLEEP_HIGH    (PORTG |= 0x08)
#define XBEE_SLEEP_LOW     (PORTG &= 0xF7) 

#define ALL 17
#define Offset 5;

extern uint8_t SWITCH;
extern float CURRENT_VOLT;
extern float CURRENT_AMP;

//extern eeprom float ADJ0_SENSOR5A;
//extern eeprom float ADJ0_SENSOR20A;
//extern eeprom float ADJ0_SENSOR30A;
//extern eeprom uint8_t SAVE_DEVICE_STAT;

typedef struct{
	uint16_t length;
	uint8_t value[255];
}data_t;

typedef struct{
	uint16_t idx_r;	//index read
	uint16_t idx_w;	//index write
	uint16_t size;	//size of data
	uint8_t value[512];
}recv_uart_t;

typedef struct{
	uint8_t ch_id;
	uint8_t ch_status;
	float _volt;
	float _amp;
}ch_dv_f;

void sendStatusReport(void);
void sendDataReport(void);
void manag_sw(void);
void ReadCurrent(void);
void ReadVoltage(void);
void nx_delay_us(uint32_t us);
int16_t getADC(unsigned short CS,unsigned short channel);
void ChipSelect(unsigned short CS,unsigned short status);
void Read_Switch(void);
//void EXTI0_1_IRQHandler_Config(void);
extern recv_uart_t recv_uart_1,recv_uart_2;
extern UART_HandleTypeDef huart2;
extern unsigned char  Read_SW[16];

#endif