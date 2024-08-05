/*****************************************************************************
* | File      	:   EPD_7in3f_test.c
* | Author      :   Waveshare team
* | Function    :   7.3inch e-Paper (F) Demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2023-03-13
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
#include "EPD_Test.h"
#include "run_File.h"
#include "EPD_7in3f.h"
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"
#include "../main.h"
#include "../lib/RTC/waveshare_PCF85063.h"

uint8_t BlackImage[EPD_7IN3F_IMAGE_BYTESIZE];

void show_info(float voltage) {
    char strVoltage[8];
    char strPercentage[8];
    char strTime[24];
    sprintf(strVoltage, "%.3fV", voltage);
    float percentage = voltageToPercentage(voltage);
    sprintf(strPercentage, "%.1f%%", percentage);
    Time_data time = PCF85063_GetTime();
    time_to_str(strTime, time);
    // draw voltage to top left
    uint8_t current_background_color = Paint.Image[0];
    uint8_t font_color;
    if (current_background_color) {
        font_color = EPD_7IN3F_BLACK;
    }
    else {
        font_color = EPD_7IN3F_WHITE;
    }
    Paint_DrawString_EN(10, 0, strVoltage, &Font16, font_color, EPD_7IN3F_TRANSPARENT);
    // draw time to top middle
    Paint_DrawString_EN(EPD_7IN3F_WIDTH/2, 0, strTime, &Font16, font_color, EPD_7IN3F_TRANSPARENT);
    // draw percentage to top right
    Paint_DrawString_EN(EPD_7IN3F_WIDTH-65, 0, strPercentage, &Font16, font_color, EPD_7IN3F_TRANSPARENT);
}

void EPD_7in3f_display_BMP(float voltage)
{
    printf("e-Paper Init and Clear\n");
    EPD_7IN3F_Init();
    Paint_NewImage(BlackImage, EPD_7IN3F_WIDTH, EPD_7IN3F_HEIGHT, 0);

    uint32_t index = setFilePath();
    setPathIndex(index);
    GUI_ReadBmp_RGB_7Color();
    show_info(voltage);

    EPD_7IN3F_Display(BlackImage);

    printf("sending EPD sleep\n");
    EPD_7IN3F_Sleep();
}
