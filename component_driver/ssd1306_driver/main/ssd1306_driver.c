#include <stdio.h>
#include "ssd1306_driver.h"

SSD1306_t main_display;

void app_main(void)
{
    ssd1306_init(&main_display, 128, 64);
    ssd1306_clear_screen(&main_display, false);
    ssd1306_contrast(&main_display, 0xFF);
    ssd1306_display_text(&main_display, 0, "  MAIN DISPLAY  ", 16, true);
    ssd1306_display_text(&main_display, 1, "----------------", 16, false);
}
