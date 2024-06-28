#include "display_tft.h"



TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite_total = TFT_eSprite(&tft);
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



void display_plot(int tank_id, int days_range, int x_start, int y_start);

int _wrapText(String& text, int width) {
    int textLength = text.length();
    int lineWidth = 0;
    int lastSpaceIndex = -1;
    int numberLineBreaks = 0;

    for (int i = 0; i < textLength; i++) {
        char currentChar = text.charAt(i);
        int charWidth = sprite_msg.textWidth(String(currentChar));

        if (currentChar == ' ') {
            lastSpaceIndex = i;
        }
        else if (currentChar == '\n') {
            numberLineBreaks++;
            lineWidth = 0;
        }

        if (lineWidth + charWidth > width) {
            if (lastSpaceIndex != -1) {
                text.setCharAt(lastSpaceIndex, '\n');
                lineWidth = lineWidth - sprite_msg.textWidth(text.substring(0, lastSpaceIndex)) - sprite_msg.textWidth(" ");
                lastSpaceIndex = -1;
            } 
            else {
                text = text.substring(0, i) + "\n" + text.substring(i);
                lineWidth = 0; }
            numberLineBreaks++;
        }

        lineWidth += charWidth;
    }

    return numberLineBreaks;
}

void _draw_bar(int location_h, int volume_percent, int volume_liters) {
    int location_h_px = location_h * SPRITE_WIDTH / 100;

    int x_bar = location_h_px - BAR_WIDTH / 2;
    int y_bar = BAR_PADDING * SPRITE_HEIGHT / 100;

    int x_bar_inner = x_bar + 2 * BAR_PADDING_INNER;
    int y_bar_inner = y_bar + 2 * BAR_PADDING_INNER + (SPRITE_HEIGHT - (2 * BAR_PADDING * SPRITE_HEIGHT / 100) - 4 * BAR_PADDING_INNER) * 0.01 * (100 - volume_percent);
    int bar_inner_width = (BAR_WIDTH - 4 * BAR_PADDING_INNER);

    //uint16_t bar_color = volume_percent < 33 ? BAR_COLOR_EMPTY : volume_percent < 80 ? BAR_COLOR_MEDIUM : BAR_COLOR_FULL;
    uint16_t bar_color;
    if (volume_percent < 50)
        bar_color = tft.alphaBlend(map(volume_percent, 0, 50, 0, 255), BAR_COLOR_MEDIUM, BAR_COLOR_EMPTY);
    else bar_color = tft.alphaBlend(map(volume_percent, 50, 100, 0, 255), BAR_COLOR_FULL, BAR_COLOR_MEDIUM);


    if (volume_percent > 2) {
        sprite_total.fillSmoothRoundRect(
            x_bar_inner,
            y_bar_inner,
            bar_inner_width + 1,
            (SPRITE_HEIGHT - (2 * BAR_PADDING * SPRITE_HEIGHT / 100) - 4 * BAR_PADDING_INNER) * 0.01 * volume_percent + 1,
            bar_inner_width / 2,
            tft.alphaBlend(90, 0x0000, bar_color),
            BG_COLOR);
  
        sprite_total.fillSmoothRoundRect(
            x_bar_inner - bar_inner_width * 0.2,
            y_bar_inner - bar_inner_width * 0.2,
            bar_inner_width + 1,
            (SPRITE_HEIGHT - (2 * BAR_PADDING * SPRITE_HEIGHT / 100) - 4 * BAR_PADDING_INNER) * 0.01 * volume_percent + 1,
            bar_inner_width / 2,
            bar_color);

        sprite_total.fillSmoothRoundRect(
            x_bar_inner + bar_inner_width * 0.1,
            y_bar_inner + bar_inner_width / 2,
            bar_inner_width * 0.2,
            (SPRITE_HEIGHT - (2 * BAR_PADDING * SPRITE_HEIGHT / 100) - 4 * BAR_PADDING_INNER) * 0.01 * volume_percent + 1 - 1.2 * bar_inner_width,
            bar_inner_width * 0.2 / 2,
            tft.alphaBlend(80, 0xFFFF, bar_color));    
    }

    int bar_cover_paddding = bar_inner_width * 0.5;
    int correction = 1;
    sprite_total.drawSmoothRoundRect(
        x_bar_inner - bar_cover_paddding - correction,
        y_bar_inner - bar_cover_paddding - correction,
        bar_inner_width / 2 + bar_cover_paddding,
        bar_inner_width / 2,
        bar_inner_width + 2 * bar_cover_paddding + 2 * correction,
        (SPRITE_HEIGHT - (2 * BAR_PADDING * SPRITE_HEIGHT / 100) - 4 * BAR_PADDING_INNER) * 0.01 * volume_percent + 1 + 2 * bar_cover_paddding + 2 * correction,
        BG_COLOR,
        BG_COLOR);

    sprite_total.drawSmoothRoundRect(
        x_bar,
        y_bar,
        BAR_WIDTH / 2,
        BAR_WIDTH / 2 - 1,
        BAR_WIDTH,
        SPRITE_HEIGHT - (2 * BAR_PADDING * SPRITE_HEIGHT / 100),
        TFT_WHITE,
        BG_COLOR);

    sprite_total.drawWedgeLine(
        x_bar_inner - 4 * BAR_PADDING_INNER - ARROW_SIZE,
        y_bar_inner,
        x_bar_inner - 4 * BAR_PADDING_INNER,
        y_bar_inner,
        ARROW_SIZE,
        1,
        TFT_WHITE,
        BG_COLOR
    );

    sprite_total.drawWedgeLine(
        x_bar_inner + 4 * BAR_PADDING_INNER + ARROW_SIZE + bar_inner_width,
        y_bar_inner,
        x_bar_inner + 4 * BAR_PADDING_INNER + bar_inner_width,
        y_bar_inner,
        ARROW_SIZE,
        1,
        TFT_WHITE,
        BG_COLOR
    );


    String text = String(volume_liters) + " L";

    sprite_total.setTextSize(1);
    sprite_total.setTextFont(2);
    sprite_total.setTextColor(TFT_WHITE);
    sprite_total.setCursor(x_bar + BAR_WIDTH / 2 - (sprite_total.textWidth(text) / 2) + 2, (BAR_PADDING * SPRITE_HEIGHT / 100) / 2 - 6);
    sprite_total.println(text);


    text = "Tank ";
    String number = (location_h == BAR1_LOCATION_H) ? "1" : "2";
    text.concat(number);

    sprite_total.setCursor(x_bar + BAR_WIDTH / 2 - (sprite_total.textWidth(text) / 2) + 2, SPRITE_HEIGHT - (BAR_PADDING * SPRITE_HEIGHT / 100) / 2 - 6);
    sprite_total.println(text);
}


