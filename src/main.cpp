
#include <Arduino.h>
#include "main.h"


// ---------------------- LORA ---------------------- //
#define PIN_ESP_UART1_RX                                  18
#define PIN_ESP_UART2_TX                                  17
#define PIN_E220_AUX                                      21
#define PIN_E220_M0                                       13
#define PIN_E220_M1                                       12

#define DESTINATION_ADDL                                  2

LoRa_E220 e220ttl(PIN_ESP_UART2_TX,
                  PIN_ESP_UART1_RX,
                  &Serial1,
                  PIN_E220_AUX,
                  PIN_E220_M0,
                  PIN_E220_M1,
                  UART_BPS_RATE_9600);
 
 

// ---------------------- SYSTEM ---------------------- //

static unsigned long lastMeasurementTime = 0;



// ---------------------- DATA ---------------------- //

remoteSensor_t sensor_1;
remoteSensor_t sensor_2;

int calculate_liters(int measurement) {
  measurement = measurement < TANK_LVL_FULL_CM ? TANK_LVL_FULL_CM : measurement;
  measurement = measurement > TANK_LVL_EMPTY_CM ? TANK_LVL_EMPTY_CM : measurement;

  int tank_level = TANK_LVL_EMPTY_CM - measurement;
  float tank_volume = PI / 4 * TANK_DIAMETER_CM * TANK_DIAMETER_CM * tank_level;
  return tank_volume / 1000; // in liters
}

void request_sensor_data(remoteSensor_t &sensor);
void handle_error(remoteSensor_t &sensor);
void update_sensor_data();
bool warning_full();


// ---------------------- SETUP ---------------------- //

void setup() {
  Serial.begin(9600);
  Serial.println("Startup, waiting for data...");

  pinMode(GPIO_NUM_14, INPUT);

  e220ttl.begin();
  e220ttl.setMode(MODE_1_WOR_TRANSMITTER);

  display_init();

  // --------- initialize data buffers --------- //
  memset(&sensor_1, 0x00, sizeof(sensor_1));
  memset(&sensor_2, 0x00, sizeof(sensor_2));
  sensor_1.address_L = 0x03;
  sensor_2.address_L = 0x04;
  sensor_1.sensor_id = 1;
  sensor_2.sensor_id = 2;
  sensor_1.full_liters = calculate_liters(TANK_LVL_FULL_CM);
  sensor_2.full_liters = calculate_liters(TANK_LVL_FULL_CM);
  sensor_1.error = SENSOR_ERROR_NONE;
  sensor_2.error = SENSOR_ERROR_NONE;

  // --------- request initial data --------- //
  update_sensor_data();
}
 


// ---------------------- LOOP ---------------------- //
bool flag_warning_displayed = false;
unsigned long last_warning_time = 0;
void loop() {
  // ------------------ pull measurement ------------------ //
  unsigned long currentTime = millis();

  if (currentTime - lastMeasurementTime >= MEASUREMENT_INTERVAL)
    update_sensor_data();



  if(digitalRead(GPIO_NUM_14) == LOW) {
    unsigned long pressTime = millis();
    while (digitalRead(GPIO_NUM_14) == LOW) {
      if (millis() - pressTime > 1000) {
        // state machine trigger, switch to advanced display
        display_levels((uint8_t*) &sensor_1, (uint8_t*) &sensor_2);
        display_msg_box(SPRITE_WIDTH / 2, SPRITE_HEIGHT / 2, SPRITE_WIDTH - 5, "Erweiterte Anzeige...", TFT_BLACK);
        delay(2000);
        display_levels((uint8_t*) &sensor_1, (uint8_t*) &sensor_2);
        return;
      }
      delay(10);
    }

    display_levels((uint8_t*) &sensor_1, (uint8_t*) &sensor_2);
    display_msg_box(SPRITE_WIDTH / 2, SPRITE_HEIGHT / 2, SPRITE_WIDTH - 5, "Frage Sensordaten ab...", TFT_BLACK);
    update_sensor_data();
  }

  if (!flag_warning_displayed && millis() - last_warning_time > 1000) {
    flag_warning_displayed = warning_full();
    last_warning_time = millis();
  }

  if (flag_warning_displayed && millis() - last_warning_time > 1000) {
    display_levels((uint8_t*) &sensor_1, (uint8_t*) &sensor_2);
    flag_warning_displayed = false;
    last_warning_time = millis();
  }




  // ------------------ test display ------------------ //

 /*
  int full = calculate_liters(TANK_LVL_FULL_CM);

  int test_vol1 = full / 2 * sin(millis() / 2000.0) + full / 2;
  int test_vol2 = full / 2 * cos(millis() / 2000.0) + full / 2;

  display_levels(test_vol1, test_vol2);*/

}

