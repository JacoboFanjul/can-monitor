#ifndef _CAN_MONITOR_H
#define _CAN_MONITOR_H

#include "implementation/tables/hashtable_common.h"
#include "implementation/tables/hashtable_sensors.h"
#include "implementation/tables/hashtable_sensorgroups.h"
#include "implementation/tables/table_can.h"
#include "implementation/impl.h"

#define MS_TYPE "monitor-agent"
// TODO delete test part
#define MQTT_API_PREFIX "test/adms/v2"
#define DATA_TOPIC_PREFIX MQTT_API_PREFIX "/" MS_TYPE
#define DISCOVERY_TOPIC MQTT_API_PREFIX "/discovery"

/* Macro */
#define ERR_CHECK(x)                                          \
    if (x.code)                                               \
    {                                                         \
        fprintf(stderr, "Error: %d: %s\n", x.code, x.reason); \
        adeptness_service_free(service);                      \
        return x.code;                                        \
    }

// Aux for dev only:
#define EXISTS_CAN 0
#define DEV 0
#define AUTOSTART 0

extern ms_status status;

extern HashTableSensors *sensors_table;
extern HashTableSensorgroups *sensorgroup_table;
extern TableCan *can_ids_table;

extern int rest_server_port;
extern char *mqtt_broker_host;
extern int mqtt_broker_port;
extern char *monitor_id;
extern char *mqtt_username;
extern int mqtt_qos;

extern uint8_t restart_mqtt;
extern uint8_t restart_http;
extern uint8_t can_up;

extern char *can_conf_id;
extern char *canport;
extern int bitrate;


int get_info_from_environment_variables(void);

int get_info_from_config_file(char *config_file);

void mask_can_frame(void *var_addr, struct can_frame *frame, uint32_t init_bit, uint32_t end_bit);

void parse_can_frame(struct can_frame *frame);

void var_cast(char *var_str, void *var_value, char* type);

// TODO Delete
#if DEV
void print_sensorgroup(sensorgroup *sensorgroup);
void print_sensor(sensor *sensor);
void create_dummy_struct(void);
void print_sensors_table(void);
void print_sensorgroups_table(void);
void print_canid_table(void);
void print_struct(void);
#endif

#endif
