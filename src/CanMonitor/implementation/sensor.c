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
#include "tables/table_can.h"

extern ms_status status;
extern HashTableSensors *sensors_table;
extern TableCan *can_ids_table;

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

        uint32_t can_id = 0;
        uint32_t init_bit = 0;
        uint32_t end_bit = 0;
        uint8_t flag = 0;

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
                        sscanf(can_id_str, "%"SCNu32, &can_id);
                        ++flag;
                    }
                    else if (strcmp(key, "init-bit") == 0)
                    {
                        const char *init_bit_str = json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_VALUE);
                        // TODO find more robust method
                        sscanf(init_bit_str, "%"SCNu32, &init_bit);
                        ++flag;
                    }
                    else if (strcmp(key, "end-bit") == 0)
                    {
                        const char *end_bit_str = json_object_get_string(settingObject, JSON_KEY_SENSOR_CONF_VALUE);
                        // TODO find more robust method
                        sscanf(end_bit_str, "%"SCNu32, &end_bit);
                        ++flag;
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
            if (flag != 3)
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
        //table_can_put(can_ids_table, sens->can_id, strdup(sens->id));
    }
    else
    {
        create_error_message(values, "The sensor information is not valid");
        return ERROR;
    }
    
    strcpy(*values, "");

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
            sensor *sens = hts_delete(sensors_table, id);
            if (sens != NULL)
            {
                table_can_delete(can_ids_table, sens->can_id, strdup(sens->id));
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

    // Borrar de las listas de sensorgroups
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

int read_sensor_measurements(char **readings, query_pairs *queries)
{
    printf("-- Read Sensor Values\n");
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
                    printf("\t- Sensor %s: %s\n", sens->id, sens->value);

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
                printf("\t- Sensor %s: %s\n", sensor->id, sensor->value);
                listptr = listptr->next;
            }
        }

        *readings = malloc(strlen(json_serialize_to_string(general_branch)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(general_branch));
        return OK;
    }
}
