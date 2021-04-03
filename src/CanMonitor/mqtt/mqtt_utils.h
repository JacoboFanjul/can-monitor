#include "MQTTClient.h"

void connlost(void *context, char *cause);

int initialize_mqtt(char *mqtt_broker_host, int mqtt_broker_port, int mqtt_qos, char *mqtt_username, char *client_identifier);

void clean_mqtt(void);

void publish(char* topic, char* payload);
