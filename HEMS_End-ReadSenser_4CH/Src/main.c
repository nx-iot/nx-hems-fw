/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal.h"
#include "main.h"
#include "print_debug.h"
#include "xbee.h"
#include "xbeehandler.h"
#include "protocol.h"
#include "handler.h"
#include <math.h>
//#include "stm32f0xx_it.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;


data_t data_gw_xbee;
ch_dv_f data_status[4];

recv_uart_t recv_uart_1,recv_uart_2;
/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);

extern int FlagReadCurrent;
extern int FlagReadVolt ;
extern int countSampling;
extern int FlagSendData;
extern int number; 
extern int FlagSendStatus; 
extern int StatusSampling;



/******* Global *************/
	int status_sw = 0;
	int event = 0;
	uint32_t timeStamp = 0;
	char stat_on[]  = {0x01, 0x00, 0x00, 0x00};
	char stat_off[]  = {0x00, 0x00, 0x00, 0x00};
	int8_t ret = 0;
	int joinState = 1;
	int ret_SN = 0;
	uint8_t specData[13] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xFF, 0xFF, 0xFF, 0xFF};   // Specific Stucture of Join Req packet	
  uint8_t statusRepParam[13]; // Parameter of status report
  DATASET data[16];
	float total[]     = {0.0,0.0,0.0,0.0};
	float avg[]       = {2500,2500,2500,2500};
	float value     = 0.0;
	float Viout     = 0.0; 
	float Vdif      = 0.0;
	float Vsq_avg   = 0.0;
	float volt[]    = {0.0,0.0,0.0,0.0};
	float amp[]     = {0.0,0.0,0.0,0.0};
	float power[]   = {0.0,0.0,0.0,0.0};
	float whour[]   = {0.0,0.0,0.0,0.0};
	float Vsum[]    = {0.0,0.0,0.0,0.0};
	float Isum[]    = {0.0,0.0,0.0,0.0};
	float Psum[]    = {0.0,0.0,0.0,0.0};
	float WHsum[]   = {0.0,0.0,0.0,0.0};
	float Vavg      = 0.0;
	float Iavg      = 0.0;
	float Pavg      = 0.0; 
//	float Vavg[4];
//	float Iavg[4];
//	float Pavg[4]; 
	int Ret = 0;
	unsigned char  Read_SW[16];
	uint8_t *dataRe;
	uint16_t *lenRe; 
	int ChckADCMax =0;

	uint16_t adcValue       = 0; 
	//uint16_t number         = 0;
	
	uint8_t SWITCH;
	float CURRENT_VOLT;
	float CURRENT_AMP;
/******* Global *************/

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
	print_init(&huart1);
	__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);
	
	ChipSelect(ALL,1);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,0);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,1);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,0);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,1);
	
	
	 //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	xbee_flush();
	print_debug(0,"Nathapoong\r\n");
	while(joinState != 5)
	{
		xbee_flush();
		switch(joinState) {
			case 1:
					print_debug(0,"PAIRING OPERATION> Start..\r\n");
					print_debug(0,"PAIRING OPERATION> Check Serial Number..\r\n");
					xbee_readSerialNumber();
					ret_SN = xbee_getserial();
					if(ret_SN == 1){
						memcpy(&specData[0], &SENDER_ADDRESS[0], 8);
						joinState = 2;
					}
					break;
			case 2:
				if(_xbee_chkAsso_res != 1){
					print_debug(0,"PAIRING OPERATION> Waiting Association..\r\n");
					xbee_checkAssociation();
					HAL_Delay(1000);
					xbee_receivePacket();
				}
				else {
					if(_xbee_join !=1){
						print_debug(0,"PAIRING OPERATION> Xbee Association..\r\n");
						xbee_checkAssociation();
					}
					else {
						print_debug(0,"PAIRING OPERATION> Xbee Joined..\r\n");
						joinState = 3;
					}
					
				}
				break;
			case 3:
						INT_sendNetworkMaintenanceCmd(HEARTBEAT, NULL, 0, 0);
						HAL_Delay(1000);
						xbee_receivePacket();
						if(_hb_res){
							print_debug(0,"PAIRING OPERATION> Xbee Heartbeat OK ..\r\n");
							HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
							joinState = 4;
						}
						break;		
				case 4:
						INT_sendNetworkMaintenanceCmd(JOINREQ,(char*)specData,sizeof(specData),0);
						HAL_Delay(1000);
						xbee_receivePacket();
						if(_joinreq_res){
							print_debug(0,"PAIRING OPERATION> Xbee Join OK ..\r\n");
							HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
							joinState = 5;
							//manag_sw();
							event = 1;
						}
						break;
		}		
	}
	
	
	HAL_TIM_Base_Init(&htim3);
	HAL_TIM_Base_Start_IT(&htim3);
	//event = 1;
	while(event == 1)
	{
		/*---------- ADC0 Sampling (AC Current Sensor) every 1 ms. ----------*/
		if(FlagReadCurrent)
		{
			ReadCurrent();
		}
		
		/*---------- ADC1 Sampling (AC Voltage Sensor) every 1s. ----------*/
		if(FlagReadVolt)
		{
			ReadVoltage();
			xbee_receivePacket();
			
		}
		/*---------- Send Status every 10s. ----------*/
		if(FlagSendStatus)
		{
			Read_Switch();
			sendStatusReport();
			FlagSendStatus = 0;
			StatusSampling = 0;
		}
		/*---------- SendData every 20s. ----------*/
		if(FlagSendData)
		{
			HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_9);
		
			print_debug(0,"Mode DataReport \r\n");
			sendDataReport();  					/*****sendDataReport*******/


			FlagSendData=0;
			number=0;
		
		}
