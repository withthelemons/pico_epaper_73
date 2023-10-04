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
load24bitBMP(BMPINFOHEADER *bmpInfoHeader, FIL *fil) {
    unsigned int br, x, y, color;
    uint8_t Rdata[3];
    printf("reading data, 24bpp\n");
    for (y = 0; y < (*bmpInfoHeader).biHeight; y++) {//Total display column
        for (x = 0; x < (*bmpInfoHeader).biWidth; x++) {//Show a line in the line
            if (f_read(fil, &Rdata, 3, &br) != FR_OK) {
                printf("error reading BMP\n");
                return;
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
            Paint_SetPixel_fast((*bmpInfoHeader).biWidth - 1 - x, y, color);
        }
    }
}

void
load8bitBMP(BMPINFOHEADER *bmpInfoHeader, FIL *fil) {
    unsigned int br, x, y;
    unsigned int image_width = (*bmpInfoHeader).biWidth;
    uint8_t* read_data = (uint8_t*)malloc(image_width);
    printf("reading data, 8bpp\n");
    for (y = 0; y < (*bmpInfoHeader).biHeight; y++) {//Total display column
        if (f_read(fil, read_data, image_width, &br) != FR_OK) {
            printf("error reading BMP\n");
            break;
        }
        for (x = 0; x < image_width; x++) {//Show a line in the line
            Paint_SetPixel_fast((*bmpInfoHeader).biWidth - 1 - x, y, read_data[x]);
        }
    }
    free(read_data);
}


void
load4bitBMP(BMPINFOHEADER *bmpInfoHeader, FIL *fil) {
    unsigned int br;
    printf("reading data, 4bpp\n");
    uint32_t image_size = (*bmpInfoHeader).bimpImageSize;
    if (image_size > EPD_7IN3F_IMAGE_BYTESIZE) {
        printf("image is too big\n");
        return;
    }
    if ((*bmpInfoHeader).biCompression != 0) {
        printf("compressed images are not supported (yet)\n");
        return;
    }
    FRESULT read_result = f_read(fil, Paint.Image, image_size, &br);
    if (read_result != FR_OK) {
        printf("error reading BMP\n");
    }
}

void GUI_ReadBmp_RGB_7Color(const char *path) {
    BMPFILEHEADER bmpFileHeader;  //Define a bmp file header structure
    BMPINFOHEADER bmpInfoHeader;  //Define a bmp info header structure

    FIL fil;
    unsigned int br;
    printf("open %s\n", path);
    FRESULT fr = f_open(&fil, path, FA_READ);
    if (FR_OK != fr && FR_EXIST != fr) {
        panic("f_open(%s) error: %s (%d)\n", path, FRESULT_str(fr), fr);
    }

    // Set the file pointer from the beginning
    f_lseek(&fil, 0);
    f_read(&fil, &bmpFileHeader, sizeof(BMPFILEHEADER), &br);   // sizeof(BMPFILEHEADER) must be 14
    if (br != sizeof(BMPFILEHEADER)) {
        panic("f_read bmpFileHeader error\n");
    }
    f_read(&fil, &bmpInfoHeader, sizeof(BMPINFOHEADER), &br);   // sizeof(BMPFILEHEADER) must be 50
    if (br != sizeof(BMPINFOHEADER)) {
        panic("f_read bmpInfoHeader error\n");
    }
    f_lseek(&fil, bmpFileHeader.bOffset);
    uint32_t image_width = bmpInfoHeader.biWidth;
    uint32_t image_height = bmpInfoHeader.biHeight;
    printf("image dimensions = %lu * %lu\n", image_width, image_height);
    if (image_width < image_height) {
        Paint_SetRotate(90);
        image_width = bmpInfoHeader.biHeight;
        image_height = bmpInfoHeader.biWidth;
    }
    if (image_width > EPD_7IN3F_WIDTH || image_height > EPD_7IN3F_HEIGHT) {
        Paint_DrawString_EN(10, 10, "Wrong image size", &Font16, EPD_7IN3F_WHITE, EPD_7IN3F_BLACK);
    }

    // Determine if it is a monochrome bitmap
    uint16_t bit_depth = bmpInfoHeader.biBitCount;
    if (bit_depth == 4) {
        load4bitBMP(&bmpInfoHeader, &fil);
    } else if (bit_depth == 8) {
        load8bitBMP(&bmpInfoHeader, &fil);
    } else if (bit_depth == 24) {
        load24bitBMP(&bmpInfoHeader, &fil);
    } else {
        printf("Unsupported image depth: %u\n", bit_depth);
    } // or static image instead: Paint_DrawBitMap(Image7color); Paint_SetRotate(270);
    printf("close file\n");
    f_close(&fil);
    Paint_SetRotate(180 + Paint.Rotate);
}


