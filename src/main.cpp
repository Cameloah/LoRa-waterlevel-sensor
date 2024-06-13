
#include <Arduino.h>
#include "LoRa_E220.h"
#include "TFT_eSPI.h"

#include "pin_config.h"
// #include "display.h"
#include "display_tft.h"



// ---------------------- LORA ---------------------- //
#define PIN_ESP_UART1_RX                                  18
#define PIN_ESP_UART2_TX                                  17
#define PIN_E220_AUX                                      21
#define PIN_E220_M0                                       13
#define PIN_E220_M1                                       12

LoRa_E220 e220ttl(PIN_ESP_UART2_TX,
                  PIN_ESP_UART1_RX,
                  &Serial1,
                  PIN_E220_AUX,
                  PIN_E220_M0,
                  PIN_E220_M1,
                  UART_BPS_RATE_9600);
 
 

// ---------------------- SYSTEM ---------------------- //

#define EXPECTED_TIME                                    2400 // 2 seconds + 20% (400 milliseconds)



// ---------------------- DATA ---------------------- //

struct tankData {
  int tank1_measurement;
  int tank2_measurement;
};

#define TANK_DIAMETER_CM                                  200 // in cm
#define TANK_LVL_EMPTY_CM                                 400 // in cm
#define TANK_LVL_FULL_CM                                  20 // in cm

int calculate_liters(int measurement) {
  measurement = measurement < TANK_LVL_FULL_CM ? TANK_LVL_FULL_CM : measurement;
  measurement = measurement > TANK_LVL_EMPTY_CM ? TANK_LVL_EMPTY_CM : measurement;

  int tank_level = TANK_LVL_EMPTY_CM - measurement;
  float tank_volume = PI / 4 * TANK_DIAMETER_CM * TANK_DIAMETER_CM * tank_level;
  return tank_volume / 1000; // in liters
}



// ---------------------- SETUP ---------------------- //

void setup() {
  Serial.begin(9600);
  delay(1500);
 
  Serial.println("Startup, waiting for data...");

  e220ttl.begin();


  display_tft_init();
}
 


// ---------------------- LOOP ---------------------- //

void loop() {

  // -------------- display -------------- //
  

  // -------------- lora -------------- //
  static unsigned long lastMessageTime = 0;
  unsigned long currentTime = millis();

  if (e220ttl.available() > 1) {
    ResponseStructContainer rsc = e220ttl.receiveMessage(sizeof(tankData));
    tankData currentTankData = *(tankData*) rsc.data;

    int tank1_volume = calculate_liters(currentTankData.tank1_measurement);
    int tank2_volume = calculate_liters(currentTankData.tank2_measurement);

    Serial.print("Waste level tank 1  [L]: ");
    Serial.println(tank1_volume);
    Serial.print("Waste level tank 2  [L]: ");
    Serial.println(tank2_volume);

    display_tft_levels(tank1_volume, tank2_volume);

    lastMessageTime = currentTime;
    Serial.println();
  } 
  
  else {
    unsigned long elapsedTime = currentTime - lastMessageTime;

    if (elapsedTime > EXPECTED_TIME) {
      display_tft_error(elapsedTime);      
      delay(1000);
    }
  }
}