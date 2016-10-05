#ifndef RTC_DS1672_H
#define RTC_DS1672_H

#include "debug.h"
/*============================================================*/
#define ADDRTC          0xD0
#define ACK             1
#define NACK            0

#define RTC_REAL        0
#define RTC_BACKUP      1

#define DAY_MON         1
#define DAY_TUE         2
#define DAY_WED         3
#define DAY_THU         4
#define DAY_FRI         5
#define DAY_SAT         6
#define DAY_SUN         7

/*============================================================*/

struct   rtc_structure {
                unsigned long year;
                unsigned long month;
                unsigned long date;
                unsigned long hour;
                unsigned long min;
                unsigned long sec;         
                unsigned int  day;
                };
                                                                         
extern unsigned long raw_rtc;   

/*============================================================*/                           
int DS1672_trickerChargeEnable(void);                                                                
unsigned long int DS1672_read(void);                         

unsigned long int date_to_binary(struct rtc_structure rtcSrc);     
int binary_to_date(unsigned long x, struct rtc_structure *rtcCov);       

int RTC_setBinary(unsigned long int rtc_bin);
int RTC_setDateTime(struct rtc_structure rtcSrc,int timezone);
int RTC_getDateTime(struct rtc_structure *rtcPtr, int timezone);  

void printRTC(struct rtc_structure time);
void init_RTC(void);

#endif
