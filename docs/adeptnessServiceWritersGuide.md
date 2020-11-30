Notes on writing an adeptness service
---------------------------------

This is a template for an adeptness service. Every adeptness service is web server responding to a well-known RESTfull API and a MQTT publisher/subscriber. 

Related with the rest server, fundamentally every new Adeptness Service is composed of a number of callbacks. These callbacks are provided by the genric AdeptnessSvc to allow the service to respond to different events. These callbacks (adeptness_callbacks) are as follows:


* get
* put
* stop

An adeptness service must provide an implementation of each callback. A small amount of setup is required of a new adeptness service, this is usually performed in the main. An adeptness_service should be created, containing, amongst other fields the adeptness_callbacks and an impldata pointer which is passed back every time a callback is invoked. The service must then call adeptness_service_start, upon exit the service should call adeptness_service_stop.



Get
---
The Get handler deals with incoming requests to get data from a service. The following information is provided to the service developer:

* void *impl - The impldata pointer given as part of edgex_device_service.
* char *devname - The name of the device being queried.
* char *url Info in url after path /adeptnessMs/specific/.
* adeptness_commandresult * readings - Once a reading has been taken from a service, the resulting value is placed into the readings. T



Put
---
The Put handler deals with requests to write/transmit data to a service. It is provided with the same set of metadata as the GET callback. However, this time the put handler should write the data provided to the variables in the service. The process of using the metadata provided to perform the correct protocol-specific write/put action is similar to that of performing a get.



Stop
----
The stop handler is called when adeptness_service_stop is called. This handler should be used to clean-up all service specific resources. Typically this would involve freeing any resources allocated during execution of the  service.
