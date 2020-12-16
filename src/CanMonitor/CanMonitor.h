#ifndef _MYADEPTNESSSERVICE_H
#define _MYADEPTNESSSERVICE_H

#include <bits/types/sig_atomic_t.h>

// static char *mqtt_discovery_topic = "adms/v1/discovery";
// static char *mqtt_data_topic = "adms/v1/monitor-agent/data";

enum ms_status{ready, configured, running, error, exit_ms};

extern sig_atomic_t ms_status;

extern int rest_server_port;
extern char *mqtt_broker_host;
extern int mqtt_broker_port;
extern char *monitor_id;
extern char *mqtt_username;
extern int mqtt_qos;

int getInfoFromEnviromentVariables (void);

int read_config_file(char *config_file);

#endif
