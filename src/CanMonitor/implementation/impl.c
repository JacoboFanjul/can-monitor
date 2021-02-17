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

        int32_t can_id = -1;
        int32_t init_bit = -1;
        int32_t end_bit = -1;

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
                        // TODO find more robust method
                        can_id = atoi(can_id_str);
                    }
                    else if (strcmp(key, "init-bit") == 0)
                    {
                        const char *init_bit_str = json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_VALUE);
                        // TODO find more robust method
                        init_bit = atoi(init_bit_str);
                    }
                    else if (strcmp(key, "end-bit") == 0)
                    {
                        const char *end_bit_str = json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_VALUE);
                        // TODO find more robust method
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
                sprintf(message, "There is no sensor with id %s", id);
                printf("%s\n", message);
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

    // TODO borrar de las listas de sensorgroups
    ListSensorgroups *sg_listptr;
    for (unsigned int i = 0; i < sensorgroup_table->size; ++i) 
    {
        sg_listptr = sensorgroup_table->array[i];

        while (sg_listptr != NULL) 
        {
            sensorgroup *sg = sg_listptr->sensorgroup;

            int j;
            for (j = 0; j < sg->sensorcount; j++)
            {
                if(strcmp(sg->sensor_list[j], id) == 0)
                {
                    break;
                }
            }

            if(j != sg->sensorcount)
            {
                for (int k = j; k < (sg->sensorcount - 1); k++)
                {
                    sg->sensor_list[k] = strdup(sg->sensor_list[k+1]);
                }
                sg->sensorcount--;
            }

            sg_listptr = sg_listptr->next;
        }
    }
    
    return OK;
}

