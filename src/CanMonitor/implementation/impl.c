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

/**
 * @brief  Adeptness  template get random
 *
 * Random measures 
 *
 * @param impl ...
 *
 * @return Result of parsing
 */
bool adeptness_get_random(void *impl)
{
    myAdeptnessService_state *st = (myAdeptnessService_state *)impl;
    //unsigned char buffer[DEF_BUFF_SIZE];

    st->logical_data = rand() % 100;

    return true;
}

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
    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SETUP_ENDPOINT_TYPE) != NULL && json_object_get_value(jobj, JSON_KEY_SETUP_IP) != NULL &&
    json_object_get_value(jobj, JSON_KEY_SETUP_PORT) != NULL && json_object_get_value(jobj, JSON_KEY_SETUP_QOS) != NULL)
    {

        char *endpoint_type = strdup(json_object_get_string(jobj, JSON_KEY_SETUP_ENDPOINT_TYPE));

        if (strcpy(endpoint_type, "http") == 0)
        {
            extern int rest_server_port;
            rest_server_port = json_object_get_number(jobj, JSON_KEY_SETUP_PORT);

            extern uint8_t restart_http;
            restart_http = 1;
            strcpy(*values, "");
            return OK;
        }
        else if (strcpy(endpoint_type, "mqtt") == 0)
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

int create_sensors_configuration(char **values)
{
    printf("Create sensor configuration\n");

    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_ID) != NULL && json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_NAME) != NULL &&
        json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_TYPE) != NULL &&  json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_RATE) != NULL &&
        json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_SETTINGS) != NULL)
    {
        char *id = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_ID));

        sensor *sens = hts_get(sensors_table, id);
        if (sens != NULL)
        {
            create_error_message(values, "A sensor with that id already exists");
            return ERROR;
        }
        free(sens);

        char *variableName = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_NAME));

        char *variableType = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_TYPE));

        JSON_Array *connectionSettings = json_object_get_array(jobj, JSON_KEY_SENSOR_CONF_SETTINGS);
        size_t settingsCount = json_array_get_count(connectionSettings);

        int8_t can_id = -1;
        int8_t init_bit = -1;
        int8_t end_bit = -1;

        if (settingsCount == 3)
        {
            for (size_t i = 0; i < settingsCount; i++)
            {
                JSON_Value *connectionSetting = json_array_get_value(connectionSettings, i);
                JSON_Object *settingObject = json_value_get_object(connectionSetting);
                if (json_object_get_value(settingObject, JSON_KEY_SENSOR_CONF_KEY) != NULL && 
                json_object_get_value(settingObject, JSON_KEY_SENSOR_CONF_VALUE) != NULL)
                {
                    char *key = strdup(json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_KEY));
                    if (strcmp(key, "can-id") == 0)
                    {
                        const char *can_id_str = json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_VALUE);
                        can_id = atoi(can_id_str);
                    }
                    else if (strcmp(key, "init-bit") == 0)
                    {
                        const char *init_bit_str = json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_VALUE);
                        init_bit = atoi(init_bit_str);
                    }
                    else if (strcmp(key, "end-bit") == 0)
                    {
                        const char *end_bit_str = json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_VALUE);
                        end_bit = atoi(end_bit_str);
                    }
                    else
                    {
                        create_error_message(values, "The sensor information is not valid");
                        return ERROR;
                    }
                }
                else
                {
                    create_error_message(values, "The sensor information is not valid");
                    return ERROR;
                }
            }
            if (can_id == -1 || init_bit == -1 || end_bit == -1)
            {
                create_error_message(values, "The sensor information is not valid");
                return ERROR;
            }
            
        }
        else
        {
            create_error_message(values, "The sensor information is not valid");
            return ERROR;
        }

        int samplingRate = json_object_get_number(jobj, JSON_KEY_SENSOR_CONF_RATE);

        sens = malloc(sizeof (sensor));
        sens->id = strdup(id);
        sens->name = strdup(variableName);
        sens->type = strdup(variableType);
        sens->can_id = can_id;
        sens->init_bit = init_bit;
        sens->end_bit = end_bit;
        sens->sampling_rate = samplingRate;
        sens->value = "";
        sens->timestamp = 0;
        hts_put(sensors_table, sens->id, sens);
    }
    else
    {
        create_error_message(values, "The sensor information is not valid");
        return ERROR;
    }
    
    strcpy(*values, "");
    return OK;
}

