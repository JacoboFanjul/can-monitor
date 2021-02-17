#ifndef _CAN_MONITOR_H
#define _CAN_MONITOR_H

#include "implementation/hash_table.h"
#include "implementation/impl.h"

// Aux for dev only:
#define EXISTS_CAN 0
#define DEV 1
#define SENSORS_AVAILABLE 0
#define AUTOSTART 1

// static char *mqtt_discovery_topic = "adms/v1/discovery";
// static char *mqtt_data_topic = "adms/v1/monitor-agent/data";

//enum ms_status{ready, configured, running, error, exit_ms};

extern ms_status status;

extern HashTableSensors *sensors_table;
extern HashTableSensorgroups *sensorgroup_table;

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


int getInfoFromEnvironmentVariables (void);

int read_config_file(char *config_file);

// TODO Delete
#if DEV
void print_sensorgroup(sensorgroup *sensorgroup);
void print_sensor(sensor *sensor);
void create_dummy_struct(void);
void print_struct(void);
void create_test_struct(void);
int print_test_table(void);
#endif

#endif
