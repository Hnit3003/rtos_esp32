#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#include "string.h"
#include "esp_log.h"
#include "ssd1306_driver.h"
#include "font8x8_basic.h"
#include "driver/i2c.h"

// #define SSD1306_I2C_SDA_PIN             18
// #define SSD1306_I2C_SCL_PIN            	19

#define SSD1306_I2C_SDA_PIN             15
#define SSD1306_I2C_SCL_PIN            	27

#define I2C_MASTER_FREQ_HZ      		400000 		/* I2C clock of SSD1306 can run at 400 kHz max */
#define SSD1306_I2CADDRESS      		0x3C        /* Address SSD1306 */
#define CONFIG_OFFSETX 					0
#define I2C_NUM 						I2C_NUM_0   /* I2C Port 0 */

/* Control byte for i2c
Co : bit 8 : Continuation Bit 
 * 1 = no-continuation (only one byte to follow) 
 * 0 = the controller should expect a stream of bytes. 
D/C# : bit 7 : Data/Command Select bit 
 * 1 = the next byte or byte stream will be Data. 
 * 0 = a Command byte or byte stream will be coming up next. 
 Bits 6-0 will be all zeros. 
Usage: 
0x80 : Single Command byte 
0x00 : Command Stream 
0xC0 : Single Data byte 
0x40 : Data Stream
*/
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_SINGLE   0xC0
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20
#define OLED_CMD_SET_HORI_ADDR_MODE     0x00    // Horizontal Addressing Mode
#define OLED_CMD_SET_VERT_ADDR_MODE     0x01    // Vertical Addressing Mode
#define OLED_CMD_SET_PAGE_ADDR_MODE     0x02    // Page Addressing Mode
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP_0    0xA0    
#define OLED_CMD_SET_SEGMENT_REMAP_1    0xA1    
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8    
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

// Scrolling Command
#define OLED_CMD_HORIZONTAL_RIGHT       0x26
#define OLED_CMD_HORIZONTAL_LEFT        0x27
#define OLED_CMD_CONTINUOUS_SCROLL      0x29
#define OLED_CMD_DEACTIVE_SCROLL        0x2E
#define OLED_CMD_ACTIVE_SCROLL          0x2F
#define OLED_CMD_VERTICAL               0xA3

#define PACK8 __attribute__((aligned( __alignof__( uint8_t ) ), packed ))

typedef struct {
	bool _valid; // Not using it anymore
	int _segLen; // Not using it anymore
	uint8_t _segs[128];
} PAGE_t;

typedef struct {
	int _address;
	int _width;
	int _height;
	int _pages;
	int _dc;
	bool _scEnable;
	int _scStart;
	int _scEnd;
	int _scDirection;
	PAGE_t _page[8];
	bool _flip;
} SSD1306_t;

typedef union out_column_t {
	uint32_t u32;
	uint8_t  u8[4];
} PACK8 out_column_t;

void ssd1306_config_flip(SSD1306_t * display, bool is_flip);
void ssd1306_init(SSD1306_t * display, int width, int height);
void i2c_init(SSD1306_t * display, int width, int height) ;
void ssd1306_clear_screen(SSD1306_t * display, bool invert);
void ssd1306_display_text(SSD1306_t * display, int page, char * text, int text_len, bool invert);
void ssd1306_invert(uint8_t *buf, size_t blen);
void ssd1306_flip(uint8_t *buf, size_t blen);
uint8_t ssd1306_rotate_byte(uint8_t ch1);
void ssd1306_display_image(SSD1306_t * display, int page, int seg, uint8_t * images, int width);
void i2c_display_image(SSD1306_t * display, int page, int seg, uint8_t * images, int width);
void ssd1306_contrast(SSD1306_t * display, int contrast);
void i2c_contrast(SSD1306_t * display, int contrast);
void ssd1306_display_text_x3(SSD1306_t * display, int page, char * text, int text_len, bool invert);



#endif
