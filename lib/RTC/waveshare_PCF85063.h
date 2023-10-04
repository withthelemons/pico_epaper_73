#ifndef WAVESHARE_PCF85063_H_
#define WAVESHARE_PCF85063_H_

#include "DEV_Config.h"

#define PCF85063_ADDRESS   0x51

#define		CONTROL_1_REG         0x00  
#define 	CONTROL_2_REG         0x01 
#define 	OFFSET_REG            0x02 
#define 	RAM_BYTE_REG          0x03 
#define		SECONDS_REG           0x04 
#define 	MINUTES_REG           0x05 
#define 	HOURS_REG             0x06 
#define 	DAYS_REG              0x07 
#define		WEEKDAYS_REG          0x08 
#define		MONTHS_REG            0x09 
#define 	YEARS_REG             0x0A 
#define 	SECOND_ALARM_REG      0x0B 
#define 	MINUTES_ALARM_REG     0x0C 
#define 	HOUR_ARARM_REG        0x0D  
#define 	DAY_ALARM_REG         0x0E
#define 	WEEKDAY_ALARM_REG     0x0F
#define 	TIMER_VALUE_REG       0x10
#define 	TIMER_MODE_REG        0x11

typedef struct{
    uint8_t years;
    uint8_t months;
    uint8_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
}Time_data;

void PCF85063_init();
void time_to_str(char* buf, Time_data time);
Time_data PCF85063_GetTime();
void PCF85063_SetTime(Time_data time);
void PCF85063_alarm_Time_Enabled(Time_data time);
void PCF85063_alarm_Time_Disable();
bool PCF85063_get_alarm_flag();
void PCF85063_clear_alarm_flag();
void scheduleAlarm(int minutes);


#endif
