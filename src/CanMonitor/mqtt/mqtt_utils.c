#include "mqtt_utils.h"
#include "../CanMonitor.h"

#include <string.h>

extern int mqtt_qos;

static MQTTClient mqtt_client;

int initialize_mqtt(char *mqttBrokerHost, int mqttBrokerPort, int mqttQoS, char *mqttUsername, char *clientIdentifier) {
    printf("-- Initialize MQTT:\n");
    /* mosquitto init */
    printf("\t- HOST: %s\n", mqttBrokerHost);
    printf("\t- PORT: %d\n", mqttBrokerPort);
    printf("\t- QoS: %d\n", mqttQoS);
    printf("\t- USERNAME: %s\n", mqttUsername);

    char mqtt_host[100];
    sprintf(mqtt_host, "tcp://%s:%d", mqttBrokerHost, mqttBrokerPort);

    MQTTClient_create(&mqtt_client, mqtt_host, clientIdentifier, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions connOpts = MQTTClient_connectOptions_initializer;
    connOpts.cleansession = 1;
    connOpts.reliable = 0;
    connOpts.username = mqttUsername;

    MQTTClient_setCallbacks(mqtt_client, NULL, NULL, NULL, NULL);
    printf("-- MQTT client initialized\n");
    mqtt_qos = mqttQoS;
    int rc;
    if ((rc = MQTTClient_connect(mqtt_client, &connOpts)) != MQTTCLIENT_SUCCESS) {
        printf("ERR: Failed to connect, return code %d\n", rc);
        return -1;
    }
    printf("-- MQTT client connected to broker\n");
    return 0;
}

void clean_mqtt() {
    MQTTClient_disconnect(mqtt_client, 1000);
}

void publish(char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = mqtt_qos;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(mqtt_client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(mqtt_client, token, 1000L);
}