int update_sensors_configuration(char **values, query_pairs *queries)
{
    printf("Update variables configuration\n");
    int response = delete_sensors_configuration(values, queries);
    if (response != OK)
    {
        return response;
    }
    response = create_sensors_configuration(values);
    if (response != OK)
    {
        return response;
    }
    strcpy(*values, "");
    return OK;
}

int read_sensors_configuration(char **readings)
{
    printf("Read Sensors Configuration\n");
    extern HashTableSensors *sensors_table;
    
    JSON_Value *general_branch = json_value_init_array();
    JSON_Array *general_leaves = json_value_get_array(general_branch);

    unsigned int i;
    ListSensors *listptr;
    for (i = 0; i < sensors_table->size; ++i) {

        listptr = sensors_table->array[i];

        while (listptr != NULL) {
            sensor *sensor = malloc(sizeof *sensor);
            sensor = listptr->sensor;
                
            JSON_Value *leaf_value = json_value_init_object();
            JSON_Object *leaf_object = json_value_get_object(leaf_value);

            json_object_set_string(leaf_object, JSON_KEY_SENSOR_CONF_ID, sensor->id);
            json_object_set_string(leaf_object, JSON_KEY_SENSOR_CONF_NAME, sensor->name);
            json_object_set_string(leaf_object, JSON_KEY_SENSOR_CONF_TYPE, sensor->type);

            JSON_Value *settings_array_value = json_value_init_array();
            JSON_Array *settings_array_object = json_value_get_array(settings_array_value);

            JSON_Value *settings_value = json_value_init_object();
            JSON_Object *settings_object = json_value_get_object(settings_value);
            json_object_set_string(settings_object, JSON_KEY_SENSOR_CONF_KEY, "can-id");
            char value[20];
            sprintf(value, "%d", sensor->can_id);
            json_object_set_string(settings_object, JSON_KEY_SENSOR_CONF_VALUE, value);
            json_array_append_value(settings_array_object, settings_value);

            settings_value = json_value_init_object();
            settings_object = json_value_get_object(settings_value);
            json_object_set_string(settings_object, JSON_KEY_SENSOR_CONF_KEY, "init-bit");
            sprintf(value, "%d", sensor->init_bit);
            json_object_set_string(settings_object, JSON_KEY_SENSOR_CONF_VALUE, value);
            json_array_append_value(settings_array_object, settings_value);

            settings_value = json_value_init_object();
            settings_object = json_value_get_object(settings_value);
            json_object_set_string(settings_object, JSON_KEY_SENSOR_CONF_KEY, "end-bit");
            sprintf(value, "%d", sensor->end_bit);
            json_object_set_string(settings_object, JSON_KEY_SENSOR_CONF_VALUE, value);
            json_array_append_value(settings_array_object, settings_value);
            
            json_object_set_value(leaf_object, JSON_KEY_SENSOR_CONF_SETTINGS, settings_array_value);
            json_object_set_number(leaf_object, JSON_KEY_SENSOR_CONF_RATE, sensor->sampling_rate);
            json_array_append_value(general_leaves, leaf_value);

            listptr = listptr->next;
        }
    }

    *readings = malloc(strlen(json_serialize_to_string(general_branch)) * sizeof(char));
    strcpy(*readings, json_serialize_to_string(general_branch));
    return OK;
}

int delete_sensors_configuration(char **values, query_pairs *queries)
{
    query_pairs *tmp;
    tmp = queries;
    char* id = NULL;
    while(tmp != NULL)
    {
        if(strcmp(tmp->name, QUERY_KEY_ID) == 0)
        {
            id = strdup(tmp->value);
            printf("Delete sensor with id %s\n", id);
            extern HashTableSensors *sensors_table;
            sensor *sens = hts_delete(sensors_table, id);
            if (sens != NULL)
            {
                printf("Sensor with id %s deleted\n", sens->id);
            }
            else
            {
                char message[50];
                printf("There is no sensor with id %s\n", id);
                sprintf(message, "There is no sensor with id %s", id);
                create_error_message(values, message);
                return ERROR;
            }
        }
        tmp = tmp->next;
    }
    if (id == NULL)
    {
        printf("Delete sensor request without id\n");
        create_error_message(values, "There is no id query.");
        return ERROR;
    }

    return OK;
}

