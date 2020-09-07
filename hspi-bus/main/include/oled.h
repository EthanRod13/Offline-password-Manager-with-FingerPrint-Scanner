#ifndef __OLED_H__
#define __OLED_H__

#include "esp_err.h"
#include "esp_log.h"

// determine how trasmitted bits are processed
#define OLED_CMD    0x0
#define OLED_DATA   0x200

#define TAG "OLED"
#define OLED_NUM_BITS 10

// instruction code
// ref: https://jcinfotr.com/Utilitaires/1602LCD.pdf
#define OLED_CLEAR_DISP       0x1
#define OLED_CURSOR_RET       0x2
#define OLED_BUY_INPUT        0x4   // OR with LEFT, RIGHT, TEXT
#define OLED_BUY_INPUT_LEFT   0x0
#define OLED_BUY_INPUT_RIGHT  0x2
#define OLED_BUY_INPUT_TEXT   0x1
#define OLED_DISP_CTRL        0x8
#define OLED_DISP_CTRL_OPEN   0x4
#define OLED_DISP_CTRL_CURSOR 0x2
#define OLED_DISP_CTRL_FLASH  0x1
#define OLED_TEXT_SHIFT       0x18
#define OLED_CURSOR_SHIFT     0x10
#define OLED_WRITE_DATA       0x200 // OR with to write data
#define OLED_READ_DATA        0x300 // OR with to read data

esp_err_t oled_init();
esp_err_t clear_all();
esp_err_t display_upper(const char* str);
esp_err_t display_lower(const char* str);

#endif