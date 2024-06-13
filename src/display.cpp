#include "display.h"

#define GFX_DEV_DEVICE LILYGO_T_DISPLAY_S3
#define GFX_EXTRA_PRE_INIT()              \
    {                                     \
        pinMode(15 /* PWD */, OUTPUT);    \
        digitalWrite(15 /* PWD */, HIGH); \
    }

#define GFX_BL 38

Arduino_DataBus *bus = new Arduino_ESP32PAR8Q (
    7,  // DC
    6,  // CS 
    8,  // WR 
    9,  // RD
    39, // D0
    40, // D1
    41, // D2
    42, // D3
    45, // D4 
    46, // D5
    47, // D6 
    48  // D7
    );

Arduino_GFX *gfx = new Arduino_ST7789 (
    bus,
    5,      // RST
    0,      // rotation
    true,   // IPS
    170,    // width
    320,    // height
    35,     // col offset 1
    0,      // row offset 1
    35,     // col offset 2
    0       // row offset 2
    );



void display_init()
{
    GFX_EXTRA_PRE_INIT();

    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);


    gfx->begin();
    gfx->setRotation(0);
    delay(200);
}


void display_levels(int tank1_volume, int tank2_volume)
{
    gfx->fillScreen(BLACK);
    gfx->setCursor(0, 0);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->println("Waste level tank 1  [L]: ");
    gfx->println(tank1_volume);
    gfx->println("Waste level tank 2  [L]: ");
    gfx->println(tank2_volume);
    delay(1000);
}