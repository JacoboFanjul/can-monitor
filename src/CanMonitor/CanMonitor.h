#ifndef _MYADEPTNESSSERVICE_H
#define _MYADEPTNESSSERVICE_H

// static char *mqtt_discovery_topic = "adms/v1/discovery";
// static char *mqtt_data_topic = "adms/v1/monitor-agent/data";


extern int rest_server_port;
extern char *mqtt_broker_host;
extern int mqtt_broker_port;
extern char *monitor_id;
extern char *mqtt_username;
extern int mqtt_qos;

int getInfoFromEnviromentVariables (void);

int read_config_file(char *config_file);

#endif