//			print_debug(0,"Mode Event\r\n");
//			if(status_sw == TURN_ON){
//				print_debug(0,"Mode DataReport \r\n");
//				sendDataReport();  					/*****sendDataReport*******/
//				HAL_Delay(1000);
//				xbee_receivePacket();
//			}
//			print_debug(0,"Mode StatusReport \r\n");
//				sendStatusReport();						/*****sendStatusReport*******/
//				HAL_Delay(1000);
//				xbee_receivePacket();
					//while(1);
//		Ret = xbee_read(dataRe,&lenRe);
//		//print_debug(0,"Ret ------------> %d\r\n",Ret);
//		if(Ret==1)
//		{
//			if(lenRe>0)
//			{
//				print_payload(0, dataRe, sizeof(dataRe));
//			}
//		}

	}
	
		
}
void sendDataReport(){
//	for(int i=0;i<4;i++)
//	{
//		Vavg[i] = Vsum[i]/number;      // Voltage
//		Iavg[i] = Isum[i]/number;      // Current
//		Pavg[i] = Psum[i]/number;      // Power 
//				
//		CURRENT_VOLT = Vavg[i];
//		CURRENT_AMP = Iavg[i]; 

//		print_debug(0,"\r\n======================================================\r\n");
//		print_debug(0,"Relay CH = %d\r\n", i+1);
//		//print_debug(0,"Vsum = %0.4f  ", Vsum[i]); print_debug(0,"Isum = %0.4f  ", Isum[i]); print_debug(0,"Psum = %0.4f\r\n", Psum[i]);
//		print_debug(0,"Vavg = %0.4f   ", Vavg[i]); print_debug(0,"Iavg = %0.4f   ", Iavg[i]); print_debug(0,"Pavg = %0.4f\r\n", Pavg[i]);
//		print_debug(0,"Watt-Hour Sum = %0.4f\r\n", WHsum[i]);
//		print_debug(0,"Number Sampling = %d\r\n", number);
//		print_debug(0,"======================================================\r\n\r\n");
//		
//		
//		Vsum[i] = 0;
//		Isum[i] = 0;
//		Psum[i] = 0;
//				//WHsum = 0;  

//	}
//	
//	data[0].dataID = 1;
//	data[0].dataType = VOLT;
//	data[0].value = Vavg[0];
//	data[1].dataID = 1;
//	data[1].dataType = AMP;
//	data[1].value = Iavg[0]*1000.0;    // mA
//	data[2].dataID = 1;
//	data[2].dataType = PWR;
//	data[2].value = Pavg[0];
//	data[3].dataID = 1;
//	data[3].dataType = KWH;
//	data[3].value = WHsum[0];
//	
//	data[4].dataID = 2;
//	data[4].dataType = VOLT;
//	data[4].value = Vavg[1];
//	data[5].dataID = 2;
//	data[5].dataType = AMP;
//	data[5].value = Iavg[1]*1000.0;    // mA
//	data[6].dataID = 2;
//	data[6].dataType = PWR;
//	data[6].value = Pavg[1];
//	data[7].dataID = 2;
//	data[7].dataType = KWH;
//	data[7].value = WHsum[1];
//	
//	data[8].dataID = 3;
//	data[8].dataType = VOLT;
//	data[8].value = Vavg[2];
//	data[9].dataID = 3;
//	data[9].dataType = AMP;
//	data[9].value = Iavg[2]*1000.0;    // mA
//	data[10].dataID = 3;
//	data[10].dataType = PWR;
//	data[10].value = Pavg[2];
//	data[11].dataID = 3;
//	data[11].dataType = KWH;
//	data[11].value = WHsum[2];
//	
//	data[12].dataID = 4;
//	data[12].dataType = VOLT;
//	data[12].value = Vavg[3];
//	data[13].dataID = 4;
//	data[13].dataType = AMP;
//	data[13].value = Iavg[3]*1000.0;    // mA
//	data[14].dataID = 4;
//	data[14].dataType = PWR;
//	data[14].value = Pavg[3];
//	data[15].dataID = 4;
//	data[15].dataType = KWH;
//	data[15].value = WHsum[3];
//			
//	INT_sendDataReportCmd(timeStamp, 0x00, 0x10, data, TIMEOUT);
//	HAL_Delay(1000);
//	xbee_receivePacket();
	
	
	for(int i=0;i<4;i++)
	{
		if(Read_SW[i]==1)
		{
			Vavg = Vsum[i]/number;      // Voltage
			Iavg = Isum[i]/number;      // Current
			Pavg = Psum[i]/number;      // Power 
					
			CURRENT_VOLT = Vavg;
			CURRENT_AMP = Iavg; 

			print_debug(0,"\r\n======================================================\r\n");
	//		print_debug(0,"Vsum = %0.4f  ", Vsum[i]); print_debug(0,"Isum = %0.4f  ", Isum[i]); print_debug(0,"Psum = %0.4f\r\n", Psum[i]);
			print_debug(0,"Vavg = %0.4f   ", Vavg); print_debug(0,"Iavg = %0.4f   ", Iavg); print_debug(0,"Pavg = %0.4f\r\n", Pavg);
			print_debug(0,"Watt-Hour Sum = %0.4f\r\n", WHsum[i]);
			print_debug(0,"Number Sampling = %d\r\n", number);
			print_debug(0,"======================================================\r\n\r\n");
			
			data[0].dataID = i+1;
			data[0].dataType = VOLT;
			data[0].value = Vavg;
			data[1].dataID = i+1;
			data[1].dataType = AMP;
			data[1].value = Iavg*1000.0;    // mA
			data[2].dataID = i+1;
			data[2].dataType = PWR;
			data[2].value = Pavg;
			data[3].dataID = i+1;
			data[3].dataType = KWH;
			data[3].value = WHsum[i];
					
			INT_sendDataReportCmd(timeStamp, 0x00, 0x04, data, TIMEOUT);
			
			Vsum[i] = 0;
			Isum[i] = 0;
			Psum[i] = 0;
					//WHsum = 0;  
			HAL_Delay(1000);
			xbee_receivePacket();
		}
		
	}
				
			
}
void sendStatusReport(void){
	/*---------- STATUS_REPORT_CMD ----------*/ 
			for(int i=0;i<4;i++)
			{
				CURRENT_VOLT = Vsum[i]/number;      // Voltage
				CURRENT_AMP = Isum[i]/number;      // Current
				
				data_status[i].ch_id = i+1;
				data_status[i].ch_status = Read_SW[i];
				data_status[i]._volt = CURRENT_VOLT;
				data_status[i]._amp = CURRENT_AMP;
				
				
				print_debug(0,"\r\n======================================================\r\n");
				print_debug(0,"CH ID = Relay CH %d   \r\n", data_status[i].ch_id); 
				print_debug(0,"Status = %x   \r\n", data_status[i].ch_status); 
				print_debug(0,"CURRENT_VOLT = %0.4f  \r\n", CURRENT_VOLT); 
				print_debug(0,"CURRENT_AMP = %0.4f   \r\n", CURRENT_AMP); 
				print_debug(0,"======================================================\r\n\r\n");
				
				statusRepParam[0] = data_status[i].ch_status;  
				statusRepParam[1] = data_status[i].ch_id;
				statusRepParam[2] = 0x00;
				statusRepParam[3] = 0x00;
				statusRepParam[4] = 0x00;  
				memcpy(&statusRepParam[5], &CURRENT_VOLT, 4);          
				memcpy(&statusRepParam[9], &CURRENT_AMP, 4);      
		
				INT_sendStatusReportCmd(timeStamp, 0x20, (char*)statusRepParam, 13 , TIMEOUT); 
				
				HAL_Delay(1000);
				xbee_receivePacket();
					
			}   
			//TIMER_setTimer(&statusRep_retrytime, 60);
}
void manag_sw(void){
		print_debug(0,"In Switch on/off\r\n");
		switch(status_sw) {
			case 0:
					print_debug(0,"SWITCH ON!\r\n");           
					INT_sendEventReportCmd(timeStamp, 0x00, 0xA0, stat_on, TIMEOUT);
					HAL_Delay(1000);
					xbee_receivePacket();
					status_sw = TURN_ON;
					break;
			case 1:
					print_debug(0,"SWITCH OFF!\r\n");           
					INT_sendEventReportCmd(timeStamp, 0x00, 0xA0, stat_off, TIMEOUT);
					HAL_Delay(1000);
					xbee_receivePacket();
					status_sw = TURN_OFF;
					break;
		}
}