void display_init()
{
    // This IO15 must be set to HIGH, otherwise nothing will be displayed when USB is not connected.
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    tft.begin();
    tft.setRotation(ROTATION);
    tft.fillScreen(TFT_BLACK);

    sprite_total.createSprite(SPRITE_WIDTH, SPRITE_HEIGHT);


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

    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);

}

void display_update() {
    sprite_total.pushSprite(0, 0);
    }

void display_levels(uint8_t *sensor_buffer1, uint8_t *sensor_buffer2)
{
    remoteSensor_t sensor1 = *(remoteSensor_t *) sensor_buffer1;
    remoteSensor_t sensor2 = *(remoteSensor_t *) sensor_buffer2;

    sprite_total.fillSprite(BG_COLOR);

    _draw_bar(BAR1_LOCATION_H, sensor1.volume_percent, sensor1.volume_liters);
    _draw_bar(BAR2_LOCATION_H, sensor2.volume_percent, sensor2.volume_liters);
}

void display_msg_box(int center_x, int center_y, int width, String msg, bool center, uint16_t color = TFT_BLACK) {
    int box_padding = 6;
    TFT_eSprite sprite_box = TFT_eSprite(&tft);

    int number_lines = _wrapText(msg, width - 2 * box_padding) + 1;
    int sprite_height =  16 * number_lines + 2 * box_padding;

    sprite_box.createSprite(width, sprite_height);
    sprite_box.fillSmoothRoundRect(
        0,
        0,
        width,
        sprite_height,
        10,
        color,
        BG_COLOR);

    sprite_box.drawSmoothRoundRect(
        0,
        0,
        10,
        9,
        width,
        sprite_height,
        TFT_WHITE,
        BG_COLOR);

    // set text properties
    sprite_box.setTextSize(1);
    sprite_box.setTextFont(2);
    sprite_box.setTextColor(TFT_WHITE);
    sprite_box.setCursor(box_padding, box_padding);

    int line_number = 0;
    int startPos = 0;
    int endPos = msg.indexOf('\n');
    while (endPos != -1) {
        String substring = msg.substring(startPos, endPos);
        if (center)
            sprite_box.drawCentreString(substring, width / 2, line_number * 16 + box_padding, 2);
        else {
            sprite_box.setCursor(box_padding, line_number * 16 + box_padding);
            sprite_box.print(substring);
        }
        startPos = endPos + 1;
        line_number++;
        endPos = msg.indexOf('\n', startPos);
    }

    // Draw the last substring
    String lastSubstring = msg.substring(startPos);
    if (center) 
        sprite_box.drawCentreString(lastSubstring, width / 2, line_number * 16 + box_padding, 2);
    else {
        sprite_box.setCursor(box_padding, line_number * 16 + box_padding);
        sprite_box.print(lastSubstring);
    }


    sprite_box.pushToSprite(&sprite_total, center_x - width / 2, center_y - sprite_height / 2);
    sprite_box.deleteSprite();
}

