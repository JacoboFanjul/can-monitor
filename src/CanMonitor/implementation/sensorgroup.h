/** @file sensorgroup.h
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 2019
 * Ikerlan
 */

#ifndef _SENSORGROUP_H
#define _SENSORGROUP_H

/* Includes */
#include "../common/parson.h"
#include "../common/rest-server.h"

// JSON Keys
#define JSON_KEY_SENSORGROUPS_CONF_ID "sensorgroup-id"
#define JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE "publish-rate"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST "sensor-list"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID "sensor-id"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME "sensor-name"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE "sensor-type"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS "sensor-settings"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_KEY "key"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE "value"
#define JSON_KEY_SENSORGROUPS_CONF_SENSOR_SAMPLING_RATE "sampling-rate"

/* Structs */
typedef struct sensorgroup
{
    char *id;
    uint32_t publish_rate;
    size_t sensorcount;
    char **sensor_list;
    struct timeval last_publish_time;
} sensorgroup;

// Sensorgroup subscription
int create_sensorgroups_subscription(char **values);
int update_sensorgroups_subscription(char **values, query_pairs *queries);
int read_sensorgroups_subscription(char **readings);
int delete_sensorgroups_subscription(char **values, query_pairs *queries);

#endif
