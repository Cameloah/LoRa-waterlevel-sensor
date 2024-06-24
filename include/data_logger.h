#include "Arduino.h"
#include "memory_module.h"
#include "main.h"

typedef uint8_t dataItem_t;
typedef uint16_t dataStamp_t;

#define DATALOG_MAX_ITEMS                                       30000               // 30KB


extern dataItem_t logdata_tank_1[];
extern dataStamp_t logstamp_tank_1[];

extern dataItem_t logdata_tank_2[];
extern dataStamp_t logstamp_tank_2[];


void data_logger_init();
void data_logger_save(int tankid, uint8_t data, unsigned long int timestamp);
