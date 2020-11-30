/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _ADEPTNESS_SERVICE_H_
#define _ADEPTNESS_SERVICE_H_ 1

#include "rest-server.h"

#define MILLIS 1000
#define MICROS 1000000
#define NANOS 1000000000

/**
 * @brief Callback issued to handle GET requests for device readings.
 * @param impl The context data passed in when the service was created.
 * @param devname The name of the device to be queried.
 * @param url Info in url after path /adeptnessMs/specific/.
 * @param readings An array in which to return the requested readings.
 * @return true if the operation was successful, false otherwise.
 */

typedef adeptness_rest_response (*adeptness_handle_get)(void *impl, const char *devname, char *url, char **payload);

/**
 * @brief Callback issued to handle PUT requests for setting device values.
 * @param impl The context data passed in when the service was created.
 * @param devname The name of the device to be queried.
 * @param url Info in url after path /adeptnessMs/specific/.
 * @param values An array with the values to set. 
 * @return true if the operation was successful, false otherwise.
 */

typedef adeptness_rest_response (*adeptness_handle_put)(void *impl, const char *devname, char *url, char **payload);

/**
 * @brief Callback issued during service shutdown. The implementation
 * should stop processing and release any resources that were being used.
 * @param impl The context data passed in when the service was created.
 * @param force A 'force' stop has been requested. An unclean shutdown may be
 *              performed if necessary.
 */

typedef adeptness_rest_response (*adeptness_handle_post)(void *impl, const char *devname, char *url, char **payload);

/**
 * @brief Callback issued during service shutdown. The implementation
 * should stop processing and release any resources that were being used.
 * @param impl The context data passed in when the service was created.
 * @param force A 'force' stop has been requested. An unclean shutdown may be
 *              performed if necessary.
 */

typedef void (*adeptness_stop)(void *impl, bool force);

typedef struct adeptness_rest_callbacks
{
  adeptness_handle_get gethandler;
  adeptness_handle_put puthandler;
  adeptness_handle_post posthandler;
 
  adeptness_stop stop;
} adeptness_callbacks;

typedef enum 
{ 
  LOCKED, 
  UNLOCKED 
} adeptness_adminstate;

typedef enum 
{ 
  ENABLED, 
  DISABLED 
} adeptness_operatingstate;

struct adeptness_service
{
  const char *name;
  const char *version;
  void *userdata;
  adeptness_callbacks userfns;
  uint16_t port;
  atomic_bool *stopconfig;
  adeptness_rest_server *daemon;
  adeptness_operatingstate opstate;
  adeptness_adminstate adminstate;
  uint64_t starttime;

  pthread_mutex_t discolock;
};

//struct adeptness_service;
typedef struct adeptness_service adeptness_service;

/**
 * @brief Create a new device service.
 * @param name The device service name, used in logging, metadata lookups and
 *             and to scope configuration.
 * @param version The version string for this service. For information only.
 * @param impldata An object pointer which will be passed back whenever one of
 *                 the callback functions is invoked.
 * @param implfns Structure containing the device implementation functions. The
 *                SDK will call these functions in order to carry out its
 *                various actions.
 * @param err Nonzero reason codes will be set here in the event of errors.
 * @return The newly instantiated service
 */

adeptness_service *adeptness_service_new(const char *name, const char *version, void *impldata, adeptness_callbacks userfns, int32_t port, adeptness_error *err);

/**
 * @brief Start a device service.
 * @param svc The service to start.
 * @param registryURL If set, this identifies a registry implementation for the
 *                    service to use. The service will register itself and
                      obtain configuration from this registry. If no
 *                    configuration is available, it will be read from file and
 *                    uploaded to the registry ready for subsequent runs.
 * @param profile Configuration profile to use (may be null).
 * @param confDir Directory containing configuration files.
 * @param err Nonzero reason codes will be set here in the event of errors.
 */

void adeptness_service_start(adeptness_service *svc, adeptness_error *err);

/**
 * @brief Stop the event service. Any automatic events will be cancelled
          and the rest api for the device service will be shut down.
 * @param svc The device service.
 * @param force Force stop.
 * @param err Nonzero reason codes will be set here in the event of errors.
 */

void adeptness_device_service_stop(adeptness_service *svc, bool force, adeptness_error *err);

/**
 * @brief Free the device service object and associated resources.
 * @param svc The device service.
 */

void adeptness_service_free(adeptness_service *svc);

void adeptness_service_stop(adeptness_service *svc, bool force, adeptness_error *err);

uint64_t device_millitime(void);
#endif
