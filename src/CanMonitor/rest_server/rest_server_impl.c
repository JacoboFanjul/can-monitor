#include "rest_server_impl.h"
#include "../impl.h"

adeptness_rest_response myAdeptnessService_get_handler(void *impl, const char *devname, char *url, char **readings)
{
    myAdeptnessService_state *st = (myAdeptnessService_state *)impl;

    printf("GET on value: %s\n", url);
    if (strcmp(url, "Data") == 0)
    {
        JSON_Value *rval = json_value_init_object();
        JSON_Object *robj = json_value_get_object(rval);

        json_object_set_number(robj, "data", st->logical_data);
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));

        printf("Value: %s\n", *readings);
    }
    else if (strcmp(url, "PollingInterval") == 0)
    {
        JSON_Value *rval = json_value_init_object();
        JSON_Object *robj = json_value_get_object(rval);

        json_object_set_number(robj, "PollingInterval", st->polling_interval);
        *readings = malloc(strlen(json_serialize_to_string(rval)) * sizeof(char));
        strcpy(*readings, json_serialize_to_string(rval));

        printf("Value: %s\n", *readings);
    }
    else
    {
        return RESOURCE_NOT_FOUND;
    }

    return OK;
}

adeptness_rest_response myAdeptnessService_put_handler(void *impl, const char *devname, char *url, char **values)
{
    myAdeptnessService_state *st = (myAdeptnessService_state *)impl;

    printf("PUT on %s, with %s\n", url, *values);

    if (strcmp(url, "PollingInterval") == 0)
    {
        JSON_Value *jval = json_parse_string(*values);
        JSON_Object *jobj = json_value_get_object(jval);

        if (json_object_get_value(jobj, "PollingInterval") != NULL) 
        {
            st->polling_interval = (unsigned int) json_object_get_number(jobj, "PollingInterval");
            printf("Changing polling interval [%u] \n", st->polling_interval);
        }
        else 
        {
            return INCORRECT_PAYLOAD;
        }
    }
    if (strcmp(url, "setup") == 0)
    {
        // TODO this is a template, change with each microservice
        JSON_Value *jval = json_parse_string(*values);
        JSON_Object *jobj = json_value_get_object(jval);

        if (json_object_get_value(jobj, "endpoint-type") != NULL) 
        {
            char* ep_type = strdup(json_object_get_string(jobj, "endpoint-type"));
            printf("Endpoint type: %s \n", ep_type);
            if (json_object_get_value(jobj, "endpoint") != NULL)
            {
                char* ep_type = strdup(json_object_get_string(jobj, "endpoint"));
                printf("Endpoint: %s \n", ep_type);
            }
            else
            {
                return INCORRECT_PAYLOAD;
            }
        }
        else
        {
            return INCORRECT_PAYLOAD;
        }
    }
    else
    {
        return RESOURCE_NOT_FOUND;
    }
    return OK;
}

adeptness_rest_response myAdeptnessService_post_handler(void *impl, const char *devname, char *url, char **values)
{
  return RESOURCE_NOT_FOUND;
}
