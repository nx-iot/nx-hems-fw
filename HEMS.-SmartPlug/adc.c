#include <mega128a.h> 
#include <stdio.h>
#include <stdint.h>
#include <delay.h>
#include <string.h>
#include "adc.h"
#include "debug.h"

//eeprom float _adcVref = 5.0;
char Vreferent = VREF_AVCC;
/* ================================================================================= */ 
void init_adc(unsigned char vrff) {

    // ADC initialization
    // ADC Clock frequency: 691.200 kHz
    // ADC Voltage Reference: vrff
    Vreferent = vrff;
    ADMUX = Vreferent & 0xff;
    ADCSRA = 0xA4;   
    //printDebug("ADMUX = %02X\r\n", ADMUX);

}
/* ================================================================================= */ 
// Read the AD conversion result
uint16_t read_adc(unsigned char adc_input) {

    unsigned int adc_data = 0x0000;          
    
    ADMUX = (Vreferent | adc_input);
    //printDebug("ADMUX = %02X\r\n", ADMUX);
    
    // Delay needed for the stabilization of the ADC input voltage
    delay_us(50);   
    
    // Start the AD conversion 
    ADCSRA |= 0x80; // ENABLE ADC
    ADCSRA |= 0x40;  // Start convert
    
    // Wait for the AD conversion to complete
    while((ADCSRA & 0x10) == 0);
    adc_data = ADCL;                
    adc_data |= (ADCH & 0x00ff) << 8;           
    
    ADCSRA |= 0x10;                           
    
    return adc_data;
    
}
/* ================================================================================= */ 




