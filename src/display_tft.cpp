#include "display_tft.h"



TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite_lvl = TFT_eSprite(&tft);
TFT_eSprite sprite_msg = TFT_eSprite(&tft);

#if defined(LCD_MODULE_CMD_1)
typedef struct {
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;

lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},
};
#endif








void _draw_bar(int location_h, int volume_percent) {
    int location_h_px = location_h * SPRITE_WIDTH / 100;

    int x = location_h_px - BAR_WIDTH / 2;
    int y = BAR_PADDING * SPRITE_HEIGHT / 100;

    sprite_lvl.drawSmoothRoundRect(x, y, BAR_WIDTH / 2, BAR_WIDTH / 2 - 1, BAR_WIDTH, SPRITE_HEIGHT - (2 * BAR_PADDING * SPRITE_HEIGHT / 100), TFT_WHITE, BG_COLOR);
}


void display_tft_init()
{
    // This IO15 must be set to HIGH, otherwise nothing will be displayed when USB is not connected.
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    tft.begin();
    tft.setRotation(ROTATION);
    tft.fillScreen(TFT_BLACK);

    sprite_lvl.createSprite(SPRITE_WIDTH, SPRITE_HEIGHT);


#if defined(LCD_MODULE_CMD_1)
    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++) {
        tft.writecommand(lcd_st7789v[i].cmd);
        for (int j = 0; j < lcd_st7789v[i].len & 0x7f; j++) {
            tft.writedata(lcd_st7789v[i].data[j]);
        }

        if (lcd_st7789v[i].len & 0x80) {
            delay(120);
        }
    }
#endif

    // Turn on backlight
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);

}



void display_tft_levels(int tank1_volume, int tank2_volume)
{
    sprite_lvl.fillSprite(BG_COLOR);
    sprite_lvl.setTextSize(2);
    sprite_lvl.setTextColor(TFT_WHITE, TFT_DARKGREY);
    sprite_lvl.setCursor(0, 0);
    sprite_lvl.print("Waste level tank 1  [L]: ");
    sprite_lvl.println(tank1_volume);
    sprite_lvl.print("Waste level tank 2  [L]: ");
    sprite_lvl.println(tank2_volume);


    _draw_bar(BAR1_LOCATION_H, tank1_volume);
    _draw_bar(BAR2_LOCATION_H, tank2_volume);



    sprite_lvl.pushSprite(0, 0);
}

void display_tft_levels_old(int tank1_volume, int tank2_volume)
{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print("Waste level tank 1  [L]: ");
    tft.println(tank1_volume);
    tft.print("Waste level tank 2  [L]: ");
    tft.println(tank2_volume);
}

void display_tft_error(int elapsedTime)
{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("Error, time since last message [ms]: ");
    tft.println(elapsedTime);
}






// ---------------------- TFT PIN CHECK ---------------------- //
#if PIN_LCD_WR  != TFT_WR || \
    PIN_LCD_RD  != TFT_RD || \
    PIN_LCD_CS    != TFT_CS   || \
    PIN_LCD_DC    != TFT_DC   || \
    PIN_LCD_RES   != TFT_RST  || \
    PIN_LCD_D0   != TFT_D0  || \
    PIN_LCD_D1   != TFT_D1  || \
    PIN_LCD_D2   != TFT_D2  || \
    PIN_LCD_D3   != TFT_D3  || \
    PIN_LCD_D4   != TFT_D4  || \
    PIN_LCD_D5   != TFT_D5  || \
    PIN_LCD_D6   != TFT_D6  || \
    PIN_LCD_D7   != TFT_D7  || \
    PIN_LCD_BL   != TFT_BL  || \
    TFT_BACKLIGHT_ON   != HIGH  || \
    170   != TFT_WIDTH  || \
    320   != TFT_HEIGHT
#error  "Error! Please make sure <User_Setups/Setup206_LilyGo_T_Display_S3.h> is selected in <TFT_eSPI/User_Setup_Select.h>"
#error  "Error! Please make sure <User_Setups/Setup206_LilyGo_T_Display_S3.h> is selected in <TFT_eSPI/User_Setup_Select.h>"
#error  "Error! Please make sure <User_Setups/Setup206_LilyGo_T_Display_S3.h> is selected in <TFT_eSPI/User_Setup_Select.h>"
#error  "Error! Please make sure <User_Setups/Setup206_LilyGo_T_Display_S3.h> is selected in <TFT_eSPI/User_Setup_Select.h>"
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
#error  "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif