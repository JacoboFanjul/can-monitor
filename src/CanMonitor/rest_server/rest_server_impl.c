#include "rest_server_impl.h"
#include "../implementation/impl.h"

adeptness_rest_response get_config_connection_handler(char **readings)
{
    return read_connection_configuration(readings);
}

adeptness_rest_response get_config_sensors_handler(char **readings)
{
    return read_sensors_configuration(readings);
}

adeptness_rest_response get_config_sensorgroups_handler(char **readings)
{
    return read_sensorgroups_configuration(readings);
}

adeptness_rest_response get_agentStatus_handler(char **readings)
{
    return read_monitoring_agent_status(readings);
}

adeptness_rest_response get_sensors_measurements_handler(char **readings, query_pairs *queries)
{
    return read_sensor_measurements(readings, queries);
}

adeptness_rest_response put_setup_handler(char **values)
{
    return update_microservice_configuration(values);
}

adeptness_rest_response put_config_connection_handler(char **values)
{
    return update_connection_configuration(values);
}

adeptness_rest_response put_config_sensors_handler(char **values, query_pairs *queries)
{
    return update_sensors_configuration(values, queries);
}

adeptness_rest_response put_config_sensorgroups_handler(char **values, query_pairs *queries)
{
    return update_sensorgroups_configuration(values, queries);
}

adeptness_rest_response put_cmdExecute_handler(char **values)
{
    return cmd_execute_configuration(values);
}

adeptness_rest_response post_config_sensors_handler(char **values)
{
    return create_sensors_configuration(values);
}

adeptness_rest_response post_config_sensorgroups_handler(char **values)
{
    return create_sensorgroups_configuration(values);
}

adeptness_rest_response delete_config_sensors_handler(char **values, query_pairs *queries)
{
    return delete_sensors_configuration(values, queries);
}

adeptness_rest_response delete_config_sensorgroups_handler(char **values, query_pairs *queries)
{
    return delete_sensorgroups_configuration(values, queries);
}

adeptness_rest_response myAdeptnessService_get_handler(void *impl, const char *devname, char *url, char **readings, query_pairs *queries)
{
    if (strcmp(url, URL_CONNECTION) == 0)
    {
        return get_config_connection_handler(readings);
    }
    else if (strcmp(url, URL_SENSORS_CONF) == 0)
    {
        return get_config_sensors_handler(readings);
    }
    else if (strcmp(url, URL_SENSORGROUPS) == 0)
    {
        return get_config_sensorgroups_handler(readings);
    }
    else if (strcmp(url, URL_AGENT_STATUS) == 0)
    {
        return get_agentStatus_handler(readings);
    }
    else if (strcmp(url, URL_SENSORS_VAL) == 0)
    {
        return get_sensors_measurements_handler(readings, queries);
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
        return put_setup_handler(values);
    }
    else if (strcmp(url, URL_CONNECTION) == 0)
    {
        return put_config_connection_handler(values);
    }
    else if (strcmp(url, URL_SENSORS_CONF) == 0)
    {
        return put_config_sensors_handler(values, queries);
    }
    else if (strcmp(url, URL_SENSORGROUPS) == 0)
    {
        return put_config_sensorgroups_handler(values, queries);
    }
    else if (strcmp(url, URL_CMD_EXECUTE) == 0)
    {
        return put_cmdExecute_handler(values);
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
        return post_config_sensors_handler(values);
    }
    else if (strcmp(url, URL_SENSORGROUPS) == 0)
    {
        return post_config_sensorgroups_handler(values);
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
        return delete_config_sensors_handler(values, queries);
    }
    else if (strcmp(url, URL_SENSORGROUPS) == 0)
    {
        return delete_config_sensorgroups_handler(values, queries);
    }
    else
    {
        return RESOURCE_NOT_FOUND;
    }
}
