#ifndef _REST_SERVER_H
#define _REST_SERVER_H

#include "../common/adeptnessSvc.h"

// URLs
#define URL_SETUP        "setup"
#define URL_CONNECTION   "monitoring-agent/config/connection"
#define URL_SENSORS_CONF "monitoring-agent/config/sensors"
#define URL_SENSORGROUPS "monitoring-agent/config/sensorgroups"
#define URL_AGENT_STATUS "monitoring-agent/monitoring-agent-status"
#define URL_CMD_EXECUTE  "monitoring-agent/cmd-execute"
#define URL_SENSORS_VAL  "monitoring-agent/sensors"

// List of query keys
#define QUERY_KEY_ID "id"
#define QUERY_KEYS { QUERY_KEY_ID }


// /* Service callbacks functions */
adeptness_rest_response myAdeptnessService_get_handler(void *impl, const char *devname, char *url, char **readings, query_pairs *queries);
adeptness_rest_response myAdeptnessService_put_handler(void *impl, const char *devname, char *url, char **values, query_pairs *queries);
adeptness_rest_response myAdeptnessService_post_handler(void *impl, const char *devname, char *url, char **values, query_pairs *queries);
adeptness_rest_response myAdeptnessService_delete_handler(void *impl, const char *devname, char *url, char **values, query_pairs *queries);

#endif