// TODO POST SENSORGROUP SUBSCRIPTION
// TODO add functionality
int create_sensorgroups_configuration(char **values)
{
    printf("Create subscriptions\n");
    // TODO Create setup data & error control

    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_ID) != NULL)
    {
        char *id = strdup(json_object_get_string(jobj, JSON_KEY_SENSORGROUPS_CONF_ID));
        printf("{\n\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_ID, id);
    }
    else
    {
        create_error_message(values, "There is no sensorgroup-id key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE) != NULL)
    {
        int publishRate = json_object_get_number(jobj, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE);
        printf("\t\"%s\": \"%d\",\n", JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE, publishRate);
    }
    else
    {
        create_error_message(values, "There is no publish-rate key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST) != NULL)
    {
        printf("\t\"%s\": [\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST);

        JSON_Array *sensorList = json_object_get_array(jobj, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST);
        size_t sensorCount = json_array_get_count(sensorList);
        for (size_t i = 0; i < sensorCount; i++)
        {
            printf("\t\t{\n");

            JSON_Value *sensor = json_array_get_value(sensorList, i);
            JSON_Object *sensorObject = json_value_get_object(sensor);

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID) != NULL)
            {
                char *sensorId = strdup(json_object_get_string(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID));
                printf("\t\t\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID, sensorId);
            }
            else
            {
                create_error_message(values, "There is no sensor-id key in the connectionSettings array.");
                return ERROR;
            }

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME) != NULL)
            {
                char *sensorName = strdup(json_object_get_string(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME));
                printf("\t\t\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME, sensorName);
            }
            else
            {
                create_error_message(values, "There is no sensor-name key in the connectionSettings array.");
                return ERROR;
            }

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE) != NULL)
            {
                char *sensorType = strdup(json_object_get_string(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE));
                printf("\t\t\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE, sensorType);
            }
            else
            {
                create_error_message(values, "There is no sensor-type key in the connectionSettings array.");
                return ERROR;
            }

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS) != NULL)
            {
                printf("\t\t\t\"%s\": [\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS);
                
                JSON_Array *sensorSettingsList = json_object_get_array(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS);
                size_t sensorSettingsCount = json_array_get_count(sensorSettingsList);

                for (size_t i = 0; i < sensorSettingsCount; i++)
                {
                    printf("\t\t\t\t{\n");

                    JSON_Value *sensorSetting = json_array_get_value(sensorSettingsList, i);
                    JSON_Object *sensorSettingObject = json_value_get_object(sensorSetting);

                    if (json_object_get_value(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_KEY) != NULL)
                    {
                        char *key = strdup(json_object_get_string(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_KEY));
                        printf("\t\t\t\t\t\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_KEY, key);
                    }
                    else
                    {
                        create_error_message(values, "There is no key key in the connectionSettings array.");
                        return ERROR;
                    }
                    if (json_object_get_value(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_VALUE) != NULL)
                    {
                        char *value = strdup(json_object_get_string(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_VALUE));
                        printf("\t\t\t\t\t\t\"%s\": %s\n", JSON_KEY_SENSORGROUPS_CONF_VALUE, value);
                    }
                    else
                    {
                        create_error_message(values, "There is no value key in the connectionSettings array.");
                        return ERROR;
                    }

                    if (i < sensorSettingsCount - 1)
                    {
                        printf("\t\t\t\t},\n");
                    }
                    else
                    {
                        printf("\t\t\t\t}\n");
                    }

                }

                printf("\t\t\t],\n");
            }
            else
            {
                create_error_message(values, "There is no sensor-settings key in the connection-settings array.");
                return ERROR;
            }

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SAMPLING_RATE) != NULL)
            {
                int samplingRate = json_object_get_number(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SAMPLING_RATE);
                printf("\t\t\t\"%s\": %d\n", JSON_KEY_SENSORGROUPS_CONF_SAMPLING_RATE, samplingRate);
            }
            else
            {
                create_error_message(values, "There is no sampling-rate key in the connection-settings array.");
                return ERROR;
            }

            if (i < sensorCount - 1)
            {
                printf("\t\t},\n");
            }
            else
            {
                printf("\t\t}\n");
            }
        }
    }
    else
    {
        create_error_message(values, "There is no connection-settings key in the payload json.");
        return ERROR;
    }


    printf("\t]\n");
    printf("}\n");
    strcpy(*values, "");
    return OK;
}

