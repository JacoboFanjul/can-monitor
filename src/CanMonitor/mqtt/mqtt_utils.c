#include "mqtt_utils.h"
#include "../CanMonitor.h"

#include <string.h>

extern int mqtt_qos;

void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = mqtt_qos;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Published:\tToken: %i\n\tTopic '%s'\n\tMessage: '%s'\n", token, topic, payload);
}
