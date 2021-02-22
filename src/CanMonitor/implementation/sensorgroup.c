/** @file sensorgroup.c
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 2019
 * Ikerlan
 */

/* Include */
#include "sensorgroup.h"
#include "sensor.h"

#include "../CanMonitor.h"
#include "../rest_server/rest_server_impl.h"

/* Functions */
int create_sensorgroups_subscription(char **values)
{
    printf("-- Create sensorgroup subscription\n");
    
    JSON_Value *jval = json_parse_string(*values);
    JSON_Object *jobj = json_value_get_object(jval);

    if (json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_ID) != NULL && json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE) != NULL &&
    json_object_get_value(jobj, JSON_KEY_SENSORGROUPS_CONF_SENSOR_LIST) != NULL)
    {
        char *id = strdup(json_object_get_string(jobj, JSON_KEY_SENSORGROUPS_CONF_ID));
        printf("\t- Sensorgroup id: %s\n", id);
        sensorgroup *sg = htsg_get(sensorgroup_table, id);
        if (sg != NULL)
        {
            create_error_message(values, "A sensorgroup with that id already exists");
            printf("\t- Id %s, already exists\n", id);
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

            // Complete sensor config
            if (json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID) != NULL && json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME) != NULL &&
            json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE) != NULL && json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS) != NULL &&
            json_object_get_value(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SAMPLING_RATE) != NULL)
            {
                char *sensor_id = strdup(json_object_get_string(sensors_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID));
                sensor *sens = hts_get(sensors_table, sensor_id);

                if (sens != NULL)
                {
                    hts_delete(sensors_table, sensor_id);
                    printf("\t- Sensor %s deleted\n", sensor_id);
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
                printf("\t- Sensor %s created\n", sens->id);
                
                
                sensor_list[i] = malloc(sizeof(char*));
                strcpy(sensor_list[i], sensor_id);
                printf("\t- Sensor %s added to the list\n", sensor_id);
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
                    printf("\t- Sensor %s added to the list\n", sensor_id);
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
        sg->last_publish_time = (struct timeval){0};
        htsg_put(sensorgroup_table, sg->id, sg);

        printf("\t- Sensorgroup %s created\n", sg->id);

        strcpy(*values, "");
        extern ms_status status;
        if (status != running)
        {
            status = configured;
        }
        return OK;
    }
    else
    {
        create_error_message(values, "The sensorgroup information is not valid");
        return ERROR;
    }
}

// TODO find a way for rollback if it is not succesful
int update_sensorgroups_subscription(char **values, query_pairs *queries)
{
    printf("-- Update sensorgroups subscriptions\n");
    int response = delete_sensorgroups_subscription(values, queries);
    if (response != OK)
    {
        return response;
    }
    response = create_sensorgroups_subscription(values);
    if (response != OK)
    {
        return response;
    }

    strcpy(*values, "");
    printf("\t- Sensorgroups updated\n");
    return OK;
}

int read_sensorgroups_subscription(char **readings)
{
    printf("-- Read Sensorgroups Configuration\n");
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
            printf("\t- Sensorgroup id: %s\n", sg->id);
            json_object_set_number(leaf_object, JSON_KEY_SENSORGROUPS_CONF_PUBLISH_RATE, sg->publish_rate);

            JSON_Value *sensors_array_value = json_value_init_array();
            JSON_Array *sensors_array_object = json_value_get_array(sensors_array_value);
            for (int i = 0; i < sg->sensorcount; i++)
            {
                sensor *sens = hts_get(sensors_table, sg->sensor_list[i]);
                JSON_Value *sensor_value = json_value_init_object();
                JSON_Object *sensor_object = json_value_get_object(sensor_value);
                json_object_set_string(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID, sens->id);
                printf("\t\t- Sensor %s\n", sens->id);
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

                json_object_set_value(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SETTINGS, settings_array_value);

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
    printf("\t- Finished reading sensorgroups\n");
    return OK;

}

// TODO find a way for rollback if it is not succesful
int delete_sensorgroups_subscription(char **values, query_pairs * queries)
{
    printf("-- Delete subscription\n");

    query_pairs *tmp;
    tmp = queries;
    char* id = NULL;
    while(tmp != NULL)
    {
        if(strcmp(tmp->name, QUERY_KEY_ID) == 0)
        {
            id = strdup(tmp->value);
            printf("\t- Sensorgroup %s\n", id);
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
    printf("\t- Deleted sensorgroups %s\n", id);
    return OK;
}