// TODO PUT SENSORGROUP SUBSCRIPTION
// TODO add functionality
int update_sensorgroups_configuration(char **values, query_pairs *queries)
{
    // TODO Update setup data & error control
    // TODO add query support
    printf("Update subscriptions\n");
    // TODO Create setup data & error control

    // TODO parse query, for now, just print
    query_pairs *tmp;
    tmp = queries;
    char* id = NULL;
    while(tmp != NULL)
    {
        if(strcmp(tmp->name, QUERY_KEY_ID) == 0)
        {
            id = strdup(tmp->value);
            printf("\tName: %s\n", tmp->name);
            printf("\tValue: %s\n", id);
            printf("ID: %s\n", id);
        }
        else
        {
            printf("\tName: %s\n", tmp->name);
            printf("\tValue: %s\n", tmp->value);
        }
        
        tmp = tmp->next;
    }
    if (id == NULL)
    {
        create_error_message(values, "There is no id query.");
        return ERROR;
    }

    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_ID) != NULL)
    {
        char *id = strdup(json_object_get_string(jobj, JSON_KEY_SENSORGROUPS_CONF_ID));
        printf("{\n\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_ID, id);
    }
    else
    {
        create_error_message(values, "There is no sensorgroup-id key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE) != NULL)
    {
        int publishRate = json_object_get_number(jobj, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE);
        printf("\t\"%s\": \"%d\",\n", JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE, publishRate);
    }
    else
    {
        create_error_message(values, "There is no publish-rate key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST) != NULL)
    {
        printf("\t\"%s\": [\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST);

        JSON_Array *sensorList = json_object_get_array(jobj, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST);
        size_t sensorCount = json_array_get_count(sensorList);
        for (size_t i = 0; i < sensorCount; i++)
        {
            printf("\t\t{\n");

            JSON_Value *sensor = json_array_get_value(sensorList, i);
            JSON_Object *sensorObject = json_value_get_object(sensor);

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID) != NULL)
            {
                char *sensorId = strdup(json_object_get_string(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID));
                printf("\t\t\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID, sensorId);
            }
            else
            {
                create_error_message(values, "There is no sensor-id key in the connectionSettings array.");
                return ERROR;
            }

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME) != NULL)
            {
                char *sensorName = strdup(json_object_get_string(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME));
                printf("\t\t\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME, sensorName);
            }
            else
            {
                create_error_message(values, "There is no sensor-name key in the connectionSettings array.");
                return ERROR;
            }

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE) != NULL)
            {
                char *sensorType = strdup(json_object_get_string(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE));
                printf("\t\t\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE, sensorType);
            }
            else
            {
                create_error_message(values, "There is no sensor-type key in the connectionSettings array.");
                return ERROR;
            }

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS) != NULL)
            {
                printf("\t\t\t\"%s\": [\n", JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS);
                
                JSON_Array *sensorSettingsList = json_object_get_array(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS);
                size_t sensorSettingsCount = json_array_get_count(sensorSettingsList);

                for (size_t i = 0; i < sensorSettingsCount; i++)
                {
                    printf("\t\t\t\t{\n");

                    JSON_Value *sensorSetting = json_array_get_value(sensorSettingsList, i);
                    JSON_Object *sensorSettingObject = json_value_get_object(sensorSetting);

                    if (json_object_get_value(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_KEY) != NULL)
                    {
                        char *key = strdup(json_object_get_string(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_KEY));
                        printf("\t\t\t\t\t\t\"%s\": %s,\n", JSON_KEY_SENSORGROUPS_CONF_KEY, key);
                    }
                    else
                    {
                        create_error_message(values, "There is no key key in the connectionSettings array.");
                        return ERROR;
                    }
                    if (json_object_get_value(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_VALUE) != NULL)
                    {
                        char *value = strdup(json_object_get_string(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_VALUE));
                        printf("\t\t\t\t\t\t\"%s\": %s\n", JSON_KEY_SENSORGROUPS_CONF_VALUE, value);
                    }
                    else
                    {
                        create_error_message(values, "There is no value key in the connectionSettings array.");
                        return ERROR;
                    }

                    if (i < sensorSettingsCount - 1)
                    {
                        printf("\t\t\t\t},\n");
                    }
                    else
                    {
                        printf("\t\t\t\t}\n");
                    }

                }

                printf("\t\t\t],\n");
            }
            else
            {
                create_error_message(values, "There is no sensor-settings key in the connectionSettings array.");
                return ERROR;
            }

            if (json_object_get_value(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SAMPLING_RATE) != NULL)
            {
                int samplingRate = json_object_get_number(sensorObject, JSON_KEY_SENSORGROUPS_CONF_SAMPLING_RATE);
                printf("\t\t\t\"%s\": %d\n", JSON_KEY_SENSORGROUPS_CONF_SAMPLING_RATE, samplingRate);
            }
            else
            {
                create_error_message(values, "There is no sampling-rate key in the connectionSettings array.");
                return ERROR;
            }

            if (i < sensorCount - 1)
            {
                printf("\t\t},\n");
            }
            else
            {
                printf("\t\t}\n");
            }
        }
    }
    else
    {
        create_error_message(values, "There is no connection-settings key in the payload json.");
        return ERROR;
    }

    printf("\t]\n");
    printf("}\n");
    strcpy(*values, "");
    return OK;
}

