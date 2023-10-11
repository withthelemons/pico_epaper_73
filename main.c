#include "EPD_Test.h"   // Examples
#include "run_File.h"

#include "led.h"
#include "waveshare_PCF85063.h" // RTC
#include "ff.h"

#include <math.h>


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

void setTimeFromCard() {
    uint8_t buf[6];
    FRESULT fr;
    FIL fil;
    unsigned int br;

    fr =  f_open(&fil, "time.dat", FA_READ);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("time.dat doesn't exist\n");
        return;
    }
    f_read(&fil, &buf, 6,	&br);
    f_close(&fil);
    f_unlink("time.dat");

    Time_data time = {buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]};
    PCF85063_SetTime(time);
}

void run_display(bool hasCard, float voltage)
{
    if(hasCard) {
        run_mount();
        setTimeFromCard();
        EPD_7in3f_display_BMP(voltage);
        run_unmount();
    }

    PCF85063_clear_alarm_flag();    // clear RTC alarm flag
    scheduleAlarm(30);  // RTC run alarm
}

int main(void)
{
    printf("Init\n");
    if(DEV_Module_Init() != 0) {  // DEV init
        return -1;
    }

    watchdog_enable(8*1000, 1);
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
        run_display(hasCard, voltage);
    }
    else {  // charge state
        chargeState_callback();
        while(DEV_Digital_Read(VBUS)) {
            #if enChargingRtc
            if(!DEV_Digital_Read(RTC_INT)) {    // RTC interrupt trigger
                printf("rtc interrupt\n");
                run_display(hasCard, voltage);
            }
            #endif

            if(!DEV_Digital_Read(BAT_STATE)) {  // KEY pressed
                voltage = measureVBAT();
                printf("key interrupt\n");
                run_display(hasCard, voltage);
            }
            DEV_Delay_ms(100);
        }
    }
    
    printf("power off\n");
    powerOff(); // BAT off

    return 0;
}
