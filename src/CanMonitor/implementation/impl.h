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

/* Default values */

#define CAN_INTERFACE "can0"
#define DEF_GW_NAME "Target-1"
#define DEF_SVC_NAME "adeptness-service"
#define DEF_MAX_JSON_SIZE 1000
#define DEF_MAX_GW_ID_SIZE 50
#define DEF_BUFF_SIZE 4096

// JSON Keys
#define JSON_KEY_SETUP_ENDPOINT_TYPE "endpoint-type"
#define JSON_KEY_SETUP_ENDPOINT_CONFIG "endpoint-config"
#define JSON_KEY_SETUP_IP "ip"
#define JSON_KEY_SETUP_PORT "port"
#define JSON_KEY_SETUP_QOS "qos"
#define JSON_KEY_SETUP_BASE_TOPIC "base-topic"

#define JSON_KEY_CONNECTION_CONF_ID "id"
#define JSON_KEY_CONNECTION_CONF_TYPE "monitor-agent-type"
#define JSON_KEY_CONNECTION_CONF_SETTINGS "connection-settings"
#define JSON_KEY_CONNECTION_CONF_KEY "key"
#define JSON_KEY_CONNECTION_CONF_VALUE "value"
    
#define JSON_KEY_MONITORING_AGENT_STATUS "status"
#define JSON_KEY_CMD_EXECUTE_ORDER "order"

#define JSON_KEY_ERROR_MESSAGE "message"

/* Structs */
typedef struct myAdeptnessService_state
{
    char *svcname;
    char gw_id[DEF_MAX_GW_ID_SIZE];
    char json[DEF_MAX_JSON_SIZE];
    uint64_t logical_data;
    pid_t main_thread_pid;
} myAdeptnessService_state;

typedef enum {
    unconfigured,
    configured,
    running,
    error,
    exit_ms
} ms_status;

int can_read(int can_socket, struct can_frame *frame);

// /* Service callbacks functions */

void myAdeptnessService_stop(void *impl, bool force);

// Helper function to create error messsages
void create_error_message(char **values, char *message);

// Setup
int update_microservice_configuration(char **values);

// Connection Configuration
int read_connection_configuration(char **readings);
int update_connection_configuration(char **values);

// Monitoring agent execution
int read_monitoring_agent_status(char **readings);
int cmd_execute_configuration(char **values);

#endif
