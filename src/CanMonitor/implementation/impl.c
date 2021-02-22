/** @file impl.c
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 2019
 * Ikerlan
 */

/* Include */
#include "impl.h"
#include "../rest_server/rest_server_impl.h"
#include "../mqtt/mqtt_payload_helpers.h"
#include "../CanMonitor.h"

/* Functions */

int can_read(int can_socket, struct can_frame *frame)
{
    // Try to read a complete frame from can interface:
    int nbytes = read(can_socket, frame, sizeof(struct can_frame));

    // Check if we have read a complete frame, and whether it is a standard/extended frame:
    if (nbytes > 0)
    {
        if (frame->can_id & CAN_EFF_FLAG) // Extended Frame Format
        {
            return 2;
        }
        else // Standard Frame Format
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

/**
 * @brief  Stop
 *
 * Stop performs any final actions before the device service is terminated.
 *
 * @param impl ...
 * @param force ...
 */
void myAdeptnessService_stop(void *impl, bool force)
{
    printf("-- Adeptness service stopped\n");
}

void create_error_message(char **values, char *message)
{
    JSON_Value *rval = json_value_init_object();
    JSON_Object *robj = json_value_get_object(rval);
    //json_object_set_string(robj, "message", "There has been an error parsing or updating the configuration");
    json_object_set_string(robj, "message", strdup(message));
    *values = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
    strcpy(*values, json_serialize_to_string(rval));
}

int update_microservice_configuration(char **values)
{
    printf("-- Update microservice configuration\n");
    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SETUP_ENDPOINT_TYPE) != NULL && json_object_get_value(jobj, JSON_KEY_SETUP_IP) != NULL &&
    json_object_get_value(jobj, JSON_KEY_SETUP_PORT) != NULL && json_object_get_value(jobj, JSON_KEY_SETUP_QOS) != NULL)
    {
        char *endpoint_type = strdup(json_object_get_string(jobj, JSON_KEY_SETUP_ENDPOINT_TYPE));

        if (strcmp(endpoint_type, "http") == 0)
        {
            extern int rest_server_port;
            rest_server_port = json_object_get_number(jobj, JSON_KEY_SETUP_PORT);

            extern uint8_t restart_http;
            restart_http = 1;
            strcpy(*values, "");
            return OK;
        }
        else if (strcmp(endpoint_type, "mqtt") == 0)
        {
            extern ms_status status;
            ms_status old_status = status;

            if(status == running)
            {
                status = configured;
            }

            extern char *mqtt_broker_host;
            extern int mqtt_broker_port;
            extern int mqtt_qos;
            mqtt_broker_host = strdup(json_object_get_string(jobj, JSON_KEY_SETUP_IP));
            mqtt_broker_port = json_object_get_number(jobj, JSON_KEY_SETUP_PORT);
            mqtt_qos = json_object_get_number(jobj, JSON_KEY_SETUP_QOS);

            extern uint8_t restart_mqtt;
            restart_mqtt = 1;

            if(old_status == running)
            {
                status = running;
            }
            strcpy(*values, "");
            return OK;
        }
        else
        {
            create_error_message(values, "No valid endpoint-type");
            return ERROR;
        }
    }
    else
    {
        create_error_message(values, "Configuration is not valid.");
        return ERROR;
    }
}

int read_connection_configuration(char **readings)
{
    extern char *can_conf_id;
    extern char *canport;
    extern int bitrate;

    if (can_conf_id != NULL)
    {
        JSON_Value *rval = json_value_init_object();
        JSON_Object *robj = json_value_get_object(rval);

        json_object_set_string(robj, JSON_KEY_CONNECTION_CONF_ID, strdup(can_conf_id));
        json_object_set_string(robj, JSON_KEY_CONNECTION_CONF_TYPE, "can");

        JSON_Value *branch = json_value_init_array();
        JSON_Array *leaves = json_value_get_array(branch);

        JSON_Value *leaf_value = json_value_init_object();
        JSON_Object *leaf_object = json_value_get_object(leaf_value);
        json_object_set_string(leaf_object, JSON_KEY_CONNECTION_CONF_KEY, "canport");
        json_object_set_string(leaf_object, JSON_KEY_CONNECTION_CONF_VALUE, strdup(canport));
        json_array_append_value(leaves, leaf_value);

        leaf_value = json_value_init_object();
        leaf_object = json_value_get_object(leaf_value);
        json_object_set_string(leaf_object, JSON_KEY_CONNECTION_CONF_KEY, "bitrate");
        json_object_set_number(leaf_object, JSON_KEY_CONNECTION_CONF_VALUE, bitrate);
        json_array_append_value(leaves, leaf_value);

        json_object_set_value(robj, JSON_KEY_CONNECTION_CONF_SETTINGS, branch);

        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return OK;
    }
    else
    {
        create_error_message(readings, "There is no valid CAN configuration");
        return ERROR;
    }
}

