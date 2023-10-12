#include "led.h"
#include "DEV_Config.h"

void ledPowerOn(void)
{
    for(int i=0; i<3; i++) {
        DEV_Digital_Write(LED_ACT, 1);
        sleep_ms(200);
        DEV_Digital_Write(LED_ACT, 0);
        sleep_ms(100);
    }
    watchdog_update();
}

void ledLowPower(void)
{
    for(int i=0; i<5; i++) {
        DEV_Digital_Write(LED_PWR, 1);
        sleep_ms(200);
        DEV_Digital_Write(LED_PWR, 0);
        sleep_ms(100);
    }
    watchdog_update();
}

void ledCharging(void)
{
    DEV_Digital_Write(LED_PWR, 1);
}

void ledCharged(void)
{
    DEV_Digital_Write(LED_PWR, 0);
}

void powerOff(void)
{
    DEV_Digital_Write(BAT_OFF, 0);
}