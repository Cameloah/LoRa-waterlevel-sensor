#include "Arduino.h"
#include "memory_module.h"
#include "main.h"

typedef uint8_t dataItem_t;
typedef uint32_t dataStamp_t;

#define DATALOG_MAX_ITEMS                                       20000               // 30KB
#define DATALOG_PLOT_MAX_ITEMS                                  150

extern dataItem_t logdata_tank_1[];
extern dataStamp_t logstamp_tank_1[];

extern dataItem_t logdata_tank_2[];
extern dataStamp_t logstamp_tank_2[];

extern uint8_t plot_data_1[];
extern uint8_t plot_data_2[];


void data_logger_init();
void data_logger_save(int tankid, uint8_t data, unsigned long int timestamp);
void prepare_plot_data(int tank_id, int days_range);