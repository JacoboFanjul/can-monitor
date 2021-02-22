/** @file sensor.h
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 2019
 * Ikerlan
 */

#ifndef _SENSOR_H
#define _SENSOR_H

/* Includes */
#include "../common/parson.h"
#include "../common/rest-server.h"


// JSON Keys
#define JSON_KEY_SENSOR_CONF_ID "sensor-id"
#define JSON_KEY_SENSOR_CONF_NAME "sensor-name"
#define JSON_KEY_SENSOR_CONF_TYPE "sensor-type"
#define JSON_KEY_SENSOR_CONF_SETTINGS "sensor-settings"
#define JSON_KEY_SENSOR_CONF_KEY "key"
#define JSON_KEY_SENSOR_CONF_VALUE "value"
#define JSON_KEY_SENSOR_CONF_RATE "sampling-rate"

/* Structs */
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

// Sensors Configuration
int create_sensors_configuration(char **values);
int update_sensors_configuration(char **values, query_pairs *queries);
int read_sensors_configuration(char **readings);
int delete_sensors_configuration(char **values, query_pairs *queries);

#endif
