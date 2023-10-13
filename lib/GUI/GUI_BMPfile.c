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
#include "GUI_BMPfile.h"
#include "GUI_Paint.h"
#include "Debug.h"

#include "f_util.h"
#include "ff.h"
#include "../e-Paper/EPD_7in3f.h"

extern char disPath[255];
static uint8_t read_data[EPD_7IN3F_WIDTH];

void
load24bitBMP(FIL *fil, uint32_t image_width, uint32_t image_height) {
    unsigned int br, x, y, color;
    printf("reading data, 24bpp\n");
    for (y = 0; y < image_height; y++) {
        for (x = 0; x < image_width; x++) {
            FRESULT read_result = f_read(fil, read_data, 3, &br);
            if (read_result != FR_OK) {
                panic("f_read() error: %s (%d)\n", FRESULT_str(read_result), read_result);
            }

            if (read_data[0] == 0 && read_data[1] == 0 && read_data[2] == 0) {
                color = EPD_7IN3F_BLACK;
            } else if (read_data[0] == 255 && read_data[1] == 255 && read_data[2] == 255) {
                color = EPD_7IN3F_WHITE;
            } else if (read_data[0] == 0 && read_data[1] == 255 && read_data[2] == 0) {
                color = EPD_7IN3F_GREEN;
            } else if (read_data[0] == 255 && read_data[1] == 0 && read_data[2] == 0) {
                color = EPD_7IN3F_BLUE;
            } else if (read_data[0] == 0 && read_data[1] == 0 && read_data[2] == 255) {
                color = EPD_7IN3F_RED;
            } else if (read_data[0] == 0 && read_data[1] == 255 && read_data[2] == 255) {
                color = EPD_7IN3F_YELLOW;
            } else if (read_data[0] == 0 && read_data[1] == 128 && read_data[2] == 255) {
                color = EPD_7IN3F_ORANGE;
            }
            Paint_SetPixel_fast(image_width - 1 - x, y, color);
        }
    }
}

void
load8bitBMP(FIL *fil, uint32_t image_width, uint32_t image_height) {
    unsigned int br, x, y;
    printf("reading data, 8bpp\n");
    for (y = 0; y < image_height; y++) {
        FRESULT read_result = f_read(fil, read_data, image_width, &br);
        if (read_result != FR_OK) {
            panic("f_read() error: %s (%d)\n", FRESULT_str(read_result), read_result);
        }
        for (x = 0; x < image_width; x++) {
            Paint_SetPixel_fast(image_width - 1 - x, y, read_data[x]);
        }
    }
}


void
load4bitBMP(FIL *fil, uint32_t image_size) {
    unsigned int br;
    printf("reading data, 4bpp\n");
    if (image_size > EPD_7IN3F_IMAGE_BYTESIZE) {
        printf("image is too big\n");
        return;
    }
    FRESULT read_result = f_read(fil, Paint.Image, image_size, &br);
    if (read_result != FR_OK) {
        panic("f_read() error: %s (%d)\n", FRESULT_str(read_result), read_result);
    }
}

void
load1bitBMP(FIL *fil, uint32_t image_size) {
    unsigned int br, x, i;
    uint8_t color;
    printf("reading data, 1bpp\n");
    uint32_t address = 0;
    uint32_t remainingBytes = image_size;
    while (remainingBytes) {
        FRESULT read_result = f_read(fil, read_data, MIN(800, remainingBytes), &br);
        if (read_result != FR_OK) {
            panic("f_read() error: %s (%d)\n", FRESULT_str(read_result), read_result);
        }
        remainingBytes -= br;
        for (x = 0; x < br; x++) {
            i = 6;
            for (uint8_t mask = 0b11000000; mask != 0; mask >>= 2) {
                uint8_t two_bits = (read_data[x] & mask) >> i;
                if (two_bits == 0b11) color = 0b00010001;
                if (two_bits == 0b01) color = 0b00000001;
                if (two_bits == 0b10) color = 0b00010000;
                if (two_bits == 0b00) color = 0b00000000;
                Paint.Image[address] = color;
                address++;
                i -= 2;
            }
        }
    }
}

void GUI_ReadBmp_RGB_7Color() {
    BMPFILEHEADER bmpFileHeader;  //Define a bmp file header structure
    BMPINFOHEADER bmpInfoHeader;  //Define a bmp info header structure

    FIL fil;
    unsigned int br;
    FRESULT fr = f_open(&fil, disPath, FA_READ);
    if (FR_OK != fr && FR_EXIST != fr) {
        panic("f_open(%s) error: %s (%d)\n", disPath, FRESULT_str(fr), fr);
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

    if (bmpInfoHeader.biCompression != 0) {
        printf("compressed images are not supported (yet)\n");
        f_close(&fil);
        return;
    }

    f_lseek(&fil, bmpFileHeader.bOffset);
    uint32_t image_width = bmpInfoHeader.biWidth;
    uint32_t image_height = bmpInfoHeader.biHeight;
    printf("image dimensions: %lux%lu\n", image_width, image_height);
    if (image_width < image_height) {
        Paint_SetRotate(90);
        image_width = bmpInfoHeader.biHeight;
        image_height = bmpInfoHeader.biWidth;
    }
    if (image_width > EPD_7IN3F_WIDTH || image_height > EPD_7IN3F_HEIGHT) {
        Paint_DrawString_EN(10, 10, "Wrong image size", &Font16, EPD_7IN3F_WHITE, EPD_7IN3F_BLACK);
        f_close(&fil);
        return;
    }

    uint16_t bit_depth = bmpInfoHeader.biBitCount;
    uint32_t image_size = bmpInfoHeader.bimpImageSize;
    if (bit_depth == 1) {
        load1bitBMP(&fil, image_size);
    } else if (bit_depth == 4) {
        load4bitBMP(&fil, image_size);
    } else if (bit_depth == 8) {
        load8bitBMP(&fil, image_width, image_height);
    } else if (bit_depth == 24) {
        load24bitBMP(&fil, image_width, image_height);
    } else {
        printf("Unsupported image depth: %u\n", bit_depth);
    } // or static image instead: Paint_DrawBitMap(Image7color); Paint_SetRotate(270);
    f_close(&fil);
    Paint_SetRotate(180 + Paint.Rotate);
}