// Get ADC values
// Returns 0..4095
int16_t getADC(unsigned short CS,unsigned short channel) 
{   
  int16_t ReadADC=0;
	uint8_t BffReadADC[4];
	uint8_t a[1];
	uint8_t b[]={0x06};
	uint8_t start = (uint8_t)0x06;
	uint8_t ch = (uint8_t)0x00;
	channel=channel*4;
	ch = (uint8_t)channel;
	ChipSelect(CS,0);
	
	HAL_SPI_Transmit(&hspi1,&start,1,10);
	HAL_SPI_Transmit(&hspi1,&ch,1,10);
	HAL_SPI_Receive(&hspi1,&BffReadADC[0],2,10);
	ReadADC = (BffReadADC[0] & 0x0F);               // Get first 8 bits of ADC value
  ReadADC = ReadADC << 8;                                // Shift ADC value by 8
	HAL_SPI_Transmit(&hspi1,&ch,1,10);
	HAL_SPI_Receive(&hspi1,&BffReadADC[2],2,10);
	ReadADC |= ((BffReadADC[1] << 4) | BffReadADC[2]) & 0x00FF;
	
	ChipSelect(CS,1);

  return ReadADC ;                                    // Returns 12-bit ADC value
}

void ChipSelect(unsigned short CS,unsigned short status)
{

	switch(status)// PIN CS LOW
	{
		case 0:
			switch(CS)// PIN CS LOW
			{
				case 1:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0,0); //CS1
					break;
				case 2:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,0); //CS2
					break;
				case 3:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,0); //CS3
					break;
				case 4:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8,0); //CS4
					break;
				case 5:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11,0); //CS5
					break;
				case 6:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12,0); //CS6
					break;
				case 7:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,0); //CS7
					break;
				case 8:
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10,0); //CS8
					break;
				case 9:
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11,0); //CS9
					break;
				case 10:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4,0); //CS10
					break;
				case 11:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5,0); //CS11
					break;
				case 12:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10,0); //CS12
					break;
				case 13:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11,0); //CS13
					break;
				case 14:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12,0); //CS14
					break;
				case 15:
					HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4,0); //CS15
					break;
				case 16:
					HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5,0); //CS16
					break;
				case 17:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0,0); //CS1
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,0); //CS2
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,0); //CS3
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8,0); //CS4
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11,0); //CS5
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12,0); //CS6
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,0); //CS7
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10,0); //CS8
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11,0); //CS9
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4,0); //CS10
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5,0); //CS11
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10,0); //CS12
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11,0); //CS13
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12,0); //CS14
					HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4,0); //CS15
					HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5,0); //CS16
					break;
				default :
					break;
			}
			break;
		case 1:
			switch(CS)// PIN CS LOW
			{
				case 1:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0,1); //CS1
					break;
				case 2:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,1); //CS2
					break;
				case 3:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,1); //CS3
					break;
				case 4:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8,1); //CS4
					break;
				case 5:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11,1); //CS5
					break;
				case 6:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12,1); //CS6
					break;
				case 7:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,1); //CS7
					break;
				case 8:
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10,1); //CS8
					break;
				case 9:
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11,1); //CS9
					break;
				case 10:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4,1); //CS10
					break;
				case 11:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5,1); //CS11
					break;
				case 12:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10,1); //CS12
					break;
				case 13:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11,1); //CS13
					break;
				case 14:
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12,1); //CS14
					break;
				case 15:
					HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4,1); //CS15
					break;
				case 16:
					HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5,1); //CS16
					break;
				case 17:
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0,1); //CS1
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,1); //CS2
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,1); //CS3
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8,1); //CS4
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11,1); //CS5
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12,1); //CS6
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,1); //CS7
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10,1); //CS8
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11,1); //CS9
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4,1); //CS10
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5,1); //CS11
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10,1); //CS12
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11,1); //CS13
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12,1); //CS14
					HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4,1); //CS15
					HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5,1); //CS16
					break;
				default :
					break;
			}
			break;
		default :
			break;
	}
	
}