// TODO GET SENSORGROUP SUBSCRIPTION
int read_sensorgroups_configuration(char **readings)
{
    
    JSON_Value *general_branch = json_value_init_array();
    JSON_Array *general_leaves = json_value_get_array(general_branch);

    // TODO Loop for the size of the sensorgroup table
    for (size_t i = 0; i < 5; i++)
    {
        JSON_Value *leaf_value = json_value_init_object();
        JSON_Object *leaf_object = json_value_get_object(leaf_value);

        // TODO Add data from the tables
        json_object_set_string(leaf_object, JSON_KEY_SENSORGROUPS_CONF_ID, "string");
        json_object_set_number(leaf_object, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE, 0);

        JSON_Value *variable_list_branch = json_value_init_array();
        JSON_Array *variable_list_leaves = json_value_get_array(variable_list_branch);
        JSON_Value *variable_leaf_value;
        JSON_Object *variable_leaf_object;

        // TODO Loop for sensor-list size
        for (size_t j = 0; j < 2; j++)
        {
            variable_leaf_value = json_value_init_object();
            variable_leaf_object = json_value_get_object(variable_leaf_value);

            json_object_set_string(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID, "string");
            json_object_set_string(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME, "string");
            json_object_set_string(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE, "Uint8");

            JSON_Value *variable_settings_branch = json_value_init_array();
            JSON_Array *variable_settings_leaves = json_value_get_array(variable_settings_branch);

            // TODO Loop for sensor-settings size
            for (size_t k = 0; k < 2; k++)
            {
                JSON_Value *variable_settings_leaf_value = json_value_init_object();
                JSON_Object *variable_settings_leaf_object = json_value_get_object(variable_settings_leaf_value);
                // TODO Get data from the table
                json_object_set_string(variable_settings_leaf_object, JSON_KEY_SENSORGROUPS_CONF_KEY, "string");
                json_object_set_string(variable_settings_leaf_object, JSON_KEY_SENSORGROUPS_CONF_VALUE, "string");
                json_array_append_value(variable_settings_leaves, variable_settings_leaf_value);
            }
            
            json_object_set_value(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS, variable_settings_branch);

            // TODO Get data from the table
            json_object_set_number(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SAMPLING_RATE, 0);
            json_array_append_value(variable_list_leaves, variable_leaf_value);
        }
        json_object_set_value(leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST, variable_list_branch);
        json_array_append_value(general_leaves, leaf_value);
    }

    *readings = malloc(strlen(json_serialize_to_string(general_branch)) * sizeof(char));
    strcpy(*readings, json_serialize_to_string(general_branch));
    return OK;

}

// TODO DELETE SENSORGROUP SUBSCRIPTION
// TODO add functionality
int delete_sensorgroups_configuration(char **values, query_pairs * queries)
{
    printf("Delete subscriptions\n");

    // TODO parse query, for now, just print
    query_pairs *tmp;
    tmp = queries;
    char* id = NULL;
    while(tmp != NULL)
    {
        if(strcmp(tmp->name, QUERY_KEY_ID) == 0)
        {
            id = strdup(tmp->value);
            printf("\tName: %s\n", tmp->name);
            printf("\tValue: %s\n", id);
            printf("ID: %s\n", id);
        }
        else
        {
            printf("\tName: %s\n", tmp->name);
            printf("\tValue: %s\n", tmp->value);
        }
        
        tmp = tmp->next;
    }
    if (id == NULL)
    {
        create_error_message(values, "There is no id query.");
        return ERROR;
    }
    
    return OK;
}

