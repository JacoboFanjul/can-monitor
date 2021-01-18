/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "adeptnessSvc.h"
#include "../CanMonitor.h"

#include "metrics.h"
#include "errorlist.h"
#include "rest-server.h"
#include "parson.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/utsname.h>

#include <microhttpd.h>

#include <stdio.h> 
#include <unistd.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <bits/types/sig_atomic_t.h>
  

#define ADEPTNESS_PING "/adms/v2/ping"
#define ADEPTNESS_INFO "/adms/v2/microservice-info"
#define ADEPTNESS_METRICS "/adms/v2/performance"
#define ADEPTNESS_STATUS "/adms/v2/status"
#define ADEPTNESS_SERVICE_SPECIFIC "/adms/v2/"

#define POOL_THREADS 8

adeptness_service *adeptness_service_new(const char *name, const char *version, void *impldata, adeptness_callbacks implfns, int32_t port, adeptness_error *err)
{
    if (impldata == NULL)
    {
        // iot_log_error
        //   (iot_log_default (), "edgex_device_service_new: no implementation object");
        printf("edgex_device_service_new: no implementation object\n");
        *err = ADEPTNESS_NO_DEVICE_IMPL;
        return NULL;
    }
    if (name == NULL || strlen(name) == 0)
    {
        // iot_log_error
        //   (iot_log_default (), "edgex_device_service_new: no name specified");
        printf("edgex_device_service_new: no name specified\n");

        *err = ADEPTNESS_NO_DEVICE_NAME;
        return NULL;
    }
    if (version == NULL || strlen(version) == 0)
    {
        // iot_log_error
        //   (iot_log_default (), "edgex_device_service_new: no version specified");
        printf("edgex_device_service_new: no version specified\n");
        *err = ADEPTNESS_NO_DEVICE_VERSION;
        return NULL;
    }

    *err = ADEPTNESS_OK;
    adeptness_service *result = malloc(sizeof(adeptness_service));
    memset(result, 0, sizeof(adeptness_service));
    result->name = name;
    result->version = version;
    result->userdata = impldata;
    result->port = port;
    result->userfns = implfns;
    result->starttime = device_millitime();
    pthread_mutex_init(&result->discolock, NULL);
    return result;
}

static int adeptness_ping_handler(void *ctx, char *url, adeptness_http_method method, query_pairs *queries, const char *upload_data, size_t upload_data_size, void **reply, size_t *reply_size, const char **reply_type)
{
    adeptness_service *svc = (adeptness_service *)ctx;
    *reply = strdup(svc->version);
    *reply_size = strlen(svc->version);
    *reply_type = "text/plain";
    return MHD_HTTP_OK;
}

static int adeptness_info_handler(void *ctx, char *url, adeptness_http_method method, query_pairs *queries, const char *upload_data, size_t upload_data_size, void **reply, size_t *reply_size, const char **reply_type)
{

    extern int rest_server_port;
    char *IPbuffer;
    struct hostent *host_entry; 
    char hostbuffer[256]; 
    gethostname(hostbuffer, sizeof(hostbuffer)); 
    host_entry = gethostbyname(hostbuffer); 
    IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0])); 

    JSON_Value *rval = json_value_init_object();
    JSON_Object *robj = json_value_get_object(rval);

    json_object_set_number(robj, "id", 0);
    json_object_set_string(robj, "name", "string");
    json_object_set_string(robj, "microservice-type", "monitor-agent");

    JSON_Value *branch = json_value_init_array();
    JSON_Array *leaves = json_value_get_array(branch);

    JSON_Value *leaf_value = json_value_init_object();
    JSON_Object *leaf_object = json_value_get_object(leaf_value);
    json_object_set_string(leaf_object, "endpoint-type", "http");
    json_object_set_string(leaf_object, "ip", IPbuffer);
    json_object_set_number(leaf_object, "port", rest_server_port);
    json_object_set_number(leaf_object, "qos", 0);
    json_array_append_value(leaves, leaf_value);
    json_object_set_value(robj, "endpoints", branch);

    *reply = json_serialize_to_string(rval);
    *reply_size = strlen(*reply);
    *reply_type = "application/json";
    json_value_free(rval);
    return MHD_HTTP_OK;
}