int update_connection_configuration(char **values)
{
    extern uint8_t can_up;
    extern char *canport;

    if (can_up == 1)
    {
        char command[80];
        sprintf(command, "ip link set %s down type can", canport);
        // TODO execute command to stop de CAN
        printf("%s\n", command);
    }

    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_CONNECTION_CONF_ID) != NULL && json_object_get_value(jobj, JSON_KEY_CONNECTION_CONF_TYPE) != NULL
    && json_object_get_value(jobj, JSON_KEY_CONNECTION_CONF_SETTINGS) != NULL)
    {
        extern char *can_conf_id;
        can_conf_id = strdup(json_object_get_string(jobj, JSON_KEY_CONNECTION_CONF_ID));

        char *monitorAgentType = strdup(json_object_get_string(jobj, JSON_KEY_CONNECTION_CONF_TYPE));
        if (strcmp(monitorAgentType, "can") == 0)
        {

            JSON_Array *connectionSettings = json_object_get_array(jobj, JSON_KEY_CONNECTION_CONF_SETTINGS);
            size_t settingsCount = json_array_get_count(connectionSettings);

            if (settingsCount == 2)
            {
                extern int bitrate;
                canport = NULL;
                bitrate = 0;

                for (size_t i = 0; i < settingsCount; i++)
                {
                    JSON_Value *connectionSetting = json_array_get_value(connectionSettings, i);
                    JSON_Object *settingObject = json_value_get_object(connectionSetting);

                    if (json_object_get_value(settingObject, JSON_KEY_CONNECTION_CONF_KEY) != NULL && 
                    json_object_get_value(settingObject, JSON_KEY_CONNECTION_CONF_VALUE) != NULL)
                    {
                        char *key = strdup(json_object_get_string(settingObject, JSON_KEY_CONNECTION_CONF_KEY));
                        
                        if (strcmp(key, "canport") == 0)
                        {
                            canport = strdup(json_object_get_string(settingObject, JSON_KEY_CONNECTION_CONF_VALUE));
                        }
                        else if (strcmp(key, "bitrate") == 0)
                        {
                            bitrate = json_object_get_number(settingObject, JSON_KEY_CONNECTION_CONF_VALUE);
                        }
                        else
                        {
                            create_error_message(values, "The CAN configuration is not valid");
                            return ERROR;
                        }
                    }
                    else
                    {
                        create_error_message(values, "The CAN configuration is not valid");
                        return ERROR;
                    }
                }
                if (canport == NULL || bitrate == 0)
                {
                    create_error_message(values, "The CAN configuration is not valid");
                    return ERROR;
                }
                
            }
            else
            {
                create_error_message(values, "The CAN configuration is not valid");
                return ERROR;
            }
        }
        else
        {
            create_error_message(values, "The configuration is not a CAN configuration");
            return ERROR;
        }

        char command[80];
        sprintf(command, "sudo ip link set %s up type can bitrate %d loopback off", canport, bitrate);
        // TODO execute command to start the CAN
        printf("%s\n", command);
        can_up = 1;

        return OK;
    }
    else
    {
        create_error_message(values, "The configuration is not valid");
        return ERROR;
    }
}

int read_monitoring_agent_status(char **readings)
{
    printf("-- Read monitoring agent status\n");
    JSON_Value *rval = json_value_init_object();
    JSON_Object *robj = json_value_get_object(rval);

    extern ms_status status;
    switch (status)
    {
    case running:
        json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "executing");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        printf("\t- Executing\n");
        return OK;
    case configured:
        json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "configured");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        printf("\t- Configured\n");
        return OK;
    case unconfigured:
        json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "unconfigured");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        printf("\t- Unconfigured\n");
        return OK;
    case error:
        json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "error");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        printf("\t- Error\n");
        return OK;
    case exit_ms:
        json_object_set_string(robj, JSON_KEY_ERROR_MESSAGE, "Exiting microservice");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        printf("\t- Exiting\n");
        return ERROR;
    default:
        json_object_set_string(robj, JSON_KEY_ERROR_MESSAGE, "Unknown error");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        printf("\t- Unkown\n");
        return ERROR;
    }
}

int cmd_execute_configuration(char **values)
{
    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);
    extern ms_status status;

    if (json_object_get_value(jobj, JSON_KEY_CMD_EXECUTE_ORDER) != NULL)
    {
        char *cmd = strdup(json_object_get_string(jobj, JSON_KEY_CMD_EXECUTE_ORDER));

        if (strcmp(cmd, "start") == 0)
        {
            if(status == configured)
            {
                printf("-- Changing status to executing...\n");
                status = running;
                strcpy(*values, "");
                return OK;
            }
            else 
            {
                printf("\t- Status was not configured...\n");
                create_error_message(values, "Status was not configured.");
                return ERROR;
            }
        }
        else if (strcmp(cmd, "stop") == 0)
        {
            if(status == running)
            {
                printf("-- Changing status to configured...\n");
                status = configured;
                strcpy(*values, "");
                return OK;
            }
            else
            {
                printf("\t- Status was not running...\n");
                create_error_message(values, "Status was not running.");
                return ERROR;
            }
        }
        else
        {
            printf("Error... Unknown order.\n");
            create_error_message(values, "Unknown order.");
            return ERROR;
        }
    }
    else
    {
        create_error_message(values, "There is no order key in the payload json.");
        return ERROR;
    }
}