int read_monitoring_agent_status(char **readings)
{
    JSON_Value *rval = json_value_init_object();
    JSON_Object *robj = json_value_get_object(rval);

    extern ms_status status;
    switch (status)
    {
    case running:
        json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "executing");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return OK;
    case configured:
        json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "configured");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return OK;
    case unconfigured:
        json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "unconfigured");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return OK;
    case error:
        json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "error");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return OK;
    case exit_ms:
        json_object_set_string(robj, JSON_KEY_ERROR_MESSAGE, "Exiting microservice");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return ERROR;
    default:
        json_object_set_string(robj, JSON_KEY_ERROR_MESSAGE, "Unknown error");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
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
        printf("{\n\t\"%s\": %s\n}\n", JSON_KEY_CMD_EXECUTE_ORDER, cmd);

        if (strcmp(cmd, "start") == 0)
        {
            if(status == configured)
            {
                printf("Changing status to executing...\n");
                strcpy(*values, "");
                return OK;
            }
            else 
            {
                printf("Status was not configured...\n");
                create_error_message(values, "Status was not configured.");
                return ERROR;
            }
        }
        else if (strcmp(cmd, "stop") == 0)
        {
            if(status == running)
            {
                status = configured;
                printf("Changing status to configured...\n");
                strcpy(*values, "");
                return OK;
            }
            else
            {
                printf("Status was not running...\n");
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

// TODO GET SENSORS MEASUREMENTS
int read_sensor_measurements(char **readings, query_pairs *queries)
{
    printf("Read Sensor Values\n");
    // TODO for now, just print the query
    query_pairs *tmp;
    tmp = queries;
    char* sensor_id;

    if (tmp != NULL)
    {
        if (tmp->next == NULL)
        {
            if(strcmp(tmp->name, QUERY_KEY_ID) == 0)
            {
                sensor_id = strdup(tmp->value);
                // TODO Buscar el sensor en el hashtable

                extern HashTableSensors *sensors_table;
                sensor *sens = hts_get(sensors_table, sensor_id);
                if (sens != NULL)
                {
                    JSON_Value *general_branch = json_value_init_array();
                    JSON_Array *general_leaves = json_value_get_array(general_branch);

                    JSON_Value *leaf_value = json_value_init_object();
                    JSON_Object *leaf_object = json_value_get_object(leaf_value);

                    json_object_set_string(leaf_object, JSON_KEY_SENSOR_MEASUREMENTS_ID, sens->id);

                    JSON_Value *sensor_data = json_value_init_object();
                    JSON_Object *sensor_data_object = json_value_get_object(sensor_data);

                    json_object_set_string(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_NAME, sens->name);
                    json_object_set_string(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_TYPE, sens->type);
                    json_object_set_string(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_VALUE, sens->value);
                    json_object_set_number(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_TIMESTAMP, sens->timestamp);
                    json_object_set_value(leaf_object, JSON_KEY_SENSOR_MEASUREMENTS_DATA, sensor_data);

                    json_array_append_value(general_leaves, leaf_value);

                    *readings = malloc(strlen(json_serialize_to_string(general_branch)) * sizeof(char));
                    strcpy(*readings, json_serialize_to_string(general_branch));
                    return OK;
                }
                else
                {
                    create_error_message(readings, "There is no sensor with that id");
                    return ERROR;
                }
            }
            else
            {
                create_error_message(readings, "Unsupported query");
                return ERROR;
            }
        }
        else
        {
            create_error_message(readings, "Too many query variables in the request");
            return ERROR;
        }
    }
    else // Send the values of all sensors
    {
        extern HashTableSensors *sensors_table;
        
        JSON_Value *general_branch = json_value_init_array();
        JSON_Array *general_leaves = json_value_get_array(general_branch);

        unsigned int i;
        ListSensors *listptr;
        for (i = 0; i < sensors_table->size; ++i) {

            listptr = sensors_table->array[i];

            while (listptr != NULL) {
                sensor *sensor = malloc(sizeof *sensor);
                sensor = listptr->sensor;
                    
                JSON_Value *leaf_value = json_value_init_object();
                JSON_Object *leaf_object = json_value_get_object(leaf_value);

                json_object_set_string(leaf_object, JSON_KEY_SENSOR_MEASUREMENTS_ID, sensor->id);
                JSON_Value *sensor_data = json_value_init_object();
                JSON_Object *sensor_data_object = json_value_get_object(sensor_data);
                json_object_set_string(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_NAME, sensor->name);
                json_object_set_string(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_TYPE, sensor->type);
                json_object_set_string(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_VALUE, sensor->value);
                json_object_set_number(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_TIMESTAMP, sensor->timestamp);
                json_object_set_value(leaf_object, JSON_KEY_SENSOR_MEASUREMENTS_DATA, sensor_data);

                json_array_append_value(general_leaves, leaf_value);
                listptr = listptr->next;
            }
        }

        *readings = malloc(strlen(json_serialize_to_string(general_branch)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(general_branch));
        return OK;
    }  
}
