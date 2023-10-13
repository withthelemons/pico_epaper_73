/*****************************************************************************
* | File      	:   EPD_7in3f.c
* | Author      :   Waveshare team
* | Function    :   7.3inch e-Paper (F) Driver
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2022-08-04
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include <hardware/xosc.h>
#include <hardware/pll.h>
#include "hardware/structs/scb.h"
#include "EPD_7in3f.h"

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_7IN3F_Reset(void) {
    gpio_put(EPD_RST_PIN, 1);
    sleep_ms(20);
    gpio_put(EPD_RST_PIN, 0);
    sleep_ms(2);
    gpio_put(EPD_RST_PIN, 1);
    sleep_ms(20);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void EPD_7IN3F_SendCommand(uint8_t reg) {
    gpio_put(EPD_DC_PIN, 0);
    gpio_put(EPD_CS_PIN, 0);
    spi_write_blocking(EPD_SPI_PORT, &reg, 1);
    gpio_put(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void EPD_7IN3F_SendData(uint8_t data) {
    gpio_put(EPD_DC_PIN, 1);
    gpio_put(EPD_CS_PIN, 0);
    spi_write_blocking(EPD_SPI_PORT, &data, 1);
    gpio_put(EPD_CS_PIN, 1);
}

static void EPD_7IN3F_SendDataBulk(uint8_t *data, size_t len) {
    gpio_put(EPD_DC_PIN, 1);
    gpio_put(EPD_CS_PIN, 0);
    spi_write_blocking(EPD_SPI_PORT, data, len);
    gpio_put(EPD_CS_PIN, 1);
}

static void rosc_enable(void)
{
    uint32_t tmp = rosc_hw->ctrl;
    tmp &= (~ROSC_CTRL_ENABLE_BITS);
    tmp |= (ROSC_CTRL_ENABLE_VALUE_ENABLE << ROSC_CTRL_ENABLE_LSB);
    rosc_write(&rosc_hw->ctrl, tmp);
    // Wait for stable
    while ((rosc_hw->status & ROSC_STATUS_STABLE_BITS) != ROSC_STATUS_STABLE_BITS);
}

// https://github.com/ghubcoder/micropython-pico-deepsleep/blob/d8c8c67cd36959e325c16a3736d471e61952db6d/ports/rp2sleep/modpicosleep.c#L53C6-L53C24
void recover_from_sleep(uint scb_orig, uint clock0_orig, uint clock1_orig){
    //Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    //reset procs back to default
    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    //reset clocks
    clocks_init();
    stdio_init_all();
}

/******************************************************************************
function :	Wait until the busy_pin goes HIGH
parameter:
******************************************************************************/
static void EPD_7IN3F_ReadBusyH(void) {
    printf("e-Paper busy H\n");
    bool pin_high = false;

    // run soft-sleep loop for a few moments to see if the pin goes high
    for (int i = 0; i < 50; i++) {
        if (gpio_get(EPD_BUSY_PIN)) {
            pin_high = true;
            break;
        }
        sleep_ms(10);
    }
    watchdog_update();
    // don't go to sleep if the pin is already high
    if (!pin_high) {
        printf("dormant sleep");
        //save values for later
        uint scb_orig = scb_hw->scr;
        uint clock0_orig = clocks_hw->sleep_en0;
        uint clock1_orig = clocks_hw->sleep_en1;

        sleep_run_from_xosc();
        sleep_goto_dormant_until_level_high(EPD_BUSY_PIN);
        recover_from_sleep(scb_orig, clock0_orig, clock1_orig);
        printf("woke up");
    }

    printf("e-Paper busy H release\n");
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void EPD_7IN3F_TurnOnDisplay(void) {
    EPD_7IN3F_SendCommand(0x04); // POWER_ON
    EPD_7IN3F_ReadBusyH();

    EPD_7IN3F_SendCommand(0x12); // DISPLAY_REFRESH
    EPD_7IN3F_SendData(0x00);
    EPD_7IN3F_ReadBusyH();

    EPD_7IN3F_SendCommand(0x02); // POWER_OFF
    EPD_7IN3F_SendData(0X00);
    EPD_7IN3F_ReadBusyH();
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_7IN3F_Init(void) {
    EPD_7IN3F_Reset();
    EPD_7IN3F_ReadBusyH();
    sleep_ms(30);

    EPD_7IN3F_SendCommand(0xAA);    // CMDH
    uint8_t dataToSend[] = {0x49, 0x55, 0x20, 0x08, 0x09, 0x18};
    EPD_7IN3F_SendDataBulk(dataToSend, sizeof dataToSend);

    EPD_7IN3F_SendCommand(0x01);
    uint8_t dataToSend2[] = {0x3F,0x00,0x32,0x2A,0x0E,0x2A};
    EPD_7IN3F_SendDataBulk(dataToSend2, sizeof dataToSend2);

    EPD_7IN3F_SendCommand(0x00);
    uint8_t dataToSend3[] = {0x5F,0x69};
    EPD_7IN3F_SendDataBulk(dataToSend3, sizeof dataToSend3);

    EPD_7IN3F_SendCommand(0x03);
    uint8_t dataToSend4[] = {0x00,0x54,0x00,0x44};
    EPD_7IN3F_SendDataBulk(dataToSend4, sizeof dataToSend4);

    EPD_7IN3F_SendCommand(0x05);
    uint8_t dataToSend5[] = {0x40,0x1F,0x1F,0x2C};
    EPD_7IN3F_SendDataBulk(dataToSend5, sizeof dataToSend5);

    EPD_7IN3F_SendCommand(0x06);
    uint8_t dataToSend6[] = {0x6F,0x1F,0x1F,0x22};
    EPD_7IN3F_SendDataBulk(dataToSend6, sizeof dataToSend6);

    EPD_7IN3F_SendCommand(0x08);
    EPD_7IN3F_SendDataBulk(dataToSend6, sizeof dataToSend6); // this is not a mistake, it's same data twice

    EPD_7IN3F_SendCommand(0x13);    // IPC
    uint8_t dataToSend7[] = {0x00,0x04};
    EPD_7IN3F_SendDataBulk(dataToSend7, sizeof dataToSend7);

    EPD_7IN3F_SendCommand(0x30);
    EPD_7IN3F_SendData(0x3C);

    EPD_7IN3F_SendCommand(0x41);     // TSE
    EPD_7IN3F_SendData(0x00);

    EPD_7IN3F_SendCommand(0x50);
    EPD_7IN3F_SendData(0x3F);

    EPD_7IN3F_SendCommand(0x60);
    uint8_t dataToSend8[] = {0x00,0x02};
    EPD_7IN3F_SendDataBulk(dataToSend8, sizeof dataToSend8);

    EPD_7IN3F_SendCommand(0x61);
    uint8_t dataToSend9[] = {0x03,0x20,0x01,0xE0};
    EPD_7IN3F_SendDataBulk(dataToSend9, sizeof dataToSend9);

    EPD_7IN3F_SendCommand(0x82);
    EPD_7IN3F_SendData(0x1E);

    EPD_7IN3F_SendCommand(0x84);
    EPD_7IN3F_SendData(0x00);

    EPD_7IN3F_SendCommand(0x86);    // AGID
    EPD_7IN3F_SendData(0x00);

    EPD_7IN3F_SendCommand(0xE3);
    EPD_7IN3F_SendData(0x2F);

    EPD_7IN3F_SendCommand(0xE0);   // CCSET
    EPD_7IN3F_SendData(0x00);

    EPD_7IN3F_SendCommand(0xE6);   // TSSET
    EPD_7IN3F_SendData(0x00);

}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_7IN3F_Clear(uint8_t color) {
    UWORD Width, Height;
    Width = (EPD_7IN3F_WIDTH % 2 == 0) ? (EPD_7IN3F_WIDTH / 2) : (EPD_7IN3F_WIDTH / 2 + 1);
    Height = EPD_7IN3F_HEIGHT;

    EPD_7IN3F_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_7IN3F_SendData((color << 4) | color);
        }
    }

    EPD_7IN3F_TurnOnDisplay();
}

