/*****************************************************************************
* | File      	:   GUI_BMPfile.h
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                   Used to shield the underlying layers of each master
*                   and enhance portability
*----------------
* |	This version:   V1.0
* | Date        :   2022-10-11
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
#include <stdlib.h>
#include "GUI_BMPfile.h"
#include "GUI_Paint.h"
#include "Debug.h"

#include "f_util.h"
#include "ff.h"
#include "../e-Paper/EPD_7in3f.h"

void
load24bitBMP(BMPFILEHEADER *bmpFileHeader, BMPINFOHEADER *bmpInfoHeader, FIL *fil) {
    unsigned int br;
    uint_fast16_t x, y, color;
    uint8_t Rdata[3];
    f_lseek(fil, (*bmpFileHeader).bOffset);
    printf("reading data, 24bpp\n");
    for (y = 0; y < (*bmpInfoHeader).biHeight; y++) {//Total display column
        for (x = 0; x < (*bmpInfoHeader).biWidth; x++) {//Show a line in the line
            if (f_read(fil, (char *) Rdata, 1, &br) != FR_OK) {
                printf("error reading BMP\n");
                break;
            }
            if (f_read(fil, (char *) Rdata + 1, 1, &br) != FR_OK) {
                printf("error reading BMP\n");
                break;
            }
            if (f_read(fil, (char *) Rdata + 2, 1, &br) != FR_OK) {
                printf("error reading BMP\n");
                break;
            }

            if (Rdata[0] == 0 && Rdata[1] == 0 && Rdata[2] == 0) {
                // Image[x+(y* bmpInfoHeader.biWidth )] =  0;//Black
                color = 0;
            } else if (Rdata[0] == 255 && Rdata[1] == 255 && Rdata[2] == 255) {
                // Image[x+(y* bmpInfoHeader.biWidth )] =  1;//White
                color = 1;
            } else if (Rdata[0] == 0 && Rdata[1] == 255 && Rdata[2] == 0) {
                // Image[x+(y* bmpInfoHeader.biWidth )] =  2;//Green
                color = 2;
            } else if (Rdata[0] == 255 && Rdata[1] == 0 && Rdata[2] == 0) {
                // Image[x+(y* bmpInfoHeader.biWidth )] =  3;//Blue
                color = 3;
            } else if (Rdata[0] == 0 && Rdata[1] == 0 && Rdata[2] == 255) {
                // Image[x+(y* bmpInfoHeader.biWidth )] =  4;//Red
                color = 4;
            } else if (Rdata[0] == 0 && Rdata[1] == 255 && Rdata[2] == 255) {
                // Image[x+(y* bmpInfoHeader.biWidth )] =  5;//Yellow
                color = 5;
            } else if (Rdata[0] == 0 && Rdata[1] == 128 && Rdata[2] == 255) {
                // Image[x+(y* bmpInfoHeader.biWidth )] =  6;//Orange
                color = 6;
            }
            Paint_SetPixel((*bmpInfoHeader).biWidth - 1 - x, y, color);
        }
        watchdog_update();
    }
}

void
load8bitBMP(BMPFILEHEADER *bmpFileHeader, BMPINFOHEADER *bmpInfoHeader, FIL *fil) {
    unsigned int br;
    uint_fast16_t x, y;
    unsigned int image_width = (*bmpInfoHeader).biWidth;
    uint8_t* read_data = (uint8_t*)malloc(image_width);
    f_lseek(fil, (*bmpFileHeader).bOffset);
    printf("reading data, 8bpp\n");
    for (y = 0; y < (*bmpInfoHeader).biHeight; y++) {//Total display column
        if (f_read(fil, read_data, image_width, &br) != FR_OK) {
            printf("error reading BMP\n");
            break;
        }
        for (x = 0; x < image_width; x++) {//Show a line in the line
            Paint_SetPixel((*bmpInfoHeader).biWidth - 1 - x, y, read_data[x]);
        }
        watchdog_update();
    }
    free(read_data);
}


void
load4bitBMP(BMPFILEHEADER *bmpFileHeader, BMPINFOHEADER *bmpInfoHeader, FIL *fil) {
    unsigned int br;
    f_lseek(fil, (*bmpFileHeader).bOffset);
    printf("reading data, 4bpp\n");
    FRESULT read_result = f_read(fil, Paint.Image, EPD_7IN3F_IMAGE_BYTESIZE, &br);
    if (read_result != FR_OK) {
        printf("error reading BMP\n");
    }
    watchdog_update();
}

void GUI_ReadBmp_RGB_7Color(const char *path) {
    BMPFILEHEADER bmpFileHeader;  //Define a bmp file header structure
    BMPINFOHEADER bmpInfoHeader;  //Define a bmp info header structure

    FIL fil;
    unsigned int br;
    printf("open %s", path);
    FRESULT fr = f_open(&fil, path, FA_READ);
    if (FR_OK != fr && FR_EXIST != fr) {
        panic("f_open(%s) error: %s (%d)\n", path, FRESULT_str(fr), fr);
    }

    // Set the file pointer from the beginning
    f_lseek(&fil, 0);
    f_read(&fil, &bmpFileHeader, sizeof(BMPFILEHEADER), &br);   // sizeof(BMPFILEHEADER) must be 14
    if (br != sizeof(BMPFILEHEADER)) {
        printf("f_read bmpFileHeader error\n");
    }
    f_read(&fil, &bmpInfoHeader, sizeof(BMPINFOHEADER), &br);   // sizeof(BMPFILEHEADER) must be 50
    if (br != sizeof(BMPINFOHEADER)) {
        printf("f_read bmpInfoHeader error\n");
    }
    if (bmpInfoHeader.biWidth > bmpInfoHeader.biHeight)
        Paint_SetRotate(0);
    else
        Paint_SetRotate(90);

    printf("pixel = %lu * %lu\n", bmpInfoHeader.biWidth, bmpInfoHeader.biHeight);

    // Determine if it is a monochrome bitmap
    uint16_t bit_depth = bmpInfoHeader.biBitCount;
    if (bit_depth == 4) {
        load4bitBMP( &bmpFileHeader, &bmpInfoHeader, &fil);
    } else if (bit_depth == 8) {
        load8bitBMP( &bmpFileHeader, &bmpInfoHeader, &fil);
    } else if (bit_depth == 24) {
        load24bitBMP(&bmpFileHeader, &bmpInfoHeader, &fil);
    } else {
        printf("Unsupported image depth: %u\n", bit_depth);
    }
    printf("close file\n");
    f_close(&fil);
    Paint_SetRotate(180 + Paint.Rotate);
}