bool warning_full() {
  bool retval = false;
  if (sensor_1.error == SENSOR_ERROR_NONE && sensor_1.volume_liters > sensor_1.full_liters - TANK_LITERS_RESERVE) {
    display_msg_box(SPRITE_WIDTH / 4, SPRITE_HEIGHT / 2, SPRITE_WIDTH / 2 - 4, "VOLL!", TFT_RED);
    retval = true;
  }

  if (sensor_2.error == SENSOR_ERROR_NONE && sensor_2.volume_liters > sensor_2.full_liters - TANK_LITERS_RESERVE) {
    display_msg_box(SPRITE_WIDTH * 3 / 4, SPRITE_HEIGHT / 2, SPRITE_WIDTH / 2 - 4, "VOLL!", TFT_RED);
    retval = true;
  }

  return retval;
}

void request_sensor_data(remoteSensor_t &sensor) {
  e220ttl.setMode(MODE_1_WOR_TRANSMITTER);
  delay(100);
  
  // wake up the receiver and request data
  ResponseStatus rs = e220ttl.sendFixedMessage(0, sensor.address_L, 18, "Wake up!");
  if (rs.code != E220_SUCCESS) {
    sensor.error = SENSOR_ERROR_SEND;
    return;
  }

  e220ttl.setMode(MODE_0_NORMAL);

  ResponseStructContainer rsc;

  unsigned long currentTime = millis();

  while (true) {
    if (e220ttl.available() > 1) {
      rsc = e220ttl.receiveMessage(sizeof(sensor.data));
      break;
    }

    if (millis() - currentTime > RESPONSE_TIMEOUT) {
      sensor.error = SENSOR_ERROR_TIMEOUT;
      lastMeasurementTime = currentTime;
      return;
    }
  }

  lastMeasurementTime = currentTime;

  if (rsc.status.code != E220_SUCCESS) {
    sensor.error = SENSOR_ERROR_RECEIVE;
    return;
  }

  sensor.data = *(tankData_t*) rsc.data;

  if (sensor.data.tank_id != sensor.sensor_id) {
    sensor.error = SENSOR_ERROR_ID;
    return;
  }

  if (sensor.data.tank_measurement == -1) {
    sensor.error = SENSOR_ERROR_INVALID_DATA;
    return;
  }

  sensor.volume_liters = calculate_liters(sensor.data.tank_measurement);
  sensor.volume_percent = sensor.volume_liters * 100 / sensor.full_liters;
}

void handle_error(remoteSensor_t &sensor) {
  switch (sensor.error) {
    case SENSOR_ERROR_NONE:
      return;
    case SENSOR_ERROR_TIMEOUT:
      Serial.println("Tank " + String(sensor.sensor_id) + " Error: Timeout");
      break;
    case SENSOR_ERROR_INVALID_DATA:
      Serial.println("Tank " + String(sensor.sensor_id) + " Error: Invalid data");
      break;
    case SENSOR_ERROR_SEND:
      Serial.println("Tank " + String(sensor.sensor_id) + " Error: Send request");
      break;
    case SENSOR_ERROR_RECEIVE:
      Serial.println("Tank " + String(sensor.sensor_id) + " Error: Receive");
      break;
    case SENSOR_ERROR_ID:
      Serial.println("Tank " + String(sensor.sensor_id) + " Error: ID mismatch");
      break;
    default:
      Serial.println("Error: Unknown");
  }

  display_error((uint8_t*) &sensor);
}


void update_sensor_data() {
  request_sensor_data(sensor_1);
  request_sensor_data(sensor_2);
  display_levels((uint8_t*) &sensor_1, (uint8_t*) &sensor_2);
  handle_error(sensor_1);
  handle_error(sensor_2);
}