static int adeptness_status_handler(void *ctx, char *url, adeptness_http_method method, query_pairs *queries, const char *upload_data, size_t upload_data_size, void **reply, size_t *reply_size, const char **reply_type)
{
    JSON_Value *rval = json_value_init_object();
    JSON_Object *robj = json_value_get_object(rval);

    extern sig_atomic_t ms_status;
    
    // TODO check possible errors
    switch (ms_status)
    {
    case running:
        json_object_set_string(robj, "status", "running");
        break;
    case ready:
        json_object_set_string(robj, "status", "ready");
        break;
    case error:
        json_object_set_string(robj, "message", "The microservice is in an error state");
        break;
    default:
        json_object_set_string(robj, "message", "Unknown error");
        break;
    }

    *reply = json_serialize_to_string(rval);
    *reply_size = strlen(*reply);
    *reply_type = "application/json";
    json_value_free(rval);

    if (ms_status == running || ms_status == ready)
    {
        return MHD_HTTP_OK;
    }
    else
    {
        return MHD_HTTP_CONFLICT;
    }
}

static int adeptness_specific_handler(void *ctx, char *url, adeptness_http_method method, query_pairs *queries, const char *upload_data, size_t upload_data_size, void **reply, size_t *reply_size, const char **reply_type)
{
    adeptness_service *svc = (adeptness_service *)ctx;
    adeptness_rest_response response;
    char *result;

    if (method == GET)
    {
        response = svc->userfns.gethandler(svc->userdata, svc->name, url, &result, queries);

        if (response == OK)
        {
            *reply = result;
            *reply_size = strlen(result);
            *reply_type = "application/json";
            return MHD_HTTP_OK;
        }
        else if (response == ERROR)
        {
            *reply = result;
            *reply_size = strlen(result);
            *reply_type = "application/json";
            return MHD_HTTP_CONFLICT;
        }
        else
        {
            return MHD_HTTP_NOT_FOUND;
        }
    }
    else if (method == PUT)
    {
        if (upload_data_size == 0)
        {
            printf("PUT command recieved with no data \n");
            return MHD_HTTP_BAD_REQUEST;
        }
        JSON_Value *jval = json_parse_string(upload_data);
        if (jval == NULL)
        {
            printf("Payload did not parse as JSON \n");
            return MHD_HTTP_BAD_REQUEST;
        }

        result = strdup(upload_data);

        response = svc->userfns.puthandler(svc->userdata, svc->name, url, &result, queries);
        if (response == OK)
        {
            return MHD_HTTP_OK;
        }
        else if (response == RESOURCE_NOT_FOUND)
        {
            return MHD_HTTP_NOT_FOUND;
        }
        else if (response == ERROR)
        {
            *reply = result;
            *reply_size = strlen(result);
            *reply_type = "application/json";
            return MHD_HTTP_CONFLICT;
        }
        else if (response == INCORRECT_PAYLOAD)
        {
            return MHD_HTTP_BAD_REQUEST;
        }
        else
        {
            printf("Driver for %s failed on PUT\n", svc->name);
            return MHD_HTTP_INTERNAL_SERVER_ERROR;
        }
    }
    else if (method == POST)
    {
        if (upload_data_size == 0)
        {
            printf("POST command recieved with no data \n");
            return MHD_HTTP_BAD_REQUEST;
        }
        JSON_Value *jval = json_parse_string(upload_data);
        if (jval == NULL)
        {
            printf("Payload did not parse as JSON \n");
            return MHD_HTTP_BAD_REQUEST;
        }

        result = strdup(upload_data);

        response = svc->userfns.posthandler(svc->userdata, svc->name, url, &result, queries);
        if (response == OK)
        {
            return MHD_HTTP_OK;
        }
        else if (response == RESOURCE_NOT_FOUND)
        {
            return MHD_HTTP_NOT_FOUND;
        }
        else if (response == INCORRECT_PAYLOAD)
        {
            return MHD_HTTP_BAD_REQUEST;
        }
        else if (response == NO_CONFIG_DATA)
        {
            return MHD_HTTP_CONFLICT;
        }
        else if (response == ERROR)
        {
            *reply = result;
            *reply_size = strlen(result);
            *reply_type = "application/json";
            return MHD_HTTP_CONFLICT;
        }
        else
        {
            printf("Driver for %s failed on POST\n", svc->name);
            return MHD_HTTP_INTERNAL_SERVER_ERROR;
        }
        return MHD_HTTP_OK;
    }
    else if (method == DELETE)
    {
        // TODO
        response = svc->userfns.deletehandler(svc->userdata, svc->name, url, &result, queries);
        if (response == OK)
        {
            return MHD_HTTP_OK;
        }
        else if (response == RESOURCE_NOT_FOUND)
        {
            return MHD_HTTP_NOT_FOUND;
        }
        else if (response == INCORRECT_PAYLOAD)
        {
            return MHD_HTTP_BAD_REQUEST;
        }
        else if (response == NO_CONFIG_DATA)
        {
            return MHD_HTTP_CONFLICT;
        }
        else if (response == ERROR)
        {
            *reply = result;
            *reply_size = strlen(result);
            *reply_type = "application/json";
            return MHD_HTTP_CONFLICT;
        }
        else
        {
            printf("Driver for %s failed on DELETE\n", svc->name);
            return MHD_HTTP_INTERNAL_SERVER_ERROR;
        }
        return MHD_HTTP_OK;
    }
    return MHD_HTTP_METHOD_NOT_ALLOWED;
}

