/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _ADEPTNESS_ERROR_H_
#define _ADEPTNESS_ERROR_H_ 1

#include "os.h"

#define ADEPTNESS_OK \
    (adeptness_error) { .code = 0, .reason = "Success" }

typedef struct adeptness_error
{
    uint32_t code;
    const char *reason;
} adeptness_error;

#endif
