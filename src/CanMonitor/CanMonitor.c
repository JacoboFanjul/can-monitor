/** @file MyAdeptnessService.c
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 20
 * Ikerlan
 */

/* Include */
#include <time.h>
#include <sys/time.h>

#include "CanMonitor.h"

#include "rest_server/rest_server_impl.h"

#include "mqtt/mqtt_payload_helpers.h"
#include "mqtt/mqtt_utils.h"

#define SENSORS_TABLE_SIZE 2
#define SENSORGROUPS_TABLE_SIZE 3

// TODO delete test part
#define MQTT_API_PREFIX "test/adms/v2"
#define DATA_TOPIC_PREFIX MQTT_API_PREFIX "/monitor-agent"
#define DISCOVERY_TOPIC MQTT_API_PREFIX "/discovery"


/* Global vars */
ms_status status = 0;
uint8_t restart_mqtt = 0;
uint8_t restart_http = 0;
uint8_t can_up = 0;

int mid_sent = -1;

bool ready_for_repeat = false;

HashTableSensors *sensors_table;
HashTableSensorgroups *sensorgroup_table;

//static volatile int keepRunning = 1;

char *mqtt_username;

int rest_server_port;
char *mqtt_broker_host;
int mqtt_broker_port;
char *monitor_id;
int mqtt_qos;

char *can_conf_id;
char *canport;
int bitrate;

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
    status = exit_ms;
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

    json_value_free(config_json);

    return 0;
}

/**
 * @brief  getInfoFromEnvironmentVariables
 *
 * Get info about the service identification & connectivity params from the environment settings
 */
