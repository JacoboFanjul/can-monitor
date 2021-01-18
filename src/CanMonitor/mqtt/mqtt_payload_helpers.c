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

    printf("Create discovery payload\n");
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

const char * create_data_payload(double lift01Speed, int Lift01FloorLocation)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    JSON_Value *branch = json_value_init_array();
    JSON_Array *leaves = json_value_get_array(branch);

    JSON_Value *leaf_value_1 = json_value_init_object();
    JSON_Object *leaf_object_1 = json_value_get_object(leaf_value_1);  

    json_object_set_string(leaf_object_1, "bn", "urn:ngsi-ld:Sensor:Lift01Speed");
    json_object_set_string(leaf_object_1, "n", "varValue");
    json_object_set_number(leaf_object_1, "v", lift01Speed);
    json_object_set_number(leaf_object_1, "bt", tv.tv_sec * 1000 + tv.tv_usec / 1000);
    json_array_append_value(leaves, leaf_value_1);

    leaf_value_1 = json_value_init_object();
    leaf_object_1 = json_value_get_object(leaf_value_1);
    
    json_object_set_string(leaf_object_1, "bn", "urn:ngsi-ld:Sensor:Lift01FloorLocation");
    json_object_set_string(leaf_object_1, "n", "varValue");
    json_object_set_number(leaf_object_1, "v", Lift01FloorLocation );
    json_object_set_number(leaf_object_1, "bt", tv.tv_sec * 1000 + tv.tv_usec / 1000);
    json_array_append_value(leaves, leaf_value_1);

    return json_serialize_to_string(branch);
}
