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

#include <stdlib.h>

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
    Paint_DrawString_EN(10, 0, strVoltage, &Font16, EPD_7IN3F_BLACK, 0xFFFF);
    // draw time to top middle
    Paint_DrawString_EN(EPD_7IN3F_WIDTH/2, 0, strTime, &Font16, EPD_7IN3F_BLACK, 0xFFFF);
    // draw percentage to top right
    Paint_DrawString_EN(EPD_7IN3F_WIDTH-65, 0, strPercentage, &Font16, EPD_7IN3F_BLACK, 0xFFFF);
    if(percentage <= 15.0f) {
        // draw warning to bottom left
        Paint_DrawString_EN(10, EPD_7IN3F_HEIGHT - 16, "Low voltage, please charge.", &Font16, EPD_7IN3F_BLACK, 0xFFFF);
    }
}

void EPD_7in3f_display_BMP(const char *path, uint32_t index, float voltage)
{
    printf("e-Paper Init and Clear\n");
    EPD_7IN3F_Init();

    //Create a new image cache
    UBYTE *BlackImage;
    if((BlackImage = (UBYTE *)calloc(EPD_7IN3F_IMAGE_BYTESIZE,1)) == NULL) {
        printf("Failed to allocate image memory\n");
        return;
    }
    printf("Paint_NewImage\n");
    Paint_NewImage(BlackImage, EPD_7IN3F_WIDTH, EPD_7IN3F_HEIGHT, 0, EPD_7IN3F_WHITE);
    Paint_SetScale(7);

    printf("Display BMP\n");
    Paint_SelectImage(BlackImage);
    GUI_ReadBmp_RGB_7Color(path);
    show_info(voltage);

    printf("EPD_Display\n");
    EPD_7IN3F_Display(BlackImage);
    printf("Update Path Index\n");
    updatePathIndex(index);

    printf("sending EPD sleep\n");
    EPD_7IN3F_Sleep();
    free(BlackImage);
    BlackImage = NULL;
}