int read_sensors_configuration(char **readings)
{
    printf("Read Sensors Configuration\n");
    extern HashTableSensors *sensors_table;
    
    JSON_Value *general_branch = json_value_init_array();
    JSON_Array *general_leaves = json_value_get_array(general_branch);

    ListSensors *listptr;
    for (unsigned int i = 0; i < sensors_table->size; ++i) 
    {
        listptr = sensors_table->array[i];

        while (listptr != NULL) 
        {
            sensor *sensor = malloc(sizeof(sensor));
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

int create_sensorgroups_configuration(char **values)
{
    printf("Create sensorgroups subscription\n");
    
    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_ID) != NULL && json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE) != NULL &&
    json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST) != NULL)
    {
        char *id = strdup(json_object_get_string(jobj, JSON_KEY_SENSORGROUPS_CONF_ID));
        sensorgroup *sg = htsg_get(sensorgroup_table, id);
        if (sg != NULL)
        {
            create_error_message(values, "A sensorgroup with that id already exists");
            return ERROR;
        }
        free(sg);

        sg = malloc(sizeof(sensorgroup));
        int publishRate = json_object_get_number(jobj, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE);

        JSON_Array *sgSensorlist = json_object_get_array(jobj, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST);
        size_t sensorsCount = json_array_get_count(sgSensorlist);
        char **sensor_list = malloc(sensorsCount * sizeof(char*));
        for (size_t i = 0; i < sensorsCount; i++)
        {
            JSON_Value *sensors_value = json_array_get_value(sgSensorlist, i);
            JSON_Object *sensors_object = json_value_get_object(sensors_value);

            // TODO complete sensor config
            if (json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID) != NULL && json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME) != NULL &&
            json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE) != NULL && json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS) != NULL &&
            json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SAMPLING_RATE) != NULL)
            {
                char *sensor_id = strdup(json_object_get_string(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID));
                sensor *sens = hts_get(sensors_table, sensor_id);

                if (sens != NULL)
                {
                    hts_delete(sensors_table, sensor_id);
                }
                
                sens = malloc(sizeof(sensor));
                sens->id = sensor_id;
                sens->name = strdup(json_object_get_string(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME));
                sens->type = strdup(json_object_get_string(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE));

                int32_t can_id = -1;
                int32_t init_bit = -1;
                int32_t end_bit = -1;
                JSON_Array *sensorSettingsList = json_object_get_array(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS);
                size_t sensorSettingsCount = json_array_get_count(sensorSettingsList);
                if (sensorSettingsCount == 3)
                {
                    for (size_t k = 0; k < sensorSettingsCount; k++)
                    {
                        JSON_Value *sensorSetting = json_array_get_value(sensorSettingsList, k);
                        JSON_Object *sensorSettingObject = json_value_get_object(sensorSetting);
                        if (json_object_get_value(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_KEY) != NULL && 
                        json_object_get_value(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE) != NULL)
                        {
                            char *key = strdup(json_object_get_string(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_KEY));
                            if (strcmp(key, "can-id") == 0)
                            {
                                const char *can_id_str = json_object_get_string(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE);
                                // TODO find more robust method
                                can_id = atoi(can_id_str);
                            }
                            else if (strcmp(key, "init-bit") == 0)
                            {
                                const char *init_bit_str = json_object_get_string(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE);
                                // TODO find more robust method
                                init_bit = atoi(init_bit_str);
                            }
                            else if (strcmp(key, "end-bit") == 0)
                            {
                                const char *end_bit_str = json_object_get_string(sensorSettingObject, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE);
                                // TODO find more robust method
                                end_bit = atoi(end_bit_str);
                            }
                            else
                            {
                                // for (size_t j = 0; j < i; j++)
                                // {
                                //     hts_delete(sensors_table, sensor_list[j]);
                                // }
                                create_error_message(values, "One of the sensors of the sensorgroup is not correct");
                                return ERROR;
                            }
                        }
                        else
                        {
                            // for (size_t j = 0; j < i; j++)
                            // {
                            //     hts_delete(sensors_table, sensor_list[j]);
                            // }
                            create_error_message(values, "One of the sensors of the sensorgroup is not correct");
                            return ERROR;
                        }
                    }
                }
                else
                {
                    // for (size_t j = 0; j < i; j++)
                    // {
                    //     hts_delete(sensors_table, sensor_list[j]);
                    // }
                    create_error_message(values, "One of the sensors of the sensorgroup is not correct");
                    return ERROR;
                }

                if (can_id == -1 || init_bit == -1 || end_bit == -1)
                {
                    for (size_t j = 0; j < i; j++)
                    {
                        hts_delete(sensors_table, sensor_list[j]);
                    }
                    create_error_message(values, "One of the sensors of the sensorgroup is not correct");
                    return ERROR;
                }

                sens->can_id = can_id;
                sens->init_bit = init_bit;
                sens->end_bit = end_bit;
                sens->sampling_rate = json_object_get_number(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SAMPLING_RATE);
                sens->value = "";
                sens->timestamp = 0;
                hts_put(sensors_table, sens->id, sens);
                
                
                sensor_list[i] = malloc(sizeof(char*));
                strcpy(sensor_list[i], sensor_id);
            }

            // Only id
            else if (json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID) != NULL && json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME) == NULL &&
            json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE) == NULL && json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS) == NULL &&
            json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SAMPLING_RATE) == NULL)
            {
                char *sensor_id = strdup(json_object_get_string(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID));
                sensor *sens = hts_get(sensors_table, sensor_id);

                if (sens != NULL)
                {
                    sensor_list[i] = malloc(sizeof(char*));
                    strcpy(sensor_list[i], sensor_id);
                }
                else
                {
                    create_error_message(values, "One of the sensors of the sensorgroup is not correct");
                    return ERROR;
                }
            }
            else
            {
                create_error_message(values, "The sensorgroup information is not valid");
                return ERROR;
            }
        }

        sg->id = id;
        sg->publish_rate = publishRate;
        sg->sensorcount = sensorsCount;
        sg->sensor_list = sensor_list;
        htsg_put(sensorgroup_table, sg->id, sg);

        strcpy(*values, "");
        return OK;
    }
    else
    {
        create_error_message(values, "The sensorgroup information is not valid");
        return ERROR;
    }
}

// TODO find a way for rollback if it is not succesful
int update_sensorgroups_configuration(char **values, query_pairs *queries)
{
    printf("Update sensorgroups subscriptions\n");
    int response = delete_sensorgroups_configuration(values, queries);
    if (response != OK)
    {
        return response;
    }
    response = create_sensorgroups_configuration(values);
    if (response != OK)
    {
        return response;
    }

    strcpy(*values, "");
    return OK;
}

