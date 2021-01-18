/** @file impl.c
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 2019
 * Ikerlan
 */

/* Include */
#include "impl.h"
#include "rest_server/rest_server_impl.h"

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

// TODO add functionality
int update_microservice_configuration(char **values)
{
    printf("Update configuration\n");
    // TODO parse & update setup & error control
    // Example:

    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SETUP_ENDPOINT_TYPE) != NULL)
    {
        char *endpoint_type = strdup(json_object_get_string(jobj, JSON_KEY_SETUP_ENDPOINT_TYPE));
        printf("{\n\t\"%s\": %s,\n", JSON_KEY_SETUP_ENDPOINT_TYPE, endpoint_type);
    }
    else
    {
        create_error_message(values, "There is no endpoint-type key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SETUP_IP) != NULL)
    {
        char *iP = strdup(json_object_get_string(jobj, JSON_KEY_SETUP_IP));
        printf("\t\"%s\": \"%s\",\n", JSON_KEY_SETUP_IP, iP);
    }
    else
    {
        create_error_message(values, "There is no ip key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SETUP_PORT) != NULL)
    {
        int port = json_object_get_number(jobj, JSON_KEY_SETUP_PORT);
        printf("\t\"%s\": %d,\n", JSON_KEY_SETUP_PORT, port);
    }
    else
    {
        create_error_message(values, "There is no port key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SETUP_QOS) != NULL)
    {
        int qoS = json_object_get_number(jobj, JSON_KEY_SETUP_QOS);
        printf("\t\"%s\": %d\n", JSON_KEY_SETUP_QOS, qoS);
    }
    else
    {
        create_error_message(values, "There is no qos key in the payload json.");
        return ERROR;
    }

    printf("}\n");
    strcpy(*values, "");
    return OK;
}

int read_connection_configuration(char **readings)
{
    JSON_Value *rval = json_value_init_object();
    JSON_Object *robj = json_value_get_object(rval);

    // TODO add real data, no hardcoded.
    json_object_set_string(robj, JSON_KEY_CONNECTION_CONF_ID, "0");
    json_object_set_string(robj, JSON_KEY_CONNECTION_CONF_TYPE, "can");

    JSON_Value *branch = json_value_init_array();
    JSON_Array *leaves = json_value_get_array(branch);

    // TODO add real data, no hardcoded
    for (size_t i = 0; i < 1; i++)
    {
        JSON_Value *leaf_value = json_value_init_object();
        JSON_Object *leaf_object = json_value_get_object(leaf_value);

        json_object_set_string(leaf_object, JSON_KEY_CONNECTION_CONF_KEY, "canport");
        json_object_set_string(leaf_object, JSON_KEY_CONNECTION_CONF_VALUE, "can0");
        json_array_append_value(leaves, leaf_value);
    }

    json_object_set_value(robj, JSON_KEY_CONNECTION_CONF_SETTINGS, branch);

    // TODO check if there has been an error
    // if not error
    if (true)
    {
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return OK;
    }
    else
    {
        rval = json_value_init_object();
        robj = json_value_get_object(rval);
        json_object_set_string(robj, "message", "There has been an error reading the configuration");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return ERROR;
    }
}

// TODO add functionality
int update_connection_configuration(char **values)
{
    printf("Update configuration\n");
    // TODO parse & update setup & error control
    // Example:

    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_CONNECTION_CONF_ID) != NULL)
    {
        char *id = strdup(json_object_get_string(jobj, JSON_KEY_CONNECTION_CONF_ID));
        printf("{\n\t\"%s\": %s,\n", JSON_KEY_CONNECTION_CONF_ID, id);
    }
    else
    {
        create_error_message(values, "There is no id key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_CONNECTION_CONF_TYPE) != NULL)
    {
        char *monitorAgentType = strdup(json_object_get_string(jobj, JSON_KEY_CONNECTION_CONF_TYPE));
        printf("\t\"%s\": \"%s\",\n", JSON_KEY_CONNECTION_CONF_TYPE, monitorAgentType);
    }
    else
    {
        create_error_message(values, "There is no monitorAgentType key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_CONNECTION_CONF_SETTINGS) != NULL)
    {
        printf("\t\"%s\": [\n", JSON_KEY_CONNECTION_CONF_SETTINGS);

        JSON_Array *connectionSettings = json_object_get_array(jobj, JSON_KEY_CONNECTION_CONF_SETTINGS);
        size_t settingsCount = json_array_get_count(connectionSettings);
        for (size_t i = 0; i < settingsCount; i++)
        {
            printf("\t\t{\n");

            JSON_Value *connectionSetting = json_array_get_value(connectionSettings, i);
            JSON_Object *settingObject = json_value_get_object(connectionSetting);

            if (json_object_get_value(settingObject, JSON_KEY_CONNECTION_CONF_KEY) != NULL)
            {
                char *key = strdup(json_object_get_string(settingObject, JSON_KEY_CONNECTION_CONF_KEY));
                printf("\t\t\t\"%s\": %s\n", JSON_KEY_CONNECTION_CONF_KEY, key);
            }
            else
            {
                create_error_message(values, "There is no key key in the connectionSettings array.");
                return ERROR;
            }

            if (json_object_get_value(settingObject, JSON_KEY_CONNECTION_CONF_VALUE) != NULL)
            {
                char *value = strdup(json_object_get_string(settingObject, JSON_KEY_CONNECTION_CONF_VALUE));
                printf("\t\t\t\"%s\": %s\n", JSON_KEY_CONNECTION_CONF_VALUE, value);
            }
            else
            {
                create_error_message(values, "There is no value key in the connectionSettings array.");
                return ERROR;
            }

            if (i < settingsCount - 1)
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
        create_error_message(values, "There is no connectionSettings key in the payload json.");
        return ERROR;
    }

    printf("\t]\n");
    printf("}\n");
    strcpy(*values, "");
    return OK;
}

// TODO add functionality
int create_sensors_configuration(char **values)
{
    printf("Create variables configuration\n");
    // TODO Update setup data & error control

    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_ID) != NULL)
    {
        char *id = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_ID));
        printf("{\n\t\"%s\": %s,\n", JSON_KEY_SENSOR_CONF_ID, id);
    }
    else
    {
        create_error_message(values, "There is no sensorId key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_NAME) != NULL)
    {
        char *variableName = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_NAME));
        printf("\t\"%s\": \"%s\",\n", JSON_KEY_SENSOR_CONF_NAME, variableName);
    }
    else
    {
        create_error_message(values, "There is no sensorName key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_TYPE) != NULL)
    {
        char *variableType = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_TYPE));
        printf("\t\"%s\": \"%s\",\n", JSON_KEY_SENSOR_CONF_TYPE, variableType);
    }
    else
    {
        create_error_message(values, "There is no sensorType key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_SETTINGS) != NULL)
    {
        printf("\t\"%s\": [\n", JSON_KEY_SENSOR_CONF_SETTINGS);

        JSON_Array *connectionSettings = json_object_get_array(jobj, JSON_KEY_SENSOR_CONF_SETTINGS);
        size_t settingsCount = json_array_get_count(connectionSettings);
        for (size_t i = 0; i < settingsCount; i++)
        {
            printf("\t\t{\n");

            JSON_Value *connectionSetting = json_array_get_value(connectionSettings, i);
            JSON_Object *settingObject = json_value_get_object(connectionSetting);

            if (json_object_get_value(settingObject, JSON_KEY_SENSOR_CONF_KEY) != NULL)
            {
                char *key = strdup(json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_KEY));
                printf("\t\t\t\"%s\": %s\n", JSON_KEY_SENSOR_CONF_KEY, key);
            }
            else
            {
                create_error_message(values, "There is no key key in the sensorSettings array.");
                return ERROR;
            }

            if (json_object_get_value(settingObject, JSON_KEY_SENSOR_CONF_VALUE) != NULL)
            {
                char *value = strdup(json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_VALUE));
                printf("\t\t\t\"%s\": %s\n", JSON_KEY_SENSOR_CONF_VALUE, value);
            }
            else
            {
                create_error_message(values, "There is no value key in the sensorSettings array.");
                return ERROR;
            }

            if (i < settingsCount - 1)
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
        create_error_message(values, "There is no sensorSettings key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_RATE) != NULL)
    {
        int samplingRate = json_object_get_number(jobj, JSON_KEY_SENSOR_CONF_RATE);
        printf("\t\"%s\": \"%d\",\n", JSON_KEY_SENSOR_CONF_RATE, samplingRate);
    }
    else
    {
        create_error_message(values, "There is no samplingRate key in the payload json.");
        return ERROR;
    }

    printf("\t]\n");
    printf("}\n");
    strcpy(*values, "");
    return OK;
}

// TODO add functionality
int update_sensors_configuration(char **values, query_pairs *queries)
{
    printf("Update variables configuration\n");
    // TODO Add query for the ID, for now, just print the query
    // TODO Update setup data & error control
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

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_ID) != NULL)
    {
        char *id = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_ID));
        printf("{\n\t\"%s\": %s,\n", JSON_KEY_SENSOR_CONF_ID, id);
    }
    else
    {
        create_error_message(values, "There is no sensorId key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_NAME) != NULL)
    {
        char *variableName = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_NAME));
        printf("\t\"%s\": \"%s\",\n", JSON_KEY_SENSOR_CONF_NAME, variableName);
    }
    else
    {
        create_error_message(values, "There is no variableName key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_TYPE) != NULL)
    {
        char *variableType = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_TYPE));
        printf("\t\"%s\": \"%s\",\n", JSON_KEY_SENSOR_CONF_TYPE, variableType);
    }
    else
    {
        create_error_message(values, "There is no variableType key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_SETTINGS) != NULL)
    {
        printf("\t\"%s\": [\n", JSON_KEY_SENSOR_CONF_SETTINGS);

        JSON_Array *connectionSettings = json_object_get_array(jobj, JSON_KEY_SENSOR_CONF_SETTINGS);
        size_t settingsCount = json_array_get_count(connectionSettings);
        for (size_t i = 0; i < settingsCount; i++)
        {
            printf("\t\t{\n");

            JSON_Value *connectionSetting = json_array_get_value(connectionSettings, i);
            JSON_Object *settingObject = json_value_get_object(connectionSetting);

            if (json_object_get_value(settingObject, JSON_KEY_SENSOR_CONF_KEY) != NULL)
            {
                char *key = strdup(json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_KEY));
                printf("\t\t\t\"%s\": %s\n", JSON_KEY_SENSOR_CONF_KEY, key);
            }
            else
            {
                create_error_message(values, "There is no key key in the connectionSettings array.");
                return ERROR;
            }

            if (json_object_get_value(settingObject, JSON_KEY_SENSOR_CONF_VALUE) != NULL)
            {
                char *value = strdup(json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_VALUE));
                printf("\t\t\t\"%s\": %s\n", JSON_KEY_SENSOR_CONF_VALUE, value);
            }
            else
            {
                create_error_message(values, "There is no value key in the variableSettings array.");
                return ERROR;
            }

            if (i < settingsCount - 1)
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
        create_error_message(values, "There is no connectionSettings key in the payload json.");
        return ERROR;
    }

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_RATE) != NULL)
    {
        int samplingRate = json_object_get_number(jobj, JSON_KEY_SENSOR_CONF_RATE);
        printf("\t\"%s\": \"%d\",\n", JSON_KEY_SENSOR_CONF_RATE, samplingRate);
    }
    else
    {
        create_error_message(values, "There is no samplingRate key in the payload json.");
        return ERROR;
    }

    printf("\t]\n");
    printf("}\n");
    strcpy(*values, "");
    return OK;
}

int read_sensors_configuration(char **readings)
{
    // TODO add real data, no hardcoded. Also, loop for the array?
    JSON_Value *general_branch = json_value_init_array();
    JSON_Array *general_leaves = json_value_get_array(general_branch);

    JSON_Value *leaf_value = json_value_init_object();
    JSON_Object *leaf_object = json_value_get_object(leaf_value);

    json_object_set_string(leaf_object, JSON_KEY_SENSOR_CONF_ID, "0");
    json_object_set_string(leaf_object, JSON_KEY_SENSOR_CONF_NAME, "string");
    json_object_set_string(leaf_object, JSON_KEY_SENSOR_CONF_TYPE, "Uint8");

    JSON_Value *branch = json_value_init_array();
    JSON_Array *leaves = json_value_get_array(branch);
    JSON_Value *inner_leaf_value = json_value_init_object();
    JSON_Object *inner_leaf_object = json_value_get_object(inner_leaf_value);

    json_object_set_string(inner_leaf_object, JSON_KEY_SENSOR_CONF_KEY, "string");
    json_object_set_string(inner_leaf_object, JSON_KEY_SENSOR_CONF_VALUE, "string");
    json_array_append_value(leaves, inner_leaf_value);
    json_object_set_value(leaf_object, JSON_KEY_SENSOR_CONF_SETTINGS, branch);

    json_object_set_number(leaf_object, JSON_KEY_SENSOR_CONF_RATE, 0);

    json_array_append_value(general_leaves, leaf_value);

    // TODO check if there has been an error
    // if not error
    if (true)
    {
        *readings = malloc(strlen(json_serialize_to_string(general_branch)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(general_branch));
        return OK;
    }
    else
    {
        JSON_Value *rval = json_value_init_object();
        JSON_Object *robj = json_value_get_object(rval);
        json_object_set_string(robj, "message", "There has been an error reading the variables");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return ERROR;
    }
}

// TODO add functionality
int delete_sensors_configuration(char **values, query_pairs *queries)
{
    printf("Delete variables\n");

    // TODO For now, just print the query
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

int read_sensorgroups_configuration(char **readings)
{
    // TODO add real data, no hardcoded.
    JSON_Value *general_branch = json_value_init_array();
    JSON_Array *general_leaves = json_value_get_array(general_branch);

    for (size_t i = 0; i < 1; i++)
    {

        JSON_Value *leaf_value = json_value_init_object();
        JSON_Object *leaf_object = json_value_get_object(leaf_value);

        json_object_set_string(leaf_object, JSON_KEY_SENSORGROUPS_CONF_ID, "string");
        json_object_set_number(leaf_object, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE, i);

        JSON_Value *variable_list_branch = json_value_init_array();
        JSON_Array *variable_list_leaves = json_value_get_array(variable_list_branch);
        JSON_Value *variable_leaf_value;
        JSON_Object *variable_leaf_object;

        for (size_t j = 0; j < 1; j++)
        {
            variable_leaf_value = json_value_init_object();
            variable_leaf_object = json_value_get_object(variable_leaf_value);

            json_object_set_string(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID, "string");
            json_object_set_string(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME, "string");
            json_object_set_string(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE, "Uint8");

            JSON_Value *variable_settings_branch = json_value_init_array();
            JSON_Array *variable_settings_leaves = json_value_get_array(variable_settings_branch);
            JSON_Value *variable_settings_leaf_value = json_value_init_object();
            JSON_Object *variable_settings_leaf_object = json_value_get_object(variable_settings_leaf_value);
            json_object_set_string(variable_settings_leaf_object, JSON_KEY_SENSORGROUPS_CONF_KEY, "string");
            json_object_set_string(variable_settings_leaf_object, JSON_KEY_SENSORGROUPS_CONF_VALUE, "string");
            json_array_append_value(variable_settings_leaves, variable_settings_leaf_value);
            json_object_set_value(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS, variable_settings_branch);

            json_object_set_number(variable_leaf_object, JSON_KEY_SENSORGROUPS_CONF_SAMPLING_RATE, j);
            json_array_append_value(variable_list_leaves, variable_leaf_value);
        }
        json_object_set_value(leaf_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST, variable_list_branch);
        json_array_append_value(general_leaves, leaf_value);
    }
    // TODO check if there has been an error
    // if not error
    if (true)
    {
        printf("No hay error, vamos a devolver true\n");
        *readings = malloc(strlen(json_serialize_to_string(general_branch)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(general_branch));
        return OK;
    }
    else
    {
        JSON_Value *rval = json_value_init_object();
        JSON_Object *robj = json_value_get_object(rval);
        json_object_set_string(robj, "message", "There has been an error reading the subscriptions");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return ERROR;
    }
}

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

    // TODO add real data, no hardcoded.
    json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "ready");
    //json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "executing");
    //json_object_set_string(robj, JSON_KEY_MONITORING_AGENT_STATUS, "error");

    // TODO check if there has been an error
    // if not error
    if (true)
    {
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return OK;
    }
    else
    {
        rval = json_value_init_object();
        robj = json_value_get_object(rval);
        json_object_set_string(robj, "message", "There has been an error reading the status");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return ERROR;
    }

    printf("Read monitoring agent status\n");
    return OK;
}

int cmd_execute_configuration(char **values)
{

    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_CMD_EXECUTE_ORDER) != NULL)
    {
        char *cmd = strdup(json_object_get_string(jobj, JSON_KEY_CMD_EXECUTE_ORDER));
        printf("{\n\t\"%s\": %s\n}\n", JSON_KEY_CMD_EXECUTE_ORDER, cmd);

        if (strcmp(cmd, "start") == 0)
        {
            // TODO change status
            printf("Starting...\n");
            strcpy(*values, "");
            return OK;
        }
        else if (strcmp(cmd, "stop") == 0)
        {
            // TODO change status
            printf("Stoping...\n");
            strcpy(*values, "");
            return OK;
        }
        else
        {
            // TODO add payload
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

    strcpy(*values, "");
    return OK;
}

int read_sensor_measurements(char **readings, query_pairs *queries)
{

    // TODO for now, just print the query
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

    // TODO add real data, no hardcoded. Also, loop for the array?
    JSON_Value *general_branch = json_value_init_array();
    JSON_Array *general_leaves = json_value_get_array(general_branch);

    // TODO size depending on measurements
    for (size_t i = 0; i < 5; i++)
    {
        JSON_Value *leaf_value = json_value_init_object();
        JSON_Object *leaf_object = json_value_get_object(leaf_value);

        json_object_set_string(leaf_object, JSON_KEY_SENSOR_MEASUREMENTS_ID, "string");

        JSON_Value *sensor_data = json_value_init_object();
        JSON_Object *sensor_data_object = json_value_get_object(sensor_data);
        json_object_set_string(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_NAME, "string");
        json_object_set_string(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_TYPE, "Uint8");
        json_object_set_string(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_VALUE, "string");
        json_object_set_number(sensor_data_object, JSON_KEY_SENSOR_MEASUREMENTS_TIMESTAMP, i);
        json_object_set_value(leaf_object, JSON_KEY_SENSOR_MEASUREMENTS_DATA, sensor_data);

        json_array_append_value(general_leaves, leaf_value);
    }

    // TODO check if there has been an error
    // if not error
    if (true)
    {
        printf("No hay error, vamos a devolver true\n");
        *readings = malloc(strlen(json_serialize_to_string(general_branch)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(general_branch));
        return OK;
    }
    else
    {
        JSON_Value *rval = json_value_init_object();
        JSON_Object *robj = json_value_get_object(rval);
        json_object_set_string(robj, "message", "There has been an error reading the sensor measurements");
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));
        return ERROR;
    }
}
