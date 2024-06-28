
#include <Arduino.h>

#include "main.h"


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

static unsigned long lastMeasurementTime = 0;



// ---------------------- DATA ---------------------- //

remoteSensor_t sensor_1;
remoteSensor_t sensor_2;

bool flag_warning_displayed = false;
unsigned long last_warning_time = 0;

int calculate_liters(int measurement) {
  measurement = measurement < TANK_LVL_FULL_CM ? TANK_LVL_FULL_CM : measurement;
  measurement = measurement > TANK_LVL_EMPTY_CM ? TANK_LVL_EMPTY_CM : measurement;

  int tank_level = TANK_LVL_EMPTY_CM - measurement;
  float tank_volume = PI / 4 * TANK_DIAMETER_CM * TANK_DIAMETER_CM * tank_level;
  return tank_volume / 1000; // in liters
}

void request_sensor_data(remoteSensor_t &sensor);
void print_error(remoteSensor_t &sensor);
void update_sensor_data();
void warning_full();
void warning_low_batt();



// ---------------------- SETUP ---------------------- //

void setup() {
  Serial.begin(115200);
  Serial.println("Startup, waiting for data...");

  pinMode(GPIO_NUM_14, INPUT);

  e220ttl.begin();
  e220ttl.setMode(MODE_1_WOR_TRANSMITTER);

  display_init();

  data_logger_init();

  // --------- initialize data buffers --------- //
  memset(&sensor_1, 0x00, sizeof(sensor_1));
  memset(&sensor_2, 0x00, sizeof(sensor_2));
  sensor_1.address_L = 3;
  sensor_2.address_L = 4;
  sensor_1.sensor_id = 1;
  sensor_2.sensor_id = 2;
  sensor_1.full_liters = calculate_liters(TANK_LVL_FULL_CM);
  sensor_2.full_liters = calculate_liters(TANK_LVL_FULL_CM);
  sensor_1.error = SENSOR_ERROR_NONE;
  sensor_2.error = SENSOR_ERROR_NONE;

  // --------- request initial data --------- //
  display_msg_box(SPRITE_WIDTH / 2, SPRITE_HEIGHT / 2, SPRITE_WIDTH - 5, "Frage Sensordaten ab...", true, TFT_BLACK);
  display_update();
  request_sensor_data(sensor_1);
  request_sensor_data(sensor_2);
}
 


// ---------------------- LOOP ---------------------- //
int page_index = 0;
unsigned long timestamp_advanced_page = 0;
bool button_pressed = false;

void loop() {
  // ------------------ pull measurement ------------------ //
  if (millis() - lastMeasurementTime >= MEASUREMENT_INTERVAL) {
    request_sensor_data(sensor_1);
    request_sensor_data(sensor_2);
  }


  if(digitalRead(GPIO_NUM_14) == LOW && !button_pressed) {
    unsigned long pressTime = millis();
    while (digitalRead(GPIO_NUM_14) == LOW) {
      if (millis() - pressTime > 1000) {
        display_msg_box(SPRITE_WIDTH / 2, SPRITE_HEIGHT / 2, SPRITE_WIDTH - 5, "Frage Sensordaten ab...", true, TFT_BLACK);
        display_update();
        request_sensor_data(sensor_1);
        request_sensor_data(sensor_2);
        button_pressed = true;
        return;
      }
      delay(10);
    }

    // state machine trigger, switch to advanced display
    page_index++;
    timestamp_advanced_page = millis();
    button_pressed = true;    
  }

  if(digitalRead(GPIO_NUM_14) == HIGH)
    button_pressed = false;

  if(millis() - timestamp_advanced_page > TIME_ADVANCED_PAGE)
    page_index = 0;
  

  switch (page_index)
  {
  case 0:
    display_levels((uint8_t*) &sensor_1, (uint8_t*) &sensor_2);
    warning_full();

    if ((millis() / 2000) % 2)
      warning_low_batt();

    display_error((uint8_t*) &sensor_1);
    display_error((uint8_t*) &sensor_2);
    break;

  case 1:
    display_advanced_page();
    break;

  case 2:
    display_plot1d_page();
    break;

  case 3:
    display_plot1w_page();
    break;

  case 4:
    display_plot1m_page();
    break;

  case 5:
    display_plot3m_page();
    break;

  default:
    page_index = 0;
  }
  
  display_update();
}



void warning_full() {
  if (sensor_1.error == SENSOR_ERROR_NONE && sensor_1.volume_liters > sensor_1.full_liters - TANK_LITERS_RESERVE)
    display_msg_box(SPRITE_WIDTH / 4, SPRITE_HEIGHT / 2, SPRITE_WIDTH / 2 - 4, "VOLL!", true, TFT_RED);

  if (sensor_2.error == SENSOR_ERROR_NONE && sensor_2.volume_liters > sensor_2.full_liters - TANK_LITERS_RESERVE)
    display_msg_box(SPRITE_WIDTH * 3 / 4, SPRITE_HEIGHT / 2, SPRITE_WIDTH / 2 - 4, "VOLL!", true, TFT_RED);
}

void warning_low_batt() {
  if ((sensor_1.error == SENSOR_ERROR_NONE || sensor_1.error == SENSOR_ERROR_INVALID_DATA) && sensor_1.data.battery_voltage < BATTERY_LOW_VOLTAGE)
    display_msg_box(SPRITE_WIDTH / 4, SPRITE_HEIGHT / 2, SPRITE_WIDTH / 2 - 4, "Akku schwach!", true, TFT_BLACK);

  if ((sensor_2.error == SENSOR_ERROR_NONE || sensor_2.error == SENSOR_ERROR_INVALID_DATA) && sensor_2.data.battery_voltage < BATTERY_LOW_VOLTAGE)
    display_msg_box(SPRITE_WIDTH * 3 / 4, SPRITE_HEIGHT / 2, SPRITE_WIDTH / 2 - 4, "Akku schwach!", true, TFT_BLACK);
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
      rsc = e220ttl.receiveMessageRSSI(sizeof(sensor.data));
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
  sensor.rssi = rsc.rssi;

  if (sensor.data.tank_id != sensor.sensor_id) {
    sensor.error = SENSOR_ERROR_ID;
    return;
  }

  if (sensor.data.tank_measurement == -1) {
    sensor.error = SENSOR_ERROR_INVALID_DATA;
    return;
  }

  sensor.error = SENSOR_ERROR_NONE;

  sensor.volume_liters = calculate_liters(sensor.data.tank_measurement);
  sensor.volume_percent = sensor.volume_liters * 100 / sensor.full_liters;

  data_logger_save(sensor.sensor_id, sensor.volume_percent, sensor.data.timestamp);
  Serial.println("Tank " + String(sensor.sensor_id) + " Data: " + String(sensor.volume_liters) + "L, " + String(sensor.volume_percent) + "%, " + String(sensor.data.battery_voltage) + "mV");
}

void print_error(remoteSensor_t &sensor) {
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
}