static void startConfigured(adeptness_service *svc, adeptness_error *err)
{

    svc->adminstate = UNLOCKED;
    svc->opstate = ENABLED;

    /* Start REST server now so that we get the callbacks on device addition */

    svc->daemon = adeptness_rest_server_create(svc->port, err);
    if (err->code)
    {
        return;
    }

    /* Register REST handlers */
    adeptness_rest_server_register_handler(svc->daemon, ADEPTNESS_SERVICE_SPECIFIC, GET | PUT | POST | DELETE, svc, adeptness_specific_handler);
    adeptness_rest_server_register_handler(svc->daemon, ADEPTNESS_PING, GET, svc, adeptness_ping_handler);
    adeptness_rest_server_register_handler(svc->daemon, ADEPTNESS_INFO, GET, svc, adeptness_info_handler);
    adeptness_rest_server_register_handler(svc->daemon, ADEPTNESS_METRICS, GET, svc, adeptness_handler_metrics);
    adeptness_rest_server_register_handler(svc->daemon, ADEPTNESS_STATUS, GET, svc, adeptness_status_handler);
}

void adeptness_service_start(adeptness_service *svc, adeptness_error *err)
{

    svc->starttime = device_millitime();

    startConfigured(svc, err);

    if (err->code == 0)
    {
        printf("Service started in: %ld ms\n", device_millitime() - svc->starttime);
        printf("Listening on port: %d\n", svc->port);
    }
}

void adeptness_service_stop(adeptness_service *svc, bool force, adeptness_error *err)
{
    *err = ADEPTNESS_OK;
    printf("Stop adeptness service\n");

    if (svc->daemon)
    {
        adeptness_rest_server_destroy(svc->daemon);
    }
    svc->userfns.stop(svc->userdata, force);

    printf("Stopped adeptness service\n");
}

void adeptness_service_free(adeptness_service *svc)
{
    if (svc)
    {
        pthread_mutex_destroy(&svc->discolock);
        free(svc);
    }
}

uint64_t device_millitime()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
    {
        return ts.tv_sec * MILLIS + ts.tv_nsec / (NANOS / MILLIS);
    }
    else
    {
        return (uint64_t)time(NULL) * MILLIS;
    }
}
