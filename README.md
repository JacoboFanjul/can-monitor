#  Implementation of a monitor-agent for CAN monitoring in the Adeptness project

Based on the [template](https://gitlab.com/adeptness/source/adeptness-example-ms-cdevice), this microservice allows to monitor sensors that send data via CAN. It implements the [monitoring-agent-interface](https://gitlab.com/adeptness/wp1/interfaces/monitoring-agent-interface). 


## Roadmap

- [ ] Add CAN read functiones
- [X] Add query support
- [X] Generate data structs for variables
- [X] Generate array/map/hashtables for variable structs
- [X] Complete delete support
- [X] Delete Polling Interval references
- [X] Affect data structures with API calls

## Building

### Prerequisites

* A Linux build host
* A version of GCC supporting C99.
* CMake version 3 or greater and make.
* Development libraries and headers for curl, microhttpd, yaml, libcbor, libuuid, and paho.

```
sudo apt install build-essential cmake make gcc curl libmicrohttpd-dev mosquitto-dev libmosquitto-dev libcbor-dev libcurl4-openssl-dev libyaml-dev uuid-dev libjsoncpp-dev
```

### Build

At the toplevel directory, run 
```
./scripts/build.sh
```
This retrieves dependencies and uses CMake to build the service. Subsequent
rebuilds may be performed by moving to the ```build/release``` or
```build/debug``` directories and running ```make```.

### Build dockerized
Launch the script for creating the multi-platform docker images. It requires to have buildx support, [see here](https://medium.com/@artur.klauser/building-multi-architecture-docker-images-with-buildx-27d80f7e2408). The script builds the docker image and uploads it to Ikerlan's harbor registry.
```
./build_dockers.sh
```

## Running the microservice

The microservice supports the needed parameters to be configured using a config file (legacy), and using environment variables.

### Standalone

#### Run using environment variables.
First export the variables, e.g.,

```
export REST_PORT=48891 MQTT_BROKER_IP=emq-adeptness.iot.ikerlan.es MQTT_BROKER_PORT=1883 MQTT_QOS=2 SVC_ID=can-monitor
```

Then, run the executable in ```build/debug``` or ```build/release```, i.e.,
```
cd build/debug
./CanMonitor
```

#### Run using config file (legacy).

In ```config``` directory, a file ```config.json``` allows to configure different parameters. 

Adapt the file to your needs and execute the microservice:
```
cd build/debug
./CanMonitor -c ../../conf/config.json
```

### Docker

The ```build_docker.sh``` script uploads the image to Ikerlan's Harbor registry, so the first step is to login there:
```
docker login registry.gitlab.com
```
Then, the image can be downloaded and the container launched.

#### Run container using environment variables.
The different communication parameters can be configured with environment variables. To do this, execute the following command:

```
docker run -p 48891:48891 -e REST_PORT=48891 -e MQTT_BROKER_IP=emq-adeptness.iot.ikerlan.es -e MQTT_BROKER_PORT=1883 -e MQTT_QOS=2 -e SVC_ID=can-monitor -it --init registry.bda.ikerlan.es/adeptness/can_monitor:latest
```

## About

When launching, this microservice sends a discovery message via MQTT to indicate that it is ready. Then, the sensors and sensorgroups need to be configured using the [REST API](https://gitlab.com/adeptness/wp1/interfaces/monitoring-agent-interface/-/blob/master/monitoring_agent_openapi.yaml). Import the POSTMAN collection in the test folder to have examples.

## MQTT broker
The default MQTT broker is on the Adeptness server. The IP and ports are:
- Internal: 172.16.56.58:1883
- Internal DNS: emq-adeptness.iot.ikerlan.es:1883
- External: 193.145.247.18:1884

The broker only allows one connection with the same id, so be carefull with the selected id, i.e., MQTTfx selects the same id by default in every new configuration.

## REST API
The example port used in this README has been 48891, but it can be changed. The can-monitoring microservice implements the [monitoring-agent-interface openapi](https://gitlab.com/adeptness/wp1/interfaces/monitoring-agent-interface/-/blob/master/monitoring_agent_openapi.yaml). See the specification for more information, but here is a shor summary of the supported requests.

### Ping (keep alive)
``` 
GET http://[host]/adms/v2/ping
```

### Metrics (info about the microservice)
``` 
GET http://[host]/adms/v2/microservice-info
```

### Performance (Metrics of the performance of the microservice)
``` 
GET http://[host]/adms/v2/performance
```

### Status
``` 
GET http://[host]​/adms​/v2​/status
```

### Setup (Update microservice enpoints)
``` 
GET http://[host]​​/adms​/v2​/setup
```

### Connection Configuration
``` 
GET http://[host]​​/adms​/v2​/monitoring-agent​/config​/connection
```
``` 
PUT http://[host]​​/adms​/v2​/monitoring-agent​/config​/connection
```

### Sensors Configuration (configuration of individual variables)
``` 
GET ​/adms​/v2​/monitoring-agent​/config​/sensors
```
``` 
PUT ​/adms​/v2​/monitoring-agent​/config​/sensors?id=xx
```
``` 
POST ​/adms​/v2​/monitoring-agent​/config​/sensors
```
``` 
DELETE ​/adms​/v2​/monitoring-agent​/config​/sensors?id=xx
```

### Sensorgroups Subscription (configuration of subscriptions for groups of variables to monitor)
``` 
GET http://[host]​​/adms​/v2​/monitoring-agent​/config​/sensorgroups
```
``` 
PUT http://[host]​​/adms​/v2​/monitoring-agent​/config​/sensorgroups?id=xx
```
``` 
POST http://[host]​​/adms​/v2​/monitoring-agent​/config​/sensorgroups
```
``` 
DELETE http://[host]​​/adms​/v2​/monitoring-agent​/config​/sensorgroups?id=xx
```

### Monitoring agent execution (status of the monitor, running or stopped)
``` 
GET http://[host]​/adms​/v2​/monitoring-agent​/monitoring-agent-status
```
``` 
PUT http://[host]​​/adms​/v2​/monitoring-agent​/cmd-execute
```

### Sensors Measurements (individual variable values)
The query is optional. If it is included, the request will answer with the values of that sensor, if it is not included, it sends the values of all sensors.
``` 
GET http://[host]​​/adms​/v2​/monitoring-agent​/sensors?id=xx
```

## MQTT topics
The MQTT topics are defined in the [monitoring-agent-interface asyncapi](https://gitlab.com/adeptness/wp1/interfaces/monitoring-agent-interface/-/blob/master/monitoring_agent_asyncapi.yaml) file.

### Discovery
It is the message that the microservice sends to announce that it is ready on startup or when its mqtt configuration has changed.

Topic: 
``` 
adms/v2/discovery
``` 

Example payload:

``` 
{
  "id" : "Adeptness",
  "name" : "Adeptness",
  "microservice-type" : "monitor-agent",
  "endpoints" : [ {
    "endpoint-type" : "mqtt",
    "ip" : "emq-adeptness.iot.ikerlan.es",
    "port" : 1883,
    "qos" : 2
  }, {
    "endpoint-type" : "http",
    "ip" : "",
    "port" : 48891,
    "qos" : 0
  } ]
}
```

### Sensorgroup values
These are the messages with the value of the sensors that are being monitored. 

Topic:
```
adms/v2/monitoring-agent/{monitorId}/{subscriptionId}
```
where MonitorId is the monitor-id defined when launching the microservice and subscriptionId is the id of the sensorgroup whose information is being sent, e.g, ```adms/v2/monitor-agent/Adeptness/sg_1```.

Example payload:
```
[ {
  "bn" : "id_1",
  "n" : "Speed_1",
  "v" : "5",
  "bt" : 1613724851123
}, {
  "bn" : "id_2",
  "n" : "Speed_2",
  "v" : "4",
  "bt" : 1613724851123
} ]
```