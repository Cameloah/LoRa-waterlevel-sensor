#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "pin_config.h"

/* The product now has two screens, and the initialization code needs a small change in the new version. The LCD_MODULE_CMD_1 is used to define the
 * switch macro. */
#define LCD_MODULE_CMD_1^


// ---------------------- SETTINGS ---------------------- //

#define ROTATION                                    0

#if ROTATION == 0 || ROTATION == 2
    #define SPRITE_WIDTH                            TFT_WIDTH
    #define SPRITE_HEIGHT                           TFT_HEIGHT
#else
    #define SPRITE_WIDTH                            TFT_HEIGHT
    #define SPRITE_HEIGHT                           TFT_WIDTH
#endif

#define BG_COLOR                                    tft.color565(40, 41, 69)

#define BAR1_COLOR                                  TFT_YELLOW
#define BAR1_LOCATION_H                             27          // horizontal location of the first bar in percentage
#define BAR2_LOCATION_H                             73          // horizontal location of the second bar in percentage
#define BAR_PADDING                                 15           // padding around the bars in percentage
#define BAR_WIDTH                                   40          // width of the bars in pixels





void display_tft_init();
void display_tft_levels(int tank1_volume, int tank2_volume);
void display_tft_error(int elapsedTime);