void display_error(uint8_t *sensor_buffer) {
    remoteSensor_t sensor = *(remoteSensor_t *) sensor_buffer;
  
    String msg = "";
    
    switch(sensor.error)
    {
        case SENSOR_ERROR_NONE:
            return;
        case SENSOR_ERROR_TIMEOUT:
            msg.concat("Keine Antwort");
            break;
        case SENSOR_ERROR_INVALID_DATA:
            msg.concat("Sensor verschmutzt");
            break;
        case SENSOR_ERROR_SEND:
            msg.concat("Sendefehler");
            break;
        case SENSOR_ERROR_RECEIVE:
            msg.concat("Empfangsfehler");
            break;
        case SENSOR_ERROR_ID:
            msg.concat("Falsche ID");
            break;
        default:
            msg.concat("Unbekannter Fehler");
    }

    if (sensor.sensor_id == 1)
        display_msg_box(SPRITE_WIDTH / 4, SPRITE_HEIGHT / 2, SPRITE_WIDTH / 2 - 4, msg, true, BG_COLOR);

    else display_msg_box(3 * SPRITE_WIDTH / 4, SPRITE_HEIGHT / 2, SPRITE_WIDTH / 2 - 4, msg, true, BG_COLOR);
}

void display_advanced_data(uint8_t* sensor_buffer) {
    remoteSensor_t sensor = *(remoteSensor_t *) sensor_buffer;
    String msg = "Tank " + String(sensor.sensor_id) + "\n" +
                 "Sensormessung: " + String(sensor.data.tank_measurement) + " cm\n" +
                 "Batteriespannung: " + String(sensor.data.battery_voltage/1000.) + "V\n" +
                 "Empfangsstaerke: " + String(map(sensor.rssi, 0, 255, 0, 100)) + "%";
    
    int padding = 5; // pixels
    int x = ROTATION % 2 ? SPRITE_WIDTH / 4 + (sensor.sensor_id - 1) * SPRITE_WIDTH / 2 : SPRITE_WIDTH / 2;
    int y = ROTATION % 2 ? SPRITE_HEIGHT / 2 : SPRITE_HEIGHT / 4 + (sensor.sensor_id - 1) * SPRITE_HEIGHT / 2;
    display_msg_box(x, y, TFT_WIDTH, msg, false, BG_COLOR);
}

void display_advanced_page() {
    sprite_total.fillSprite(BG_COLOR);
    // set text properties

    display_advanced_data((uint8_t*) &sensor_1);    
    display_advanced_data((uint8_t*) &sensor_2);
}

void display_plot1d_page() {
    sprite_total.fillSprite(BG_COLOR);
    display_plot(1, 1, 5, 15);
    display_plot(2, 1, 5, 175);
}

void display_plot1w_page() {
    sprite_total.fillSprite(BG_COLOR);
    display_plot(1, 7, 5, 15);
    display_plot(2, 7, 5, 175);
}

void display_plot1m_page() {
    sprite_total.fillSprite(BG_COLOR);
    display_plot(1, 30, 5, 15);
    display_plot(2, 30, 5, 175);
}

void display_plot3m_page() {
    sprite_total.fillSprite(BG_COLOR);
    display_plot(1, 90, 5, 15);
    display_plot(2, 90, 5, 175);
}

