#include <time.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

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
char *create_discovery_payload(void) {
    JSON_Value *rval = json_value_init_object();
    JSON_Object *robj = json_value_get_object(rval);

    json_object_set_string(robj, "id", monitor_id);
    json_object_set_string(robj, "name", monitor_id);
    json_object_set_string(robj, "microservice-type", MS_TYPE);

    JSON_Value *branch = json_value_init_array();
    JSON_Array *leaves = json_value_get_array(branch);

    JSON_Value *leaf_value = json_value_init_object();
    JSON_Object *leaf_object = json_value_get_object(leaf_value);
    json_object_set_string(leaf_object, "endpoint-type", "mqtt");

    JSON_Value *endpoint_object_value = json_value_init_object();
    JSON_Object *endpoint_object = json_value_get_object(endpoint_object_value);

    json_object_set_string(endpoint_object, "ip", mqtt_broker_host);
    json_object_set_number(endpoint_object, "port", mqtt_broker_port);
    json_object_set_number(endpoint_object, "qos", mqtt_qos);
    char base_topic[100] = DATA_TOPIC_PREFIX;
    strcat(base_topic, "/");
    strcat(base_topic, monitor_id);
    json_object_set_string(endpoint_object, "base-topic", strdup(base_topic));
    json_object_set_value(leaf_object, "endpoint-config", endpoint_object_value);
    json_array_append_value(leaves, leaf_value);

    leaf_value = json_value_init_object();
    leaf_object = json_value_get_object(leaf_value);
    json_object_set_string(leaf_object, "endpoint-type", "http");
    endpoint_object_value = json_value_init_object();
    endpoint_object = json_value_get_object(endpoint_object_value);

    // TODO fix IP
    char *IPbuffer;
    struct hostent *host_entry;
    char hostbuffer[256];
    gethostname(hostbuffer, sizeof(hostbuffer));
    host_entry = gethostbyname(hostbuffer);
    IPbuffer = inet_ntoa(*((struct in_addr *) host_entry->h_addr_list[0]));
    json_object_set_string(endpoint_object, "ip", IPbuffer);

    // struct ifaddrs *ifap, *ifa;
    // struct sockaddr_in *sa;
    // char *addr;

    // getifaddrs (&ifap);
    // for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
    //     if (ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET) {
    //         sa = (struct sockaddr_in *) ifa->ifa_addr;
    //         addr = inet_ntoa(sa->sin_addr);
    //         printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
    //     }
    // }
    // freeifaddrs(ifap);


    json_object_set_string(endpoint_object, "ip", "127.0.0.1");
    json_object_set_number(endpoint_object, "port", rest_server_port);
    json_object_set_value(leaf_object, "endpoint-config", endpoint_object_value);
    json_array_append_value(leaves, leaf_value);

    json_object_set_value(robj, "endpoints", branch);

    return json_serialize_to_string(rval);

}

char *create_data_payload(sensorgroup *sg) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    JSON_Value *branch = json_value_init_array();
    JSON_Array *leaves = json_value_get_array(branch);

    for (int i = 0; i < sg->sensorcount; i++) {
        sensor *sens = hts_get(sensors_table, sg->sensor_list[i]);

        JSON_Value *sensor_value = json_value_init_object();
        JSON_Object *sensor_object = json_value_get_object(sensor_value);

        json_object_set_string(sensor_object, "bn", strdup(sens->id));
        json_object_set_string(sensor_object, "n", strdup(sens->name));
        json_object_set_string(sensor_object, "v", strdup(sens->value));
        json_object_set_number(sensor_object, "bt", sens->timestamp);

        json_array_append_value(leaves, sensor_value);
    }

    return json_serialize_to_string(branch);
}
