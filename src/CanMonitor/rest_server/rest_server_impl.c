#include "rest_server_impl.h"
#include "../implementation/impl.h"
#include "../implementation/sensor.h"
#include "../implementation/sensorgroup.h"

adeptness_rest_response myAdeptnessService_get_handler(void *impl, const char *devname, char *url, char **readings, query_pairs *queries)
{
    if (strcmp(url, URL_CONNECTION) == 0)
    {
        return read_connection_configuration(readings);
    }
    else if (strcmp(url, URL_SENSORS_CONF) == 0)
    {
        return read_sensors_configuration(readings);
    }
    else if (strcmp(url, URL_SENSORGROUPS) == 0)
    {
        return read_sensorgroups_subscription(readings);
    }
    else if (strcmp(url, URL_AGENT_STATUS) == 0)
    {
        return read_monitoring_agent_status(readings);
    }
    else if (strcmp(url, URL_SENSORS_VAL) == 0)
    {
        return read_sensor_measurements(readings, queries);
    }
    else
    {
        return RESOURCE_NOT_FOUND;
    }
}

adeptness_rest_response myAdeptnessService_put_handler(void *impl, const char *devname, char *url, char **values, query_pairs *queries)
{
    if (strcmp(url, URL_SETUP) == 0)
    {
        return update_microservice_configuration(values);
    }
    else if (strcmp(url, URL_CONNECTION) == 0)
    {
        return update_connection_configuration(values);
    }
    else if (strcmp(url, URL_SENSORS_CONF) == 0)
    {
        return update_sensors_configuration(values, queries);
    }
    else if (strcmp(url, URL_SENSORGROUPS) == 0)
    {
        return update_sensorgroups_subscription(values, queries);
    }
    else if (strcmp(url, URL_CMD_EXECUTE) == 0)
    {
        return cmd_execute_configuration(values);
    }
    else
    {
        return RESOURCE_NOT_FOUND;
    }
}

adeptness_rest_response myAdeptnessService_post_handler(void *impl, const char *devname, char *url, char **values, query_pairs *queries)
{
    if (strcmp(url, URL_SENSORS_CONF) == 0)
    {
        return create_sensors_configuration(values);
    }
    else if (strcmp(url, URL_SENSORGROUPS) == 0)
    {
        return create_sensorgroups_subscription(values);
    }
    else
    {
        return RESOURCE_NOT_FOUND;
    }
}

adeptness_rest_response myAdeptnessService_delete_handler(void *impl, const char *devname, char *url, char **values, query_pairs *queries)
{    
    if (strcmp(url, URL_SENSORS_CONF) == 0)
    {
        return delete_sensors_configuration(values, queries);
    }
    else if (strcmp(url, URL_SENSORGROUPS) == 0)
    {
        return delete_sensorgroups_subscription(values, queries);
    }
    else
    {
        return RESOURCE_NOT_FOUND;
    }
}
