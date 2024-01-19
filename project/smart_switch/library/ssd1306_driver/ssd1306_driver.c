#include "ssd1306_driver.h"

static const char * TAG = "ssd1306";

static void i2c_master_init(SSD1306_t * display, int16_t sda_pin, int16_t scl_pin)
{
    i2c_config_t i2c_config = 
    {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = sda_pin,
		.scl_io_num = scl_pin,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_MASTER_FREQ_HZ
	};
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

	display->_address = SSD1306_I2CADDRESS;

	display->_flip = true;
}

void ssd1306_config_flip(SSD1306_t * display, bool is_flip)
{
	display->_flip = is_flip;
}

void ssd1306_init(SSD1306_t * display, int width, int height)
{
	i2c_master_init(display, SSD1306_I2C_SDA_PIN, SSD1306_I2C_SCL_PIN);

	ESP_LOGI(TAG, "INTERFACE is IIC");
	ESP_LOGI(TAG, "SDA_PIN = %d", SSD1306_I2C_SDA_PIN);
	ESP_LOGI(TAG, "SCL_PIN = %d", SSD1306_I2C_SCL_PIN);
    ESP_LOGI(TAG, "Panel is 128x64");

	display->_width = width;
	display->_height = height;
	display->_pages = 8;
	if (display->_height == 32) display->_pages = 4;
	
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (display->_address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);					// AE
	i2c_master_write_byte(cmd, OLED_CMD_SET_MUX_RATIO, true);				// A8
	if (display->_height == 64) i2c_master_write_byte(cmd, 0x3F, true);
	if (display->_height == 32) i2c_master_write_byte(cmd, 0x1F, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_OFFSET, true);			// D3
	i2c_master_write_byte(cmd, 0x00, true);
	//i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);		// 40
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_START_LINE, true);		// 40
	//i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true);			// A1
	if (display->_flip) 
	{
		i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP_0, true);		// A0
	} else 
	{
		i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP_1, true);		// A1
	}
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true);			// C8
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_CLK_DIV, true);			// D5
	i2c_master_write_byte(cmd, 0x80, true);	
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_PIN_MAP, true);				// DA
	if (display->_height == 64) i2c_master_write_byte(cmd, 0x12, true);	
	if (display->_height == 32) i2c_master_write_byte(cmd, 0x02, true);	
	i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);				// 81
	i2c_master_write_byte(cmd, 0xFF, true);	
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_RAM, true);					// A4
	i2c_master_write_byte(cmd, OLED_CMD_SET_VCOMH_DESELCT, true);			// DB
	i2c_master_write_byte(cmd, 0x40, true);	
	i2c_master_write_byte(cmd, OLED_CMD_SET_MEMORY_ADDR_MODE, true);		// 20
	//i2c_master_write_byte(cmd, OLED_CMD_SET_HORI_ADDR_MODE, true);		// 00
	i2c_master_write_byte(cmd, OLED_CMD_SET_PAGE_ADDR_MODE, true);			// 02
	// Set Lower Column Start Address for Page Addressing Mode	
	i2c_master_write_byte(cmd, 0x00, true);	
	// Set Higher Column Start Address for Page Addressing Mode	
	i2c_master_write_byte(cmd, 0x10, true);	
	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);				// 8D
	i2c_master_write_byte(cmd, 0x14, true);	
	i2c_master_write_byte(cmd, OLED_CMD_DEACTIVE_SCROLL, true);				// 2E
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_NORMAL, true);				// A6
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);					// AF

	i2c_master_stop(cmd);

	esp_err_t espRc = i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) 
	{
		ESP_LOGI(TAG, "OLED configured successfully");
	} else 
	{
		ESP_LOGE(TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);

	/* Initialize internal buffer */
	for (int i=0;i<display->_pages;i++) 
	{
		memset(display->_page[i]._segs, 0, 128);
	}
}

void ssd1306_clear_screen(SSD1306_t * display, bool invert)
{
	char space[16];
	memset(space, 0x00, sizeof(space));
	for (int page = 0; page < display->_pages; page++) {
		ssd1306_display_text(display, page, space, sizeof(space), invert);
	}
}

void ssd1306_display_text(SSD1306_t * display, int page, char * text, int text_len, bool invert)
{
	if (page >= display->_pages) return;
	int _text_len = text_len;
	if (_text_len > 16) _text_len = 16;

	uint8_t seg = 0;
	uint8_t image[8];
	for (uint8_t i = 0; i < _text_len; i++) {
		memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
		if (invert) ssd1306_invert(image, 8);
		if (display->_flip) ssd1306_flip(image, 8);
		ssd1306_display_image(display, page, seg, image, 8);
		seg = seg + 8;
	}
}

