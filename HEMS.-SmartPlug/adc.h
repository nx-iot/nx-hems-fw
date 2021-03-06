#ifndef ATD_H
#define ATD_H

#include <mega128a.h> 
#include <stdint.h>

#define VREF_ARFF 0x00            // AREF, Internal VREF turned off
#define VREF_AVCC 0x40            // AVCC with external capacitor at AREF pin
#define VREF_INTERNAL_11 0x80     // Internal 1.1V Voltage Reference with external capacitor at AREF pin
#define VREF_INTERNAL_256 0xC0    // Internal 2.56V Voltage Reference with external capacitor at AREF pin

#define ADC0 0x00
#define ADC1 0x01
#define ADC2 0x02
#define ADC3 0x03
#define ADC4 0x04
#define ADC5 0x05
#define ADC6 0x06
#define ADC7 0x07


//extern eeprom float _adcVref;

void init_adc(unsigned char vrff);
uint16_t read_adc(unsigned char adc_input);


#endif