/** @file sensor.c
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 2019
 * Ikerlan
 */

/* Include */
#include "sensor.h"
#include "sensorgroup.h"

#include "../CanMonitor.h"
#include "../rest_server/rest_server_impl.h"


/* Functions */
int create_sensors_configuration(char **values)
{
    printf("-- Create sensor configuration\n");

    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_ID) != NULL && json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_NAME) != NULL &&
        json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_TYPE) != NULL &&  json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_RATE) != NULL &&
        json_object_get_value(jobj, JSON_KEY_SENSOR_CONF_SETTINGS) != NULL)
    {
        char *id = strdup(json_object_get_string(jobj, JSON_KEY_SENSOR_CONF_ID));
        printf("\t- Sensor id: %s\n", id);

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

    extern ms_status status;
    if (status != running)
    {
        status = configured;
    }

    printf("\t- Sensor created\n");
    return OK;
}

int update_sensors_configuration(char **values, query_pairs *queries)
{
    printf("-- Update variables configuration\n");
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
    printf("\t- Sensor updated\n");
    return OK;
}

int delete_sensors_configuration(char **values, query_pairs *queries)
{
    printf("-- Delete sensor configuration\n");
    query_pairs *tmp;
    tmp = queries;
    char* id = NULL;
    while(tmp != NULL)
    {
        if(strcmp(tmp->name, QUERY_KEY_ID) == 0)
        {
            id = strdup(tmp->value);
            printf("\t- Sensor id: %s\n", id);
            extern HashTableSensors *sensors_table;
            sensor *sens = hts_delete(sensors_table, id);
            if (sens != NULL)
            {
                printf("\t- Sensor deleted\n");
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
        printf("\t- Delete sensor request without id\n");
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
    printf("-- Read Sensors Configuration\n");
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

            printf("\t- Sensor id: %s\n", sensor->id);

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

    printf("\t- Finished reading sensor configuration\n");
    return OK;
}
