/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _ADEPTNESS_REST_SERVER_H_
#define _ADEPTNESS_REST_SERVER_H_ 1

#include "adeptnessError.h"

struct adeptness_rest_server;
typedef struct adeptness_rest_server adeptness_rest_server;

typedef enum
{
  GET = 1,
  POST = 2,
  PUT = 4,
  PATCH = 8,
  DELETE = 16,
  UNKNOWN = 1024
} adeptness_http_method;

typedef enum
{
  OK = 1,
  RESOURCE_NOT_FOUND = 2,
  INCORRECT_PAYLOAD = 3,
  NO_CONFIG_DATA = 4
} adeptness_rest_response;

typedef int (*http_method_handler_fn)
(
  void *context,
  char *url,
  adeptness_http_method method,
  const char *upload_data,
  size_t upload_data_size,
  void **reply,
  size_t *reply_size,
  const char **reply_type
);

extern adeptness_rest_server *adeptness_rest_server_create(uint16_t port, adeptness_error *err);

extern void adeptness_rest_server_register_handler(adeptness_rest_server *svr, const char *url, adeptness_http_method method, void *context, http_method_handler_fn handler);

extern void adeptness_rest_server_destroy (adeptness_rest_server *svr);

#endif