int getInfoFromEnvironmentVariables (void){
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

#if DEV //TODO delete, only for developing
void print_sensor(sensor *sensor)
{
    printf("Printing new sensor\n");
    printf("\tSensor ID: %s\n", sensor->id);
    printf("\tSensor name: %s\n", sensor->name);
    printf("\tSensor type: %s\n", sensor->type);

    printf("\tSensor settings:\n");
    printf("\t\tCan ID: %d\n", sensor->can_id);
    printf("\t\tInit bit: %d\n", sensor->init_bit);
    printf("\t\tEnd bit: %d\n", sensor->end_bit);

    printf("\tSampling rate: %d\n", sensor->sampling_rate);
    
    printf("\tValue: %s\n", sensor->value);
    printf("\tTimestamp: %ld\n", sensor->timestamp);
}

void print_sensorgroup(sensorgroup *sensorgroup)
{
    printf("Printing new sensorgroup\n");
    printf("\tSensor ID: %s\n", sensorgroup->id);
    printf("\tPublish_rate: %d\n", sensorgroup->publish_rate);
    printf("\tLast published: %ld\n", sensorgroup->last_publish_time.tv_sec * 1000000 + sensorgroup->last_publish_time.tv_usec);
    printf("\tSensorlist:\n");
    for (size_t i = 0; i < sensorgroup->sensorcount; i++)
    {
        printf("\t\t%s\n", sensorgroup->sensor_list[i]);
    }
}

void create_dummy_struct()
{
    // 2 individual sensors
    sensor *sensor1 = malloc(sizeof (sensor));
    sensor1->id = strdup("id_1");
    sensor1->name = strdup("Name_1");
    sensor1->type = strdup("Uint8");
    sensor1->can_id = 1;
    sensor1->init_bit = 1;
    sensor1->end_bit = 10;
    sensor1->sampling_rate = 1;
    sensor1->value = strdup("Value_1");
    sensor1->timestamp = (11111111111);
    hts_put(sensors_table, sensor1->id, sensor1);

    sensor1 = malloc(sizeof (sensor));
    sensor1->id = strdup("id_2");
    sensor1->name = strdup("Name_2");
    sensor1->type = strdup("Uint8");
    sensor1->can_id = 2;
    sensor1->init_bit = 2;
    sensor1->end_bit = 20;
    sensor1->sampling_rate = 2;
    sensor1->value = strdup("Value_2");
    sensor1->timestamp = (22222222222);
    hts_put(sensors_table, sensor1->id, sensor1);

    // 1 sensorgroup with 1 sensor
    sensorgroup *sg = malloc(sizeof(sensorgroup));
    sg->id = strdup("id_11");
    sg->publish_rate = 11000;
    sg->last_publish_time = (struct timeval){0};

    sensor1 = malloc(sizeof (sensor));
    sensor1->id = strdup("id_3");
    sensor1->name = strdup("Name_3");
    sensor1->type = strdup("Uint8");
    sensor1->can_id = 3;
    sensor1->init_bit = 3;
    sensor1->end_bit = 30;
    sensor1->sampling_rate = 3;
    sensor1->value = strdup("Value_3");
    sensor1->timestamp = (33333333333);
    hts_put(sensors_table, sensor1->id, sensor1);

    sg->sensorcount = 1;
    char **sensor_list = malloc(sg->sensorcount * sizeof(char*));
    sensor_list[0] = malloc(sizeof(char*));
    strcpy(sensor_list[0], "id_3");
    sg->sensor_list = sensor_list;
    htsg_put(sensorgroup_table, sg->id, sg);

    // 1 sensorgroup with 2 sensors
    sg = malloc(sizeof(sensorgroup));
    sg->id = strdup("id_22");
    sg->publish_rate = 5000;
    sg->last_publish_time = (struct timeval){0};

    sensor1 = malloc(sizeof (sensor));
    sensor1->id = strdup("id_4");
    sensor1->name = strdup("Name_4");
    sensor1->type = strdup("Uint8");
    sensor1->can_id = 4;
    sensor1->init_bit = 4;
    sensor1->end_bit = 40;
    sensor1->sampling_rate = 4;
    sensor1->value = strdup("Value_4");
    sensor1->timestamp = (44444444444);
    hts_put(sensors_table, sensor1->id, sensor1);

    sensor1 = malloc(sizeof (sensor));
    sensor1->id = strdup("id_5");
    sensor1->name = strdup("Name_5");
    sensor1->type = strdup("Uint8");
    sensor1->can_id = 5;
    sensor1->init_bit = 5;
    sensor1->end_bit = 50;
    sensor1->sampling_rate = 5;
    sensor1->value = strdup("Value_5");
    sensor1->timestamp = (55555555555);
    hts_put(sensors_table, sensor1->id, sensor1);

    sg->sensorcount = 2;
    sensor_list = malloc(sg->sensorcount * sizeof(char*));
    sensor_list[0] = malloc(sizeof(char*));
    strcpy(sensor_list[0], "id_4");
    sensor_list[1] = malloc(sizeof(char*));
    strcpy(sensor_list[1], "id_5");
    sg->sensor_list = sensor_list;
    htsg_put(sensorgroup_table, sg->id, sg);

}

void print_struct()
{
    printf("**************************\n******** Sensors *********\n**************************\n");
    ListSensors *s_listptr;
    for (unsigned int i = 0; i < sensors_table->size; ++i) 
    {
        printf("%d\n", i);

        s_listptr = sensors_table->array[i];
        printf("\t--------\n");

        while (s_listptr != NULL)
        {
            printf("\tkey: ");
            printf("%s\n", s_listptr->key);
            printf("\tval: ");
            sensor *sensor = malloc(sizeof *sensor);
            sensor = s_listptr->sensor;
                
            printf("ID: %s", (char*)sensor->id);
            printf(". Name: %s", sensor->name);
            printf(". Type: %s", sensor->type);
            printf(". CanID: %d", sensor->can_id);
            printf(". Init bit: %d", sensor->init_bit);
            printf(". End bit: %d", sensor->end_bit);
            printf(". Sampling_rate: %d", sensor->sampling_rate);
            printf(". Value: %s", sensor->value);
            printf(". Timestamp: %ld\n", sensor->timestamp);

            s_listptr = s_listptr->next;
            printf("\t--------\n");
        }
        printf("\tNULL\n\t--------\n");
    }

    printf("**************************\n****** Sensorgroups ******\n**************************\n");
    ListSensorgroups *sg_listptr;
    for (unsigned int i = 0; i < sensorgroup_table->size; ++i) 
    {
        printf("%d\n", i);

        sg_listptr = sensorgroup_table->array[i];
        printf("\t--------\n");

        while (sg_listptr != NULL) 
        {
   
            printf("\tkey: ");
            printf("%s\n", sg_listptr->key);
            printf("\tval: ");
            sensorgroup *sg = sg_listptr->sensorgroup;
                
            printf("ID: %s", sg->id);
            printf(". Publish_rate: %d", sg->publish_rate);
            printf(". Last published: %f", sg->last_publish_time.tv_sec * 1.0 + (sg->last_publish_time.tv_usec / 1000) / 1000.0);

            printf(". Sensor count: %ld", sg->sensorcount);
            printf(". Sensorlist:");
            
            for (int i = 0; i < sg->sensorcount; i++)
            {
                printf(" %s", sg->sensor_list[i]);
            }
            printf(".\n");
            
            sg_listptr = sg_listptr->next;
            printf("\t--------\n");
        }
        printf("\tNULL\n\t--------\n");
    }
}
#endif 
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
        if(getInfoFromEnvironmentVariables() < 0) 
        {
            printf("The configuration can not be recovered from a file or from environment variables\n");
            return 0;
        }

    } else {
        if (read_config_file(config_file) != 0)
        {
            printf("Error parsing the config_file\n");
            return 0;
        }
    }

    /* Device Callbacks */
    adeptness_callbacks adeptnesscbs =
        {
            myAdeptnessService_get_handler,     /* Get */
            myAdeptnessService_put_handler,     /* Put */
            myAdeptnessService_post_handler,    /* Post */
            myAdeptnessService_delete_handler,  /* Delete */
            myAdeptnessService_stop             /* Stop */
        };

    /* Initalise a new adeptness service */
    adeptness_service *service = adeptness_service_new(st.svcname, "1.0", &st, adeptnesscbs, rest_server_port, &e);
    ERR_CHECK(e);
    printf("-- Device service created\n");

    /* Start the adeptness service */
    adeptness_service_start(service, &e);
    ERR_CHECK(e);
    printf("-- Device service started\n");

    initializeMQTT(mqtt_broker_host, mqtt_broker_port, mqtt_qos, mqtt_username, monitor_id);

    signal(SIGINT, int_handler);
    signal(SIGUSR1, usr_handler);
    status = unconfigured;
    restart_mqtt = 0;

    // Init CAN socket:
    int sockfd;
    sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = if_nametoindex(CAN_INTERFACE);

    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    // Set non blocking
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    publish(DISCOVERY_TOPIC, strdup(create_discovery_payload()));

    sensors_table = hts_create(SENSORS_TABLE_SIZE);
    sensorgroup_table = htsg_create(SENSORGROUPS_TABLE_SIZE);

    // TODO delete, only for dev
    #if DEV
    create_dummy_struct();
    print_struct();
    status = configured;
    #endif

    while (status != exit_ms)
    {
        if (restart_mqtt != 0)
        {
            clean_mqtt();
            initializeMQTT(mqtt_broker_host, mqtt_broker_port, mqtt_qos, mqtt_username, monitor_id);

            publish(DISCOVERY_TOPIC, strdup(create_discovery_payload()));
            restart_mqtt = 0;
            printf("-- MQTT connection reconfigured\n");
        }

        if (restart_http != 0)
        {
            adeptness_service_stop(service, true, &e);
            adeptness_service *service = adeptness_service_new(st.svcname, "1.0", &st, adeptnesscbs, rest_server_port, &e);
            ERR_CHECK(e);
            adeptness_service_start(service, &e);
            ERR_CHECK(e);
            restart_http = 0;
            printf("-- REST port reconfigured\n");
        }

        // TODO delete, only for dev
        #if AUTOSTART
        status = running;
        #endif

        if (status == configured || status == running)
        {
            if (EXISTS_CAN != 0)
            {
                // Init CAN frame identifier and Extended/Standard flag:
                struct can_frame frame;
                int ExtFlag;
                uint32_t can_id;

                ExtFlag = can_read(sockfd, &frame);

                struct timeval tv;
                gettimeofday(&tv, NULL);

                //if ((ExtFlag != 0) && (frame.can_id == 0x00040030))
                if (ExtFlag != 0)
                {
                    if (ExtFlag == 2) // Extended Frame Format
                    {
                        can_id = frame.can_id & CAN_EFF_MASK;
                    }
                    else // Standard Frame Format
                    {
                        can_id = frame.can_id & CAN_SFF_MASK;
                    }
                    if (can_id == 0x00040030)
                    {
                        printf("CAN ID: %08X\n", can_id);
                        double lift01Speed = ((double) rand()*(2.0-0.5)/(double)RAND_MAX-0.5);
                        int Lift01FloorLocation = frame.data[2] & 0x3F;
                        printf("Lift at floor %d\n", Lift01FloorLocation);
                    }
                }
            }
        }
            
        if (status == running)
        {
            ListSensorgroups *listptr;
            for (unsigned int i = 0; i < sensorgroup_table->size; i++)
            {
                listptr = sensorgroup_table->array[i];

                while (listptr != NULL) 
                {
                    sensorgroup *sg = malloc(sizeof(sensorgroup));
                    sg = listptr->sensorgroup;

                    struct timeval current_time;  
                    gettimeofday (&current_time, NULL);

                    uint64_t current_ms = current_time.tv_sec * 1000 + current_time.tv_usec / 1000;
                    uint64_t last_published_ms = sg->last_publish_time.tv_sec * 1000 + sg->last_publish_time.tv_usec / 1000;
                    if (current_ms - last_published_ms >= sg->publish_rate)
                    {
                        char mqtt_data_topic[200];
                        sprintf(mqtt_data_topic, "%s/%s/%s", DATA_TOPIC_PREFIX, monitor_id, sg->id);

                        const char *payload = create_data_payload(sg);
                        
                        publish(mqtt_data_topic, strdup(payload));
                        printf("%ld - Publish on %s: %s\n", current_ms, mqtt_data_topic, payload);
                        sg->last_publish_time.tv_sec = current_time.tv_sec;
                        sg->last_publish_time.tv_usec = current_time.tv_usec;

                    }
                    listptr = listptr->next;
                }
            }
        }
        // TODO find the correct number for sleep
        sleep(1);
    }

    hts_free(sensors_table);
    htsg_free(sensorgroup_table);

    /* Clean mqtt */
    clean_mqtt();

    /* Stop the adeptness service */
    adeptness_service_stop(service, true, &e);
    ERR_CHECK(e);

    /* Clean memory and exit */
    adeptness_service_free(service);

    return 0;
}
