/** @file impl.h
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 2019
 * Ikerlan
 */

#ifndef _ADEPTNESS_MYSERVICE_H
#define _ADEPTNESS_MYSERVICE_H

/* Includes */
#include "../common/adeptnessSvc.h"
#include "../common/parson.h"

#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/can.h>

/* Macro */
#define ERR_CHECK(x)                                          \
    if (x.code)                                               \
    {                                                         \
        fprintf(stderr, "Error: %d: %s\n", x.code, x.reason); \
        adeptness_service_free(service);                      \
        return x.code;                                        \
    }

/* Default values */

#define CAN_INTERFACE "can0"
#define DEF_GW_NAME "Target-1"
#define DEF_SVC_NAME "adeptness-service"
// TODO DELETE POLLING INTERVAL
#define DEF_POLLING_INTERVAL_S 5
#define DEF_MAX_JSON_SIZE 1000
#define DEF_MAX_GW_ID_SIZE 50
#define DEF_BUFF_SIZE 4096

// JSON Keys
#define JSON_KEY_SETUP_ENDPOINT_TYPE "endpoint-type"
#define JSON_KEY_SETUP_IP "ip"
#define JSON_KEY_SETUP_PORT "port"
#define JSON_KEY_SETUP_QOS "qos"

#define JSON_KEY_CONNECTION_CONF_ID "id"
#define JSON_KEY_CONNECTION_CONF_TYPE "monitor-agent-type"
#define JSON_KEY_CONNECTION_CONF_SETTINGS "connection-settings"
#define JSON_KEY_CONNECTION_CONF_KEY "key"
#define JSON_KEY_CONNECTION_CONF_VALUE "value"

#define JSON_KEY_SENSOR_CONF_ID "sensor-id"
#define JSON_KEY_SENSOR_CONF_NAME "sensor-name"
#define JSON_KEY_SENSOR_CONF_TYPE "sensor-type"
#define JSON_KEY_SENSOR_CONF_SETTINGS "sensor-settings"
#define JSON_KEY_SENSOR_CONF_KEY "key"
#define JSON_KEY_SENSOR_CONF_VALUE "value"
#define JSON_KEY_SENSOR_CONF_RATE "sampling-rate"

#define JSON_KEY_SENSORGROUPS_CONF_ID "sensorgroup-id"
#define JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE "publish-rate"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST "sensor-list"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID "sensor-id"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME "sensor-name"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE "sensor-type"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS "sensor-settings"
#define JSON_KEY_SENSORGROUPS_CONF_KEY "key"
#define JSON_KEY_SENSORGROUPS_CONF_VALUE "value"
#define JSON_KEY_SENSORGROUPS_CONF_SAMPLING_RATE "sampling-rate"
    
#define JSON_KEY_MONITORING_AGENT_STATUS "status"
#define JSON_KEY_CMD_EXECUTE_ORDER "order"

#define JSON_KEY_SENSOR_MEASUREMENTS_ID "sensor-id"
#define JSON_KEY_SENSOR_MEASUREMENTS_DATA "sensor-data"
#define JSON_KEY_SENSOR_MEASUREMENTS_NAME "name"
#define JSON_KEY_SENSOR_MEASUREMENTS_TYPE "value-type"
#define JSON_KEY_SENSOR_MEASUREMENTS_VALUE "value"
#define JSON_KEY_SENSOR_MEASUREMENTS_TIMESTAMP "timestamp"

#define JSON_KEY_ERROR_MESSAGE "message"

/* Structs */
typedef struct myAdeptnessService_state
{
    char *svcname;
    char gw_id[DEF_MAX_GW_ID_SIZE];
    char json[DEF_MAX_JSON_SIZE];
    uint64_t logical_data;
    unsigned int polling_interval;
    pid_t main_thread_pid;
} myAdeptnessService_state;

typedef struct sensor
{
    char *id;
    char *name;
    char *type;
    uint32_t can_id;
    uint32_t init_bit;
    uint32_t end_bit;
    uint32_t sampling_rate;
    char *value;
    uint64_t timestamp;
} sensor;

typedef struct sensorgroup
{
    char *id;
    uint32_t publish_rate;
    size_t sensorcount;
    char **sensor_list;
    struct timeval last_publish_time;
} sensorgroup;

typedef enum {
    unconfigured,
    configured,
    running,
    error,
    exit_ms
} ms_status;

bool adeptness_get_random(void *impl);

int can_read(int can_socket, struct can_frame *frame);

// /* Service callbacks functions */

void myAdeptnessService_stop(void *impl, bool force);

// Helper function to create error messsages
void create_error_message(char **values, char *message);

int update_microservice_configuration(char **values);

int read_connection_configuration(char **readings);
int update_connection_configuration(char **values);

int create_sensors_configuration(char **values);
int update_sensors_configuration(char **values, query_pairs *queries);
int read_sensors_configuration(char **readings);
int delete_sensors_configuration(char **values, query_pairs *queries);

int create_sensorgroups_configuration(char **values);
int update_sensorgroups_configuration(char **values, query_pairs *queries);
int read_sensorgroups_configuration(char **readings);
int delete_sensorgroups_configuration(char **values, query_pairs *queries);

int read_monitoring_agent_status(char **readings);
int cmd_execute_configuration(char **values);

int read_sensor_measurements(char **readings, query_pairs *queries);

#endif
