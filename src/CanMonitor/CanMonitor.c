/** @file MyAdeptnessService.c
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 20
 * Ikerlan
 */

/* Include */
#include "impl.h"
#include <time.h>
#include <sys/time.h>

#include "CanMonitor.h"

#include "rest_server/rest_server_impl.h"

#include "mqtt/mqtt_payload_helpers.h"
#include "mqtt/mqtt_utils.h"

static char *mqtt_discovery_topic = "adms/v2/discovery";
static char *mqtt_data_topic = "adms/v2/monitor-agent/urn:ngis.ld:DeployableComp:MonitorCan01Edge01/urn:ngis.ld:SensorGroup:MonitorCAN01_Group01";

/* Global vars */
static sig_atomic_t running = true;

int mid_sent = -1;

bool ready_for_repeat = false;

// struct mosq_config *cfg = NULL;
// struct mosquitto *mosq = NULL;

static volatile int keepRunning = 1;

char *mqtt_username;

int rest_server_port;
char *mqtt_broker_host;
int mqtt_broker_port;
char *monitor_id;
int mqtt_qos;

/* Functions */

/**
 * @brief  Signal handler
 *
 * Handles signals to close gracefully
 *
 * @param i Signal.
 */
static void int_handler(int i)
{
    running = (i != SIGINT);
}

/**
 * @brief  User signal handler
 *
 * Handles signals to wake up sleeping thread
 *
 * @param i Signal.
 */
static void usr_handler(int i)
{
    printf("Wake up! Config has been changed\n");
}

/**
 * @brief  Usage
 *
 * Help menu for console.
 */
static void usage(void)
{
    printf("Options: \n");
    printf("   -c, --config         : Use Config file, mandatory\n");
    printf("   -h, --help           : Show this text\n");
    printf("   -n, --name=<name>    : Set the device service name\n");
}

/**
 * @brief  testArg
 *
 * Check input arguments
 */
static bool testArg(int argc, char *argv[], int *pos, const char *pshort, const char *plong, char **var)
{
    if (strcmp(argv[*pos], pshort) == 0 || strcmp(argv[*pos], plong) == 0)
    {
        if (*pos < argc - 1)
        {
            (*pos)++;
            *var = argv[*pos];
            (*pos)++;
            return true;
        }
        else
        {
            printf("Option %s requires an argument\n", argv[*pos]);
            exit(0);
        }
    }
    char *eq = strchr(argv[*pos], '=');
    if (eq)
    {
        if (strncmp(argv[*pos], pshort, eq - argv[*pos]) == 0 || strncmp(argv[*pos], plong, eq - argv[*pos]) == 0)
        {
            if (strlen(++eq))
            {
                *var = eq;
                (*pos)++;
                return true;
            }
            else
            {
                printf("Option %s requires an argument\n", argv[*pos]);
                exit(0);
            }
        }
    }
    return false;
}

int read_config_file(char *config_file)
{

    printf("Config file: %s\n", config_file);

    if (access(config_file, F_OK) == -1)
    {
        printf("Config file does not exist\n");
        return 2;
    }

    JSON_Value *config_json = json_parse_file(config_file);
    JSON_Object *json_object = json_value_get_object(config_json);

    rest_server_port = json_object_get_number(json_object, "rest_server_port");
    printf("REST server port: %d\n", rest_server_port);

    mqtt_broker_host = strdup(json_object_get_string(json_object, "mqtt_broker_ip"));
    printf("MQTT broker ip: %s\n", mqtt_broker_host);
    mqtt_broker_port = json_object_get_number(json_object, "mqtt_broker_port");
    printf("MQTT broker port: %d\n", mqtt_broker_port);
    mqtt_qos = json_object_get_number(json_object, "mqtt_qos");
    printf("MQTT QoS : %d\n", mqtt_qos);
    monitor_id = strdup(json_object_get_string(json_object, "mqtt_id"));
    printf("MQTT id: %s\n", monitor_id);
    mqtt_username = strdup(json_object_get_string(json_object, "mqtt_username"));
    printf("MQTT username: %s\n", mqtt_username);

    //const char *mqtt_publish_topic = NULL;
    //mqtt_publish_topic = json_object_get_string(json_object, "mqtt_publish_topic");
    //printf("MQTT publish topic: %s\n", mqtt_publish_topic);

    json_value_free(config_json);

    return 0;
}

/**
 * @brief  getInfoFromEnviromentVariables
 *
 * Get info about the service identification & connectivity params from the enviroment settings
 */
