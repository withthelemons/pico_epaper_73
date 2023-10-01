#include "EPD_Test.h"   // Examples
#include "run_File.h"

#include "led.h"
#include "waveshare_PCF85063.h" // RTC
#include "DEV_Config.h"

#include <math.h>

extern char *disPath;

#define enChargingRtc 0

float voltageToPercentage(float voltage)
{
    float percentage;
    if (voltage > 4.19f) {
        percentage = 100;
    } else if (voltage < 3.5f){
        percentage = 0;
    }
    else {
        percentage = 2808.3808f * powf(voltage, 4) - 43560.9157f * powf(voltage, 3) + 252848.5888f * powf(voltage, 2) - 650767.4615f * voltage + 626532.5703f;
    }
    return percentage;
}


float measureVBAT(void) {
    const float conversion_factor = (3.3f / (1 << 12)) * 3;
    uint32_t sum = 0;
    for (int i = 0; i < 100; i++) {
        uint16_t result = adc_read();
        sum += result;
    }
    float average = (float)sum / 100;
    float voltage = average * conversion_factor;
    float percentage = voltageToPercentage(voltage);
    printf("Raw value: %f, voltage: %fV, percentage: %f\n", average, voltage, percentage);
    return voltage;
}

void chargeState_callback() 
{
    if(DEV_Digital_Read(VBUS)) {
        if(!DEV_Digital_Read(CHARGE_STATE)) {  // is charging
            ledCharging();
        }
        else {  // charge complete
            ledCharged();
        }
    }
}

void run_display(Time_data Time, Time_data alarmTime, bool hasCard, float voltage)
{
    if(hasCard) {
        run_mount();
        uint32_t index = setFilePath();
        EPD_7in3f_display_BMP(disPath, index, voltage);
        run_unmount();
    }

    PCF85063_clear_alarm_flag();    // clear RTC alarm flag
    rtcRunAlarm(Time, alarmTime);  // RTC run alarm
}

void addMinutes(Time_data* currentTime, UWORD minutesToAdd) {
    UWORD oldPlusNew = currentTime->minutes + minutesToAdd;
    currentTime->hours += oldPlusNew / 60;
    currentTime->minutes = oldPlusNew % 60;
}

int main(void)
{
    Time_data Time = {2023-1970, 4, 1, 8, 0, 0};
    Time_data alarmTime = { 2023 - 1970, 4, 1, 8, 30, 0 };

    printf("Init\n");
    if(DEV_Module_Init() != 0) {  // DEV init
        return -1;
    }

    watchdog_enable(8*1000, 1);
    PCF85063_init();    // RTC init
    rtcRunAlarm(Time, alarmTime);  // RTC run alarm
    
    gpio_set_irq_enabled_with_callback(CHARGE_STATE, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, chargeState_callback);

    float voltage = measureVBAT();
    if(voltage  < 3.5f) {   // battery power is low
        printf("low power\n");
        PCF85063_alarm_Time_Disable();
        ledLowPower();  // LED flash for Low power
        powerOff(); // BAT off
        return 0;
    }
    else {
        printf("work\n");
        ledPowerOn();
    }

    bool hasCard = sdTest();
    if(!DEV_Digital_Read(VBUS)) {    // no charge state
        run_display(Time, alarmTime, hasCard, voltage);
    }
    else {  // charge state
        chargeState_callback();
        while(DEV_Digital_Read(VBUS)) {
            #if enChargingRtc
            if(!DEV_Digital_Read(RTC_INT)) {    // RTC interrupt trigger
                printf("rtc interrupt\n");
                run_display(Time, alarmTime, hasCard);
            }
            #endif

            if(!DEV_Digital_Read(BAT_STATE)) {  // KEY pressed
                voltage = measureVBAT();
                printf("key interrupt\n");
                run_display(Time, alarmTime, hasCard, voltage);
            }
            watchdog_update();
            DEV_Delay_ms(200);
        }
    }
    
    printf("power off\n");
    powerOff(); // BAT off

    return 0;
}
