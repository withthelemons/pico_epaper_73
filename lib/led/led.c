#include "led.h"
#include "DEV_Config.h"

void ledPowerOn(void)
{
    for(int i=0; i<3; i++) {
        gpio_put(LED_ACT, 1);
        sleep_ms(200);
        gpio_put(LED_ACT, 0);
        sleep_ms(100);
    }
    watchdog_update();
}

void ledLowPower(void)
{
    for(int i=0; i<5; i++) {
        gpio_put(LED_PWR, 1);
        sleep_ms(200);
        gpio_put(LED_PWR, 0);
        sleep_ms(100);
    }
    watchdog_update();
}

void ledWarning(void)
{
    for(int i=0; i<20; i++) {
        gpio_put(LED_PWR, 1);
        sleep_ms(100);
        gpio_put(LED_PWR, 0);
        sleep_ms(50);
    }
    watchdog_update();
}

void ledCharging(void)
{
    gpio_put(LED_PWR, 1);
}

void ledCharged(void)
{
    gpio_put(LED_PWR, 0);
}

void powerOff(void)
{
    gpio_put(BAT_OFF, 0);
}