int getInfoFromEnviromentVariables (void){
    const char* s = getenv("REST_PORT");
    if(s != NULL){
        rest_server_port = atoi(s);
        printf("REST server port: %d\n", rest_server_port);
    } else {
        printf("There is no REST server defined in the environment.\n");
        return -1;
    }
    s = getenv("MQTT_BROKER_IP");
    if(s != NULL){
        mqtt_broker_host = strdup(s); 
        printf("MQTT broker host: %s\n", mqtt_broker_host);
    } else {
        printf("There is no MQTT broker host defined in the environment.\n");
        return -1;
    }
    s = getenv("MQTT_BROKER_PORT");
    if(s != NULL){
        mqtt_broker_port = atoi(s); 
        printf("MQTT broker port: %d\n", mqtt_broker_port);
    } else {
        printf("There is no MQTT port defined in the environment.\n");
        return -1;
    }
    s = getenv("MQTT_QOS");
    if(s != NULL){
        mqtt_qos = atoi(s); 
        printf("MQTT QoS: %d\n", mqtt_qos);
    } else {
        printf("There is no MQTT QoS defined in the environment.\n");
        return -1;
    }
    s = getenv("SVC_ID");
    if(s != NULL){
        monitor_id = strdup(s); 
        printf("Service Id: %s\n", monitor_id);
    } else {
        printf("There is no Service Id defined in the environment.\n");
        return -1;
    }
    //s = getenv("MQTT_USERNAME");
    s = "adeptness";
    if(s != NULL){
        mqtt_username = strdup(s); 
        printf("MQTT username: %s\n", mqtt_username);
    } else {
        printf("There is no MQTT username defined in the environment.\n");
        return -1;
    }
    return 0;
}

/**
 * @brief  Main
 *
 * Entry point for service.
 *
 * @param argc Number of arguments.
 * @param argv Arguments.
 */
int main(int argc, char *argv[])
{
    /* Error var */
    adeptness_error e;
    e.code = 0;
    e.reason = NULL;

    /* Adeptness service structure initialization */
    myAdeptnessService_state st;
    memset(&st, 0, sizeof(myAdeptnessService_state));
    st.main_thread_pid = getpid();
    st.svcname = DEF_SVC_NAME;
    st.polling_interval = DEF_POLLING_INTERVAL_S;

    char *config_file = NULL;

    /* Check input parameters */
    int n = 1;
    while (n < argc)
    {
        if (strcmp(argv[n], "-h") == 0 || strcmp(argv[n], "--help") == 0)
        {
            usage();
            return 0;
        }

        if (testArg(argc, argv, &n, "-n", "--name", &st.svcname))
        {
            continue;
        }

        if (testArg(argc, argv, &n, "-c", "--config", &config_file))
        {
            printf("Configuration file: %s\n", config_file);
            continue;
        }

        printf("Unknown option %s\n", argv[n]);
        usage();
        return 0;
    }

    if (config_file == NULL)
    {
        if(getInfoFromEnviromentVariables() < 0) {
            printf("The configuration can not be recovered from a file or from environment variables\n");
            return 0;
        }

    } else {
        if (read_config_file(config_file) != 0)
        {
            return 0;
        }
    }

    /* Device Callbacks */
    adeptness_callbacks adeptnesscbs =
        {
            myAdeptnessService_get_handler, /* Get */
            myAdeptnessService_put_handler, /* Put */
            myAdeptnessService_post_handler, /* Post */
            myAdeptnessService_stop         /* Stop */
        };

    /* Initalise a new adeptness service */
    adeptness_service *service = adeptness_service_new(st.svcname, "1.0", &st, adeptnesscbs, rest_server_port, &e);
    ERR_CHECK(e);
    printf("-- Device service created\n");

    /* Start the adeptness service */
    adeptness_service_start(service, &e);
    ERR_CHECK(e);
    printf("-- Device service started\n");

    /* mosquitto init */
    printf("HOST: %s\n", mqtt_broker_host);
    printf("PORT: %d\n", mqtt_broker_port);
    printf("QoS: %d\n", mqtt_qos);
    printf("ID: %s\n", monitor_id);
    printf("USERNAME: %s\n", mqtt_username);

    char mqtt_host[100];
    sprintf(mqtt_host, "tcp://%s:%d", mqtt_broker_host, mqtt_broker_port);

    MQTTClient mqtt_client;
    MQTTClient_create(&mqtt_client, mqtt_host, monitor_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_setCallbacks(mqtt_client, NULL, NULL, NULL, NULL);
    printf("-- MQTT client initialized\n");

    int rc;
    if ((rc = MQTTClient_connect(mqtt_client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    printf("-- MQTT client connected to broker\n");

    signal(SIGINT, int_handler);
    signal(SIGUSR1, usr_handler);
    running = true;

    const char *discovery_payload = create_discovery_payload();
    printf("Discovery payload = %s\n", discovery_payload);
    publish(mqtt_client, mqtt_discovery_topic, strdup(discovery_payload));

    while (running)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        printf("Create payload.\n");
        double lift01Speed = ((double) rand()*(2.0-0.5)/(double)RAND_MAX-0.5);
        int Lift01FloorLocation = rand() % 10;
        const char *payload = create_data_payload(lift01Speed, Lift01FloorLocation);
        printf("Payload created: %s\n", payload);

        publish(mqtt_client, mqtt_data_topic, strdup(payload));
        //printf("LogicalData = %ld  %lu\n", st.logical_data, tv.tv_sec * 1000 + tv.tv_usec / 1000);
        printf("Sent JSON at [%lu]: %s\n", tv.tv_sec * 1000 + tv.tv_usec / 1000, payload);

        sleep(st.polling_interval);
    }

    /* Clean mqtt */
    MQTTClient_disconnect(mqtt_client, 1000);
    MQTTClient_destroy(&mqtt_client);

    /* Stop the adeptness service */
    adeptness_service_stop(service, true, &e);
    ERR_CHECK(e);

    /* Clean memory and exit */
    adeptness_service_free(service);

    return 0;
}