int read_sensorgroups_configuration(char **readings)
{
    printf("Read Sensorgroups Configuration\n");
    extern HashTableSensorgroups *sensorgroup_table;
    extern HashTableSensors *sensors_table;

    JSON_Value *general_branch = json_value_init_array();
    JSON_Array *general_leaves = json_value_get_array(general_branch);

    ListSensorgroups *listptr;
    for (unsigned int i = 0; i < sensorgroup_table->size; i++)
    {
        listptr = sensorgroup_table->array[i];

        while (listptr != NULL) 
        {
            sensorgroup *sg = malloc(sizeof(sensorgroup));
            sg = listptr->sensorgroup;
            JSON_Value *leaf_value = json_value_init_object();
            JSON_Object *leaf_object = json_value_get_object(leaf_value);

            json_object_set_string(leaf_object, JSON_KEY_SENSORGROUPS_CONF_ID, sg->id);
            json_object_set_number(leaf_object, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE, sg->publish_rate);

            JSON_Value *sensors_array_value = json_value_init_array();
            JSON_Array *sensors_array_object = json_value_get_array(sensors_array_value);
            for (int i = 0; i < sg->sensorcount; i++)
            {
                sensor *sens = hts_get(sensors_table, sg->sensor_list[i]);
                JSON_Value *sensor_value = json_value_init_object();
                JSON_Object *sensor_object = json_value_get_object(sensor_value);
                json_object_set_string(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID, sens->id);
                json_object_set_string(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME, sens->name);
                json_object_set_string(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE, sens->type);

                JSON_Value *settings_array_value = json_value_init_array();
                JSON_Array *settings_array_object = json_value_get_array(settings_array_value);

                JSON_Value *settings_value = json_value_init_object();
                JSON_Object *settings_object = json_value_get_object(settings_value);
                json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_KEY, "can-id");
                char value[20];
                sprintf(value, "%d", sens->can_id);
                json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE, value);
                json_array_append_value(settings_array_object, settings_value);

                settings_value = json_value_init_object();
                settings_object = json_value_get_object(settings_value);
                json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_KEY, "init-bit");
                sprintf(value, "%d", sens->init_bit);
                json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE, value);
                json_array_append_value(settings_array_object, settings_value);

                settings_value = json_value_init_object();
                settings_object = json_value_get_object(settings_value);
                json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_KEY, "end-bit");
                sprintf(value, "%d", sens->end_bit);
                json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE, value);
                json_array_append_value(settings_array_object, settings_value);

                json_object_set_value(sensor_object, JSON_KEY_SENSOR_CONF_SETTINGS, settings_array_value);

                json_object_set_number(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SAMPLING_RATE, sens->sampling_rate);
                json_array_append_value(sensors_array_object, sensor_value);
            }

            json_object_set_value(leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST, sensors_array_value);
            json_array_append_value(general_leaves, leaf_value);
            listptr = listptr->next;
        }
    }

    *readings = malloc(strlen(json_serialize_to_string(general_branch)) * sizeof(char));
    strcpy(*readings, json_serialize_to_string(general_branch));
    return OK;

}

// TODO find a way for rollback if it is not succesful
int delete_sensorgroups_configuration(char **values, query_pairs * queries)
{
    printf("Delete subscriptions\n");

    query_pairs *tmp;
    tmp = queries;
    char* id = NULL;
    while(tmp != NULL)
    {
        if(strcmp(tmp->name, QUERY_KEY_ID) == 0)
        {
            id = strdup(tmp->value);
            printf("Delete sensorgroup with id %s\n", id);
            extern HashTableSensorgroups *sensorgroup_table;
            sensorgroup *sg = htsg_get(sensorgroup_table, id);
            if (sg != NULL)
            {
                // Delete individual sensors of the group
                /*for (size_t i = 0; i < sg->sensorcount; i++)
                {
                    sensor *sens = hts_delete(sensors_table, sg->sensor_list[i]);
                    if (sens == NULL)
                    {
                        create_error_message(values, "Error deleting a sensor of the sensorgroup");
                        return ERROR;
                    }
                }*/
                sg = htsg_delete(sensorgroup_table, id);
                if (sg == NULL)
                {
                    create_error_message(values, "Error deleting the sensorgroup");
                    return ERROR;
                }
            }
            else
            {
                char message[50];
                sprintf(message, "There is no sensorgroup with id %s", id);
                printf("%s\n", message);
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

int read_sensor_measurements(char **readings, query_pairs *queries)
{
    printf("Read Sensor Values\n");
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
        for (i = 0; i < sensors_table->size; ++i) 
        {

            listptr = sensors_table->array[i];

            while (listptr != NULL) 
            {
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
