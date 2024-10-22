#include "Arduino.h"
#include "LoRa_E220.h"
#include "lora_config.h"
#include "measure.h"

#include "soc/rtc_cntl_reg.h"
#include "soc/rtc.h"
#include "driver/rtc_io.h"
#include "esp_adc_cal.h"


// --------------------------------- battery level --------------------------------- //
#define PIN_BATT_ADC                                      ADC1_CHANNEL_6
#define BATT_FULL_MV                                      4300 // milli volt
#define BATT_EMPTY_MV                                     3700  // milli volt

int _readBattery() {
  uint32_t value = 0;
  int rounds = 11;
  esp_adc_cal_characteristics_t adc_chars;

  //battery voltage divided by 2 can be measured at GPIO34, which equals ADC1_CHANNEL6
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(PIN_BATT_ADC, ADC_ATTEN_DB_11);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  //to avoid noise, sample the pin several times and average the result
  for(int i=1; i<=rounds; i++)
    value += adc1_get_raw(PIN_BATT_ADC);
  
  value /= (uint32_t)rounds;

  //due to the voltage divider (1M+1M) values must be multiplied by 2
  return esp_adc_cal_raw_to_voltage(value, &adc_chars)*2.0;
}



// --------------------------------- LoRa E220 TTL --------------------------------- //
#define PIN_E220_AUX                                      GPIO_NUM_4   //D12
#define PIN_E220_M0                                       GPIO_NUM_13  //D7   currently not connected
#define PIN_E220_M1                                       GPIO_NUM_12  //D13

#define DESTINATION_ADDL                                  2

HardwareSerial *serial_lora = &Serial2;

LoRa_E220 e220ttl(
    serial_lora,
    PIN_E220_AUX,
    PIN_E220_M0,
    PIN_E220_M1
);



// --------------------------------- APP DATA --------------------------------- //

struct tankData {
  int tank_id;
  int tank_level;
  unsigned long int timestamp;
  int battery_voltage;
};

tankData currentTankData;
 
int _measure_distance();



void setup() {
  // --------------- system setup ------------------- //
  Serial.begin(115200);
  memset(&currentTankData, 0x00, sizeof(currentTankData));

  unsigned long now = time(NULL);
  currentTankData.timestamp = now / 60;

  // ------------- battery level -------------------- //
  currentTankData.battery_voltage = _readBattery();
  Serial.println("Battery level: " + String(currentTankData.battery_voltage) + " mV");


  // ------------ lora send measurement -------------- //
  e220ttl.begin();
  delay(100);

  e220ttl.setMode(MODE_0_NORMAL);

  currentTankData.tank_id = TANK_ID;
  currentTankData.tank_level = get_fill_status();
  ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 18, &currentTankData, sizeof(currentTankData));

  // ------------- go to sleep ---------------------- //
  e220ttl.setMode(MODE_2_WOR_RECEIVER);
  serial_lora->end();

  esp_sleep_enable_ext0_wakeup(PIN_E220_AUX, LOW); 
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  gpio_deep_sleep_hold_en();


  esp_deep_sleep_start();
  delay(100);
  Serial.println("If you see this, the mcu is not sleeping!");
}
 
void loop() {}