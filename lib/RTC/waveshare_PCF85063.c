/*****************************************************************************
* | File      	:   waveshare_PCF85063.c
* | Author      :   Waveshare team
* | Function    :   PCF85063 driver
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-02-02
* | Info        :   Basic version
*
******************************************************************************/
#include <time.h>
#include "DEV_Config.h"
#include "waveshare_PCF85063.h"

/******************************************************************************
function:	Read one byte of data to EMC2301 via I2C
parameter:  
            Addr: Register address
Info:
******************************************************************************/
static UBYTE PCF85063_Read_Byte(UBYTE Addr)
{
	return I2C_Read_Byte(Addr);
}

/******************************************************************************
function:	Send one byte of data to EMC2301 via I2C
parameter:
            Addr: Register address
           Value: Write to the value of the register
Info:
******************************************************************************/
static void PCF85063_Write_Byte(UBYTE Addr, UBYTE Value)
{
	I2C_Write_Byte(Addr, Value);
}

uint8_t DecToBcd(uint8_t val)
{
	return ((val/10)*16 + (val%10)); 
}

uint8_t BcdToDec(uint8_t val)
{
	return ((val/16)*10 + (val%16));
}

void PCF85063_SetTime(Time_data time)
{
    uint8_t years = time.years % 100;
    uint8_t months = time.months % 13;
    uint8_t days = time.days % 32;
    uint8_t hours = time.hours % 24;
    uint8_t minutes = time.minutes % 60;
    uint8_t seconds = time.seconds % 60;
    PCF85063_Write_Byte(YEARS_REG  ,DecToBcd(years));
    PCF85063_Write_Byte(MONTHS_REG , DecToBcd(months) & 0x1F);
    PCF85063_Write_Byte(DAYS_REG   , DecToBcd(days) & 0x3F);
    PCF85063_Write_Byte(HOURS_REG  ,DecToBcd(hours)&0x3F);
    PCF85063_Write_Byte(MINUTES_REG,DecToBcd(minutes)&0x7F);
    PCF85063_Write_Byte(SECONDS_REG,DecToBcd(seconds)&0x7F);
}

Time_data PCF85063_GetTime()
{
	Time_data time;
	time.years = BcdToDec(PCF85063_Read_Byte(YEARS_REG));
	time.months = BcdToDec(PCF85063_Read_Byte(MONTHS_REG)&0x1F);
	time.days = BcdToDec(PCF85063_Read_Byte(DAYS_REG)&0x3F);
	time.hours = BcdToDec(PCF85063_Read_Byte(HOURS_REG)&0x3F);
	time.minutes = BcdToDec(PCF85063_Read_Byte(MINUTES_REG)&0x7F);
	time.seconds = BcdToDec(PCF85063_Read_Byte(SECONDS_REG)&0x7F);
	return time;
}

void time_to_str(char* buf, Time_data time) {
    sprintf(buf, "%hhu.%hhu.%u %hhu:%02hhu:%02hhu", time.days, time.months, time.years+2000, time.hours, time.minutes, time.seconds);
}

void PCF85063_alarm_Time_Enabled(Time_data time)
{
	PCF85063_Write_Byte(CONTROL_2_REG, PCF85063_Read_Byte(CONTROL_2_REG)|0x80);	// Alarm on
	PCF85063_Write_Byte(HOUR_ARARM_REG, DecToBcd(time.hours) & 0x7F);
	PCF85063_Write_Byte(MINUTES_ALARM_REG, DecToBcd(time.minutes) & 0x7F);
	PCF85063_Write_Byte(SECOND_ALARM_REG, DecToBcd(time.seconds) & 0x7F);
}

void PCF85063_alarm_Time_Disable() 
{
	PCF85063_Write_Byte(HOUR_ARARM_REG   ,PCF85063_Read_Byte(HOUR_ARARM_REG)|0x80);
	PCF85063_Write_Byte(MINUTES_ALARM_REG,PCF85063_Read_Byte(MINUTES_ALARM_REG)|0x80);
	PCF85063_Write_Byte(SECOND_ALARM_REG ,PCF85063_Read_Byte(SECOND_ALARM_REG)|0x80);
	PCF85063_Write_Byte(CONTROL_2_REG   ,PCF85063_Read_Byte(CONTROL_2_REG)&0x7F);	// Alarm OFF
}

bool PCF85063_get_alarm_flag()
{
	return ((PCF85063_Read_Byte(CONTROL_2_REG)&0x40) == 0x40);
}

void PCF85063_clear_alarm_flag()
{
	PCF85063_Write_Byte(CONTROL_2_REG   ,PCF85063_Read_Byte(CONTROL_2_REG)&0xBF);
}

void PCF85063_test()
{
    int count = 0;
    Time_data time = {21, 2, 28, 23, 59, 58};
    PCF85063_SetTime(time);
	while(1)
	{
		Time_data T;
		T = PCF85063_GetTime();
		printf("%d-%d-%d %d:%d:%d\n",T.years,T.months,T.days,T.hours,T.minutes,T.seconds);
		count+=1;
		DEV_Delay_ms(1000);
		if(count>6)
		break;
	}
}

void timeDataToTimeTm(struct tm* time_tm, Time_data *time_data) {
    time_tm->tm_sec = time_data->seconds;
    time_tm->tm_min = time_data->minutes;
    time_tm->tm_hour = time_data->hours;
    time_tm->tm_mday = time_data->days;
    time_tm->tm_mon = time_data->months-1;
    time_tm->tm_year = time_data->years+100;
}

void timeTmToTimeData(struct tm* time_tm, Time_data *time_data) {
    time_data->seconds = time_tm->tm_sec;
    time_data->minutes = time_tm->tm_min;
    time_data->hours = time_tm->tm_hour;
    time_data->days = time_tm->tm_mday;
    time_data->months = time_tm->tm_mon+1;
    time_data->years = time_tm->tm_year-100;
}

void addMinutes(Time_data* time_data, int minutesToAdd) {
    struct tm time_tm;
    timeDataToTimeTm(&time_tm,time_data);
    time_t equivalent = mktime(&time_tm);
    equivalent += (minutesToAdd*60);
    struct tm *newTime = gmtime(&equivalent);
    timeTmToTimeData(newTime, time_data);
}

void scheduleAlarm(int minutes)
{
    Time_data time = PCF85063_GetTime();
    addMinutes(&time, minutes);
    PCF85063_alarm_Time_Enabled(time);
}