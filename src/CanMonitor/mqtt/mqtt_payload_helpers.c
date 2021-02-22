#include <time.h>
#include <sys/time.h>

#include "mqtt_payload_helpers.h"

#include "../common/json.h"
#include "../common/parson.h"

#include "../CanMonitor.h"

extern int rest_server_port;
extern char *mqtt_broker_host;
extern int mqtt_broker_port;
extern char *monitor_id;
extern int mqtt_qos;


/**
 * @brief  create_discovery_payload
 *
 * Creates the payload to send in the discovery message V2.
 */
const char *create_discovery_payload(void)
{
    JSON_Value *rval = json_value_init_object();
    JSON_Object *robj = json_value_get_object(rval);

    json_object_set_string(robj, "id", monitor_id);
    json_object_set_string(robj, "name", monitor_id);
    json_object_set_string(robj, "microservice-type", "monitor-agent");
    
    JSON_Value *branch = json_value_init_array();
    JSON_Array *leaves = json_value_get_array(branch);

    JSON_Value *leaf_value_1 = json_value_init_object();
    JSON_Object *leaf_object_1 = json_value_get_object(leaf_value_1);
    json_object_set_string(leaf_object_1, "endpoint-type", "mqtt");
    json_object_set_string(leaf_object_1, "ip", mqtt_broker_host);
    json_object_set_number(leaf_object_1, "port", mqtt_broker_port);
    json_object_set_number(leaf_object_1, "qos", mqtt_qos);
    json_array_append_value(leaves, leaf_value_1);

    JSON_Value *leaf_value_2 = json_value_init_object();
    JSON_Object *leaf_object_2 = json_value_get_object(leaf_value_2);
    json_object_set_string(leaf_object_2, "endpoint-type", "http");
    json_object_set_string(leaf_object_2, "ip", "");
    json_object_set_number(leaf_object_2, "port", rest_server_port);
    json_object_set_number(leaf_object_2, "qos", 0);
    json_array_append_value(leaves, leaf_value_2);

    json_object_set_value(robj, "endpoints", branch);

    return json_serialize_to_string(rval);

}

const char * create_data_payload(sensorgroup *sg)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    JSON_Value *branch = json_value_init_array();
    JSON_Array *leaves = json_value_get_array(branch);

    for (int i = 0; i < sg->sensorcount; i++)
    {
        sensor *sens = hts_get(sensors_table, sg->sensor_list[i]);

        JSON_Value *sensor_value = json_value_init_object();
        JSON_Object *sensor_object = json_value_get_object(sensor_value);

        // TODO create with real data
        json_object_set_string(sensor_object, "bn", strdup(sens->id));
        json_object_set_string(sensor_object, "n", strdup(sens->name));
        json_object_set_string(sensor_object, "v", strdup(sens->value));
        json_object_set_number(sensor_object, "bt", sens->timestamp);

        json_array_append_value(leaves, sensor_value);
    }

    return json_serialize_to_string(branch);
    
            // for (int i = 0; i < sg->sensorcount; i++)
            // {
            //     sensor *sens = hts_get(sensors_table, sg->sensor_list[i]);
            //     JSON_Value *sensor_value = json_value_init_object();
            //     JSON_Object *sensor_object = json_value_get_object(sensor_value);
            //     json_object_set_string(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_ID, sens->id);
            //     json_object_set_string(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_NAME, sens->name);
            //     json_object_set_string(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_TYPE, sens->type);

            //     JSON_Value *settings_array_value = json_value_init_array();
            //     JSON_Array *settings_array_object = json_value_get_array(settings_array_value);

            //     JSON_Value *settings_value = json_value_init_object();
            //     JSON_Object *settings_object = json_value_get_object(settings_value);
            //     json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_KEY, "can-id");
            //     char value[20];
            //     sprintf(value, "%d", sens->can_id);
            //     json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE, value);
            //     json_array_append_value(settings_array_object, settings_value);

            //     settings_value = json_value_init_object();
            //     settings_object = json_value_get_object(settings_value);
            //     json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_KEY, "init-bit");
            //     sprintf(value, "%d", sens->init_bit);
            //     json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE, value);
            //     json_array_append_value(settings_array_object, settings_value);

            //     settings_value = json_value_init_object();
            //     settings_object = json_value_get_object(settings_value);
            //     json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_KEY, "end-bit");
            //     sprintf(value, "%d", sens->end_bit);
            //     json_object_set_string(settings_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_VALUE, value);
            //     json_array_append_value(settings_array_object, settings_value);

            //     json_object_set_value(sensor_object, JSON_KEY_SENSOR_CONF_SETTINGS, settings_array_value);

            //     json_object_set_number(sensor_object, JSON_KEY_SENSORGROUPS_CONF_SENSOR_SAMPLING_RATE, sens->sampling_rate);
            //     json_array_append_value(sensors_array_object, sensor_value);
            // }
}
