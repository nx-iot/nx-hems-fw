#ifndef TIMER_H
#define TIMER_H

#include <mega128a.h> 
#include <stdint.h>


typedef unsigned long int TIMER;

extern TIMER baseCounter;
extern uint8_t pressedBTCounter;
extern uint8_t _FlagBT;
extern uint8_t _Flag05INT;
extern uint8_t _Flag0001INT;
extern uint8_t _BlinkLED_1Hz;
                                       
TIMER TIMER_checkTimeLeft(TIMER time);
int TIMER_checkTimerExceed(TIMER keeper);
int TIMER_setTimer(TIMER *keeper, unsigned long int period);

void init_timer(void);
void enable_timerOverflow(int ch);
void disable_timerOverflow(int ch);

#endif
