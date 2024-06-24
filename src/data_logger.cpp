#include "data_logger.h"


MemoryModule datalog_tank_1;
MemoryModule datalog_tank_2;


dataItem_t logdata_tank_1[DATALOG_MAX_ITEMS];
dataStamp_t logstamp_tank_1[DATALOG_MAX_ITEMS];

dataItem_t logdata_tank_2[DATALOG_MAX_ITEMS];
dataStamp_t logstamp_tank_2[DATALOG_MAX_ITEMS];


void data_logger_init() {
    memset(logdata_tank_1, 0, DATALOG_MAX_ITEMS);
    memset(logstamp_tank_1, 0, DATALOG_MAX_ITEMS);

    memset(logdata_tank_2, 0, DATALOG_MAX_ITEMS);
    memset(logstamp_tank_2, 0, DATALOG_MAX_ITEMS);

    datalog_tank_1.addParameter("tank_id", 1);
    datalog_tank_1.addParameter("data_index", 0);

    datalog_tank_2.addParameter("tank_id", 2);
    datalog_tank_2.addParameter("data_index", 0);

    datalog_tank_1.loadAll();
    datalog_tank_1.loadRaw("data", logdata_tank_1, DATALOG_MAX_ITEMS);
    datalog_tank_1.loadRaw("timestamp", (uint8_t*) logstamp_tank_1, DATALOG_MAX_ITEMS);

    datalog_tank_2.loadAll();
    datalog_tank_2.loadRaw("data", logdata_tank_2, DATALOG_MAX_ITEMS);
    datalog_tank_2.loadRaw("timestamp", (uint8_t*) logstamp_tank_2, DATALOG_MAX_ITEMS);
}

void data_logger_save(int tankid, uint8_t data, unsigned long int timestamp) {
    int index = 0;
    dataStamp_t timestamp_rel = 0;

    switch (tankid) {
    case 1:
        index = *datalog_tank_1.getInt("data_index");
        index = (index + 1) % DATALOG_MAX_ITEMS;
        datalog_tank_1.set("data_index", index, true);

        // base timestamp on smallest timestamp to conserve memory
        timestamp_rel = timestamp - logstamp_tank_1[index];
        logdata_tank_1[index] = data;
        logstamp_tank_1[index] = timestamp_rel;

        datalog_tank_1.saveRaw("data", logdata_tank_1, DATALOG_MAX_ITEMS);
        datalog_tank_1.saveRaw("timestamp", (uint8_t*) logstamp_tank_1, DATALOG_MAX_ITEMS);
        break;

    case 2:
        index = *datalog_tank_2.getInt("data_index");
        index = (index + 1) % DATALOG_MAX_ITEMS;
        datalog_tank_2.set("data_index", index, true);

        timestamp_rel = timestamp - logstamp_tank_2[index];
        logdata_tank_2[index] = data;
        logstamp_tank_2[index] = timestamp_rel;

        datalog_tank_2.saveRaw("data", logdata_tank_2, DATALOG_MAX_ITEMS);
        datalog_tank_2.saveRaw("timestamp", (uint8_t*) logstamp_tank_2, DATALOG_MAX_ITEMS);
        break;
    
    default:
        break;
    }

}