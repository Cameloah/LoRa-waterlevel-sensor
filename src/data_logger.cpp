#include "data_logger.h"


MemoryModule datalog_tank_1("tank_1");
MemoryModule datalog_tank_2("tank_2");


dataItem_t logdata_tank_1[DATALOG_MAX_ITEMS];
dataStamp_t logstamp_tank_1[DATALOG_MAX_ITEMS];

dataItem_t logdata_tank_2[DATALOG_MAX_ITEMS];
dataStamp_t logstamp_tank_2[DATALOG_MAX_ITEMS];


void data_logger_init() {
    memset(logdata_tank_1, 0, DATALOG_MAX_ITEMS);
    memset(logstamp_tank_1, 0, sizeof(dataStamp_t) *  DATALOG_MAX_ITEMS);

    memset(logdata_tank_2, 0, DATALOG_MAX_ITEMS);
    memset(logstamp_tank_2, 0, sizeof(dataStamp_t) *  DATALOG_MAX_ITEMS);

    datalog_tank_1.addParameter("tank_id", 1);
    datalog_tank_1.addParameter("data_index", 0);

    datalog_tank_2.addParameter("tank_id", 2);
    datalog_tank_2.addParameter("data_index", 0);

    datalog_tank_1.loadAll();
    datalog_tank_2.loadAll();

    datalog_tank_1.loadRaw("data", logdata_tank_1, DATALOG_MAX_ITEMS);
    datalog_tank_1.loadRaw("timestamp", (uint8_t*) logstamp_tank_1, sizeof(dataStamp_t) * DATALOG_MAX_ITEMS);

    datalog_tank_2.loadRaw("data", logdata_tank_2, DATALOG_MAX_ITEMS);
    datalog_tank_2.loadRaw("timestamp", (uint8_t*) logstamp_tank_2, sizeof(dataStamp_t) * DATALOG_MAX_ITEMS);
}

void data_logger_save(int tankid, uint8_t data, unsigned long int timestamp) {
    int index = 0;

    switch (tankid) {
        case 1:
            index = *datalog_tank_1.getInt("data_index");
            index = (index + 1) % DATALOG_MAX_ITEMS;
            datalog_tank_1.set("data_index", index, true);

            logdata_tank_1[index] = data;
            logstamp_tank_1[index] = timestamp;

            datalog_tank_1.saveRaw("data", logdata_tank_1, DATALOG_MAX_ITEMS);
            datalog_tank_1.saveRaw("timestamp", (uint8_t*) logstamp_tank_1, sizeof(dataStamp_t) * DATALOG_MAX_ITEMS);
            break;

        case 2:
            index = *datalog_tank_2.getInt("data_index");
            index = (index + 1) % DATALOG_MAX_ITEMS;
            datalog_tank_2.set("data_index", index, true);

            logdata_tank_2[index] = data;
            logstamp_tank_2[index] = timestamp;

            datalog_tank_2.saveRaw("data", logdata_tank_2, DATALOG_MAX_ITEMS);
            datalog_tank_2.saveRaw("timestamp", (uint8_t*) logstamp_tank_2, sizeof(dataStamp_t) * DATALOG_MAX_ITEMS);
            break;
        
        default:
            break;
    }
}

uint8_t plot_data_1[DATALOG_PLOT_MAX_ITEMS];
uint8_t plot_data_2[DATALOG_PLOT_MAX_ITEMS];

void prepare_plot_data(int tank_id, int days_range) {

    uint8_t* plot_data = (tank_id == 1) ? plot_data_1 : plot_data_2;
    dataItem_t* logdata_tank = (tank_id == 1) ? logdata_tank_1 : logdata_tank_2;
    dataStamp_t* logstamp_tank = (tank_id == 1) ? logstamp_tank_1 : logstamp_tank_2;
    MemoryModule* datalog_tank = (tank_id == 1) ? &datalog_tank_1 : &datalog_tank_2;

    memset(plot_data, 0, DATALOG_PLOT_MAX_ITEMS);

    int max_minutes = days_range * 24 * 60;
    int interval = max_minutes / DATALOG_PLOT_MAX_ITEMS;
    int data_index = *datalog_tank->getInt("data_index");
    
    plot_data[0] = logdata_tank[data_index];
    int plot_index = 1;
    dataStamp_t latest_timestamp = logstamp_tank[data_index];

    for (int i = 0; i < DATALOG_MAX_ITEMS; i++) {

        int index = data_index - i;
        if (index < 0) {
            index += DATALOG_MAX_ITEMS;
        }

        // Serial.println("index: " + String(index) + ", Stamp: " + String(logstamp_tank[index]) + ", data: " + String(logdata_tank[index]));

        if (logstamp_tank[index] == 0)
            continue;

        int increments = (latest_timestamp - logstamp_tank[index]) / interval;

        if (logstamp_tank[index] > latest_timestamp)
            continue;

        if (increments > 0) {
            plot_data[plot_index] = logdata_tank[index];
            plot_index += increments;
            latest_timestamp = logstamp_tank[index];
        }      
    }
}