/******************************************************************************
function :	show 7 kind of color block
parameter:
******************************************************************************/
void EPD_7IN3F_Show7Block(void) {
    unsigned long i, j, k;
    unsigned char const Color_seven[8] =
            {EPD_7IN3F_BLACK, EPD_7IN3F_BLUE, EPD_7IN3F_GREEN, EPD_7IN3F_ORANGE,
             EPD_7IN3F_RED, EPD_7IN3F_YELLOW, EPD_7IN3F_WHITE, EPD_7IN3F_WHITE};

    EPD_7IN3F_SendCommand(0x10);
    for (i = 0; i < 240; i++) {
        for (k = 0; k < 4; k++) {
            for (j = 0; j < 100; j++) {
                EPD_7IN3F_SendData((Color_seven[k] << 4) | Color_seven[k]);
            }
        }
    }
    for (i = 0; i < 240; i++) {
        for (k = 4; k < 8; k++) {
            for (j = 0; j < 100; j++) {
                EPD_7IN3F_SendData((Color_seven[k] << 4) | Color_seven[k]);
            }
        }
    }
    EPD_7IN3F_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_7IN3F_Display(uint8_t *Image) {
    printf("sending data\n");
    EPD_7IN3F_SendCommand(0x10);
    EPD_7IN3F_SendDataBulk(Image, EPD_7IN3F_IMAGE_BYTESIZE);
    printf("data sent\n");
    EPD_7IN3F_TurnOnDisplay();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_7IN3F_Sleep(void) {
    EPD_7IN3F_SendCommand(0x07); // DEEP_SLEEP
    EPD_7IN3F_SendData(0XA5);
}

