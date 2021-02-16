#  Basic example for an adeptness service

Based on the KonnektSense device service, the code has been reduced as much as possible in order to have a basic service that allows you to serve a REST API and publish an MQTT message.


## Roadmap

- [X] Add query support
- [X] Generate data structs for variables
- [X] Generate array/map/hashtables for variable structs
- [X] Complete delete support
- [X] Delete Polling Interval references
- [X] Affect data structures with API calls

## About

This service generates random number every 5 seconds and publish them through MQTT.
The service provides a restfull API described below.


## Prerequisites

* A Linux build host
* A version of GCC supporting C99.
* CMake version 3 or greater and make.
* Development libraries and headers for curl, microhttpd, yaml, libcbor and libuuid, mosquitto.

```
sudo apt install build-essential cmake make gcc curl libmicrohttpd-dev mosquitto-dev libmosquitto-dev libcbor-dev libcurl4-openssl-dev libyaml-dev uuid-dev libjsoncpp-dev
```

## Building

At the toplevel directory, run 
```
./scripts/build.sh
```
This retrieves dependencies and uses CMake to build the service. Subsequent
rebuilds may be performed by moving to the ```build/release``` or
```build/debug``` directories and running ```make```.

## Config file
In ```config``` directory, a file ```config.json``` allows to configure different parameters, only MQTT for now. To use the broker from inside Ikerlan, the host has to be ```emq.konnekt.ikerlan.es```, from outside ```193.145.247.18```.

## MQTT broker
The MQTT broker allows to connect only using the ```adeptness``` username, with no password. It also allows only one connection with the same id, so be carefull with the selected id, i.e., MQTTfx selects the same id by default in every new configuration.

## API
### GET Ping (keep alive)
``` 
GET http://IP:48890/adeptnessMs/ping
```

### GET Metrics (see doc metrics.md)
``` 
GET http://IP:48890/adeptnessMs/metrics
```


### Get Data (random number)
``` 
GET http://IP:48890/adeptnessMs/specific/Data
```

### Get Data MQTT (random number)
Subcribe to  http://193.145.247.18:1883/
Topic konnekt/adeptness/data

This can be changed on src/c/mqtt/config.h
The payload is a JSON with the following format that can be changed on src/c/mqtt/mqtt_payload_helpers.c 

``` 
{"value":XX,"valueType":"Number","timestamp":"DD/MM/YY HH:MM:SS"}
```

### Get Discovery MQTT
Subscribe to http://193.145.247.18:1883/ on the topic: ``` konnekt/adeptness/discovery``` 

The topic does not follow the specification due to the current architecture.

Payload:

``` 
{"id":1234,
	"name":"Monitor_Agent_1",
	"monitortype": "monitor-agent",
	"endpoints": [
		{"endpoint-type":"mqtt","endpoint":"example.com:1883"},
		{"endpoint-type":"mqtt","endpoint":"example.com:1884"},
		{"endpoint-type":"mqtt","endpoint":"example.com:1885"}
	]
}
```


## BUILDING DOCKER IMAGE:

At the toplevel directory, run: 

```
docker build -t registry.gitlab.com/adeptness/source/adeptness-example-ms-c/adeptness-example-ms-c:0.0.2 .
```

A Docker image tagged "my-adeptness-service" should have been built. This can be checked by running:
docker image ls


## RUNNING DOCKER CONTAINER (NO NEED TO PREVIOUSLY BUILD THE DOCKER IMAGE: IT PULLS FROM THE REGISTRY)

To run the Docker container with the ADEPTNESS service, first authenticate into the registry with:

```
docker login registry.gitlab.com
```

And then run:

```
docker run -p 48890:48890 -it --init registry.gitlab.com/adeptness/source/adeptness-example-ms-c/adeptness-example-ms-c:0.0.2
```

To stop the container while being executed, press Ctrl+C

## RUNNING DOCKER CONTAINER WITH EXTERNAL CONFIG FILE

To pass an external configuration file to the container, it is necessary to mount the directory that contains the *config.json* file. To do this, execute the following command:

```
docker run -p 48890:48890 -v $PWD/conf/config.json:/adeptness/conf/config.json -it --init registry.gitlab.com/adeptness/source/adeptness-example-ms-c/adeptness-example-ms-c:0.0.2
```

## RUNNING DOCKER CONTAINER WITH ENVIRONMENT VARIABLES

The different communication parameters can be configured with environment variables. To do this, execute the following command:

```
docker run -p 48890:48890 -e REST_PORT=48890 -e MQTT_BROKER_IP=emq-adeptness.iot.ikerlan.es -e  MQTT_BROKER_PORT=1883 -e MQTT_QOS=2 -e SVC_ID=Adeptness -it --init registry.gitlab.com/adeptness/source/adeptness-example-ms-c/adeptness-example-ms-c:0.0.2

## PUSHING DOCKER IMAGE TO ADEPTNESS REGISTRY

Once a new Docker image is generated with its corresponding name (registry.gitlab.com/adeptness/source/adeptness-example-ms-c/adeptness-example-ms-c) and tag (0.0.2), in the form name:tag, first login into the registry:

```
docker login registry.gitlab.com
```

Then, push the image to the registry, using a new tag:

```
docker push registry.gitlab.com/adeptness/source/adeptness-example-ms-c/adeptness-example-ms-c:0.0.2
```

## DOCKER IMAGE REGISTRY VERSION HISTORY:

0.0.0 : Initial ADEPTNESS microservice, featuring MQTT publisher and REST server.

0.0.1 : Addition of Discovery message.

0.0.4 : Environment variables support & SenML payload example

0.0.5 : Changed MQTT library from Mosquitto to Paho

0.0.6 : Changed interface to v2