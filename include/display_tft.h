#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "pin_config.h"
#include "main.h"

/* The product now has two screens, and the initialization code needs a small change in the new version. The LCD_MODULE_CMD_1 is used to define the
 * switch macro. */
#define LCD_MODULE_CMD_1


// ---------------------- SETTINGS ---------------------- //

#define ROTATION                                    0

#if ROTATION == 0 || ROTATION == 2
    #define SPRITE_WIDTH                            TFT_WIDTH
    #define SPRITE_HEIGHT                           TFT_HEIGHT
#else
    #define SPRITE_WIDTH                            TFT_HEIGHT
    #define SPRITE_HEIGHT                           TFT_WIDTH
#endif

#define BG_COLOR                                    TFT_BLACK

#define BAR_COLOR_EMPTY                             TFT_GREEN
#define BAR_COLOR_MEDIUM                            TFT_YELLOW
#define BAR_COLOR_FULL                              tft.color565(200, 0, 0)
#define BAR1_LOCATION_H                             27          // horizontal location of the first bar in percentage
#define BAR2_LOCATION_H                             73          // horizontal location of the second bar in percentage
#define BAR_PADDING                                 15          // padding around the bars in percentage
#define BAR_WIDTH                                   40          // width of the bars in pixels
#define BAR_PADDING_INNER                           2           // padding inside the bars in pixels
#define ARROW_SIZE                                  5           // pixel


void display_init();
void display_update();
void display_levels(uint8_t *sensor_buffer1, uint8_t *sensor_buffer2);
void display_error(uint8_t *sensor_buffer);
void display_msg_box(int center_x, int center_y, int width, String msg, bool center, uint16_t color);
void display_advanced_page();