void ssd1306_invert(uint8_t *buf, size_t blen)
{
	uint8_t wk;
	for(int i=0; i<blen; i++){
		wk = buf[i];
		buf[i] = ~wk;
	}
}

void ssd1306_flip(uint8_t *buf, size_t blen)
{
	for(int i=0; i<blen; i++){
		buf[i] = ssd1306_rotate_byte(buf[i]);
	}
}

uint8_t ssd1306_rotate_byte(uint8_t ch1) 
{
	uint8_t ch2 = 0;
	for (int j=0;j<8;j++) {
		ch2 = (ch2 << 1) + (ch1 & 0x01);
		ch1 = ch1 >> 1;
	}
	return ch2;
}

void ssd1306_display_image(SSD1306_t * display, int page, int seg, uint8_t * images, int width)
{
	i2c_cmd_handle_t cmd;

	if (page >= display->_pages) return;
	if (seg >= display->_width) return;

	int _seg = seg + CONFIG_OFFSETX;
	uint8_t columLow = _seg & 0x0F;
	uint8_t columHigh = (_seg >> 4) & 0x0F;

	int _page = page;
	if (display->_flip) {
		_page = (display->_pages - page) - 1;
	}

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (display->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	// Set Lower Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x00 + columLow), true);
	// Set Higher Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x10 + columHigh), true);
	// Set Page Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, 0xB0 | _page, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (display->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, images, width, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	// Set to internal buffer
	memcpy(&display->_page[page]._segs[seg], images, width);
}

void i2c_display_image(SSD1306_t * display, int page, int seg, uint8_t * images, int width) 
{
	i2c_cmd_handle_t cmd;

	if (page >= display->_pages) return;
	if (seg >= display->_width) return;

	int _seg = seg + CONFIG_OFFSETX;
	uint8_t columLow = _seg & 0x0F;
	uint8_t columHigh = (_seg >> 4) & 0x0F;

	int _page = page;
	if (display->_flip) {
		_page = (display->_pages - page) - 1;
	}

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (display->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	// Set Lower Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x00 + columLow), true);
	// Set Higher Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x10 + columHigh), true);
	// Set Page Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, 0xB0 | _page, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (display->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, images, width, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}

void ssd1306_contrast(SSD1306_t * display, int contrast) 
{
	i2c_cmd_handle_t cmd;
	int _contrast = contrast;
	if (contrast < 0x0) _contrast = 0;
	if (contrast > 0xFF) _contrast = 0xFF;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (display->_address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);			// 81
	i2c_master_write_byte(cmd, _contrast, true);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}

void ssd1306_display_text_x3(SSD1306_t * display, int page, char * text, int text_len, bool invert)
{
	if (page >= display->_pages) return;
	int _text_len = text_len;
	if (_text_len > 5) _text_len = 5;

	uint8_t seg = 0;

	for (uint8_t nn = 0; nn < _text_len; nn++) {

		uint8_t const * const in_columns = font8x8_basic_tr[(uint8_t)text[nn]];

		// make the character 3x as high
		out_column_t out_columns[8];
		memset(out_columns, 0, sizeof(out_columns));

		for (uint8_t xx = 0; xx < 8; xx++) { // for each column (x-direction)

			uint32_t in_bitmask = 0b1;
			uint32_t out_bitmask = 0b111;

			for (uint8_t yy = 0; yy < 8; yy++) { // for pixel (y-direction)
				if (in_columns[xx] & in_bitmask) {
					out_columns[xx].u32 |= out_bitmask;
				}
				in_bitmask <<= 1;
				out_bitmask <<= 3;
			}
		}

		// render character in 8 column high pieces, making them 3x as wide
		for (uint8_t yy = 0; yy < 3; yy++)	{ // for each group of 8 pixels high (y-direction)

			uint8_t image[24];
			for (uint8_t xx = 0; xx < 8; xx++) { // for each column (x-direction)
				image[xx*3+0] = 
				image[xx*3+1] = 
				image[xx*3+2] = out_columns[xx].u8[yy];
			}
			if (invert) ssd1306_invert(image, 24);
			if (display->_flip) ssd1306_flip(image, 24);
				i2c_display_image(display, page+yy, seg, image, 24);
			memcpy(&display->_page[page+yy]._segs[seg], image, 24);
		}
		seg = seg + 24;
	}
}