void ReadCurrent(void)
{
	for(int i=0;i<4;i++)
	{
		adcValue = getADC(1,i);
		
		//print_debug(0,"ADC = %d\r\n",adcValue);
		value = ((float)adcValue) * (5000.0/ 4095.0);
		//print_debug(0,"value = %f\r\n",value);
		avg[i] = (499.0*avg[i] + value) / 500.0;
		//print_debug(0,"avg = %f\r\n",avg);	
//		if(i==0) 
//			print_debug(0,"avg = %f\r\n",avg[i]);	
		//print_debug(0,"value = %f\r\n",value);
		if(value > avg[i]) {
				Vdif = value - avg[i];
				total[i] += (Vdif*Vdif);              
		}else if(value < avg[i]) { 
				Vdif = avg[i] - value;
				total[i] += (Vdif*Vdif);
		}
		
	}


	countSampling++;
	FlagReadCurrent=0;
}
void ReadVoltage(void)
{
	HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_8);
	
	for(int i=0;i<4;i++)
	{
		//print_debug(0,"-----------------------CH %d---------------------------\r\n",i);
	 /*---------- Voltage ----------*/
		adcValue = getADC(2,i);
		//print_debug(0,"ADC = %d\r\n",adcValue);
		volt[i] = (((adcValue*5.0)/4095.0)/0.01); 
		if(volt[i] < 223.0) { 
				volt[i] += 9.0;
		}else if(volt[i] > 233.0) {
				volt[i] -= 4.0;
		}
		//print_debug(0,"Volt = %f\r\n",volt[i]);
		Vsum[i] += volt[i];
								
		/*---------- Current ----------*/

		//print_debug(0,"total = %f\r\n",total);	
		Vsq_avg = total[i] / ((float)countSampling);
		//print_debug(0,"Vsq_avg = %f\r\n",Vsq_avg);
		//print_debug(0,"countSampling = %d\r\n",countSampling);
		
		Viout = sqrt(Vsq_avg);
		Viout = Viout - Offset;
		amp[i] = Viout / 28;                  // ACS709 +-30 Amp.
		//print_debug(0,"Viout = %f\r\n",Viout);
				 
		/* Adjust Current to 0 */
		if(amp[i] < 0.025) {
				amp[i] = 0.0;                                            
		} 
				
		total[i] = 0.0;
		
		Isum[i] += amp[i];
		//print_debug(0,"amp = %f\r\n",amp[i]);	
		
		/*---------- Power ----------*/
		power[i] = volt[i]*amp[i];
		//print_debug(0,"Power = %f\r\n",power[i]);	
		Psum[i] += power[i];
								
		/*----------  Watt-hour ----------*/
		whour[i] = power[i]*(0.5/3600.0);
		//print_debug(0,"Watt-Hour = %f\r\n",whour[i]);
		WHsum[i] += whour[i];
	}
	
	
	
	//print_debug(0,"--------------------------------------------------\r\n");
	number++; 
	StatusSampling++;
	countSampling = 0;
	FlagReadVolt=0;
}