void display_plot(int tank_id, int days_range, int x_start, int y_start) {
    prepare_plot_data(tank_id, days_range);

    uint8_t* plot_data = (tank_id == 1) ? plot_data_1 : plot_data_2;

    if (days_range >= 30)
        days_range = days_range / 30 * 30;

    else if (days_range >= 7)
        days_range = days_range / 7 * 7;

    int plot_height = 100;

    sprite_total.setTextSize(1);
    sprite_total.setTextFont(2);
    sprite_total.setTextColor(TFT_WHITE);

    sprite_total.setCursor(x_start + DATALOG_PLOT_MAX_ITEMS / 2 - sprite_total.textWidth("Tank 1") / 2, y_start);
    sprite_total.println("Tank " + String(tank_id));

    y_start += 20;

    if (days_range == 1) {
        sprite_total.drawFastVLine(x_start + DATALOG_PLOT_MAX_ITEMS / 2, y_start, plot_height, TFT_DARKGREY);
        
        sprite_total.setCursor(x_start, y_start + plot_height + 5);
        sprite_total.println("24h");
        sprite_total.setCursor(x_start + DATALOG_PLOT_MAX_ITEMS / 2 - sprite_total.textWidth("12h") / 2, y_start + plot_height + 5);
        sprite_total.println("12h");
    }

    else if (days_range < 7) {
        for (int i = 0; i < 7; i++) {
            sprite_total.drawFastVLine(x_start + i * DATALOG_PLOT_MAX_ITEMS / days_range, y_start, plot_height, TFT_DARKGREY);
        }
        sprite_total.setCursor(x_start, y_start + plot_height + 5);
        sprite_total.println(String(days_range) + " Tage");
    }

    else if (days_range == 7) {
        for (int i = 1; i < 7; i++) {
            sprite_total.drawFastVLine(x_start + i * DATALOG_PLOT_MAX_ITEMS / days_range, y_start, plot_height, TFT_DARKGREY);
        }
        sprite_total.setCursor(x_start, y_start + plot_height + 5);
        sprite_total.println("1 Woche");
    }

    else if (days_range < 30) {
        int weeks = days_range / 7;
        for (int i = 1; i <= weeks; i++) {
            int x_pos = x_start + DATALOG_PLOT_MAX_ITEMS - i * (7 * DATALOG_PLOT_MAX_ITEMS / days_range);
            sprite_total.drawFastVLine(x_pos, y_start, plot_height, TFT_DARKGREY);
        }
        sprite_total.setCursor(x_start, y_start + plot_height + 5);
        sprite_total.println(String(weeks) + " Wochen");
    }

    else if (days_range == 30) {
        for (int i = 1; i <= 4; i++) {
            int x_pos = x_start + DATALOG_PLOT_MAX_ITEMS - i * (7 * DATALOG_PLOT_MAX_ITEMS / days_range);
            sprite_total.drawFastVLine(x_pos, y_start, plot_height, TFT_DARKGREY);
        }
        sprite_total.setCursor(x_start, y_start + plot_height + 5);
        sprite_total.println("1 Monat");
    }

    else {
        int months = days_range / 30;
        for (int i = 1; i <= months; i++) {
            int x_pos = x_start + DATALOG_PLOT_MAX_ITEMS - i * (30 * DATALOG_PLOT_MAX_ITEMS / days_range);
            sprite_total.drawFastVLine(x_pos, y_start, plot_height, TFT_DARKGREY);
        }
        sprite_total.setCursor(x_start, y_start + plot_height + 5);
        sprite_total.println(String(months) + " Monate");
    }

    sprite_total.drawFastHLine(x_start, y_start + plot_height - map(25, 0, 100, 0, plot_height), DATALOG_PLOT_MAX_ITEMS + 4, TFT_DARKGREY);
    sprite_total.drawFastHLine(x_start, y_start + plot_height - map(50, 0, 100, 0, plot_height), DATALOG_PLOT_MAX_ITEMS + 4, TFT_DARKGREY);
    sprite_total.drawFastHLine(x_start, y_start + plot_height - map(75, 0, 100, 0, plot_height), DATALOG_PLOT_MAX_ITEMS + 4, TFT_DARKGREY);

    

    sprite_total.drawRect(x_start, y_start, DATALOG_PLOT_MAX_ITEMS, plot_height, TFT_WHITE);
    
    sprite_total.setCursor(x_start + DATALOG_PLOT_MAX_ITEMS - sprite_total.textWidth("0") / 2, y_start + plot_height + 5);
    sprite_total.println("0");
    sprite_total.setCursor(x_start + DATALOG_PLOT_MAX_ITEMS + 5, y_start + plot_height / 2 - sprite_total.fontHeight() / 2);
    sprite_total.println("%");


    int y_prev = 0;

    for (int i = 0; i < DATALOG_PLOT_MAX_ITEMS; i++) {
        int x = x_start + i;
        int y = y_start + plot_height - map(plot_data[DATALOG_PLOT_MAX_ITEMS - 1 - i], 0, 100, 0, plot_height);
        
        if (i > 0)
            sprite_total.drawLine(x - 1, y_prev, x, y, TFT_YELLOW);
        y_prev = y;
    }
   
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