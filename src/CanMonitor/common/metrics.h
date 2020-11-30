/*
 * Copyright (c) 2019
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _EDGEX_DEVICE_METRICS_H_
#define _EDGEX_DEVICE_METRICS_H_ 1

#include "rest-server.h"

#include <stddef.h>

extern int adeptness_handler_metrics(void *ctx, char *url, adeptness_http_method method, const char *upload_data, size_t upload_data_size, void **reply, size_t *reply_size, const char **reply_type);

#endif