void Read_Switch(void)
{
	uint16_t pin=0x0001;
	int i=0;
	for(i=0;i<8;i++)
	{
		Read_SW[i] = HAL_GPIO_ReadPin(GPIOB,pin);
		pin = pin<<1;
	}
	
	pin = 0x1000;
	for(i=8;i<12;i++)
	{
		Read_SW[i] = HAL_GPIO_ReadPin(GPIOB,pin);
		pin = pin<<1;
	}
	
	pin = 0x0040;
	for(i=12;i<16;i++)
	{
		Read_SW[i] = HAL_GPIO_ReadPin(GPIOC,pin);
		pin = pin<<1;
	}
	
}

void nx_delay_us(uint32_t us)
{
		volatile uint32_t counter = 7 *us;
		while(counter--);
}
/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* SPI1 init function */
void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  HAL_SPI_Init(&hspi1);

}

/* TIM3 init function */
void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  //htim3.Init.Prescaler = 2048000;
	//htim3.Init.Prescaler = 15625;
	//htim3.Init.Prescaler = 31250;
	//htim3.Init.Prescaler = 93750;
	//htim3.Init.Prescaler = 23437;
	htim3.Init.Prescaler = 48000;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim3);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

}

/* USART1 init function */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart1);

}

/* USART2 init function */
void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart2);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : Input1_Pin Input2_Pin */
  GPIO_InitStruct.Pin = Input1_Pin|Input2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : switch_user_Pin */
  GPIO_InitStruct.Pin = switch_user_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(switch_user_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Cs1_Pin Cs2_Pin Cs3_Pin Cs4_Pin 
                           Cs5_Pin Cs6_Pin Cs7_Pin */
  GPIO_InitStruct.Pin = Cs1_Pin|Cs2_Pin|Cs3_Pin|Cs4_Pin 
                          |Cs5_Pin|Cs6_Pin|Cs7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Cs15_Pin Cs16_Pin */
  GPIO_InitStruct.Pin = Cs15_Pin|Cs16_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : Cs10_Pin Cs11_Pin Cs12_Pin Cs13_Pin 
                           Cs14_Pin */
  GPIO_InitStruct.Pin = Cs10_Pin|Cs11_Pin|Cs12_Pin|Cs13_Pin 
                          |Cs14_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : Relay1_Pin Relay2_Pin Relay3_Pin Relay9_Pin 
                           Relay10_Pin Relay11_Pin Relay12_Pin Relay4_Pin 
                           Relay5_Pin Relay6_Pin Relay7_Pin Relay8_Pin */
  GPIO_InitStruct.Pin = Relay1_Pin|Relay2_Pin|Relay3_Pin|Relay9_Pin 
                          |Relay10_Pin|Relay11_Pin|Relay12_Pin|Relay4_Pin 
                          |Relay5_Pin|Relay6_Pin|Relay7_Pin|Relay8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Cs8_Pin Cs9_Pin */
  GPIO_InitStruct.Pin = Cs8_Pin|Cs9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Relay13_Pin Relay14_Pin Relay15_Pin Relay16_Pin */
  GPIO_InitStruct.Pin = Relay13_Pin|Relay14_Pin|Relay15_Pin|Relay16_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PF6 PF7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Cs1_Pin|Cs2_Pin|Cs3_Pin|Cs4_Pin 
                          |Cs5_Pin|Cs6_Pin|Cs7_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, Cs15_Pin|Cs16_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Cs10_Pin|Cs11_Pin|Cs12_Pin|Cs13_Pin 
                          |Cs14_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Relay1_Pin|Relay2_Pin|Relay3_Pin|Relay9_Pin 
                          |Relay10_Pin|Relay11_Pin|Relay12_Pin|Relay4_Pin 
                          |Relay5_Pin|Relay6_Pin|Relay7_Pin|Relay8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Cs8_Pin|Cs9_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Relay13_Pin|Relay14_Pin|Relay15_Pin|Relay16_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);
	
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
		//print_debug(0,"Wrong parameters value: file %s on line %d\r\n", file, line);
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
