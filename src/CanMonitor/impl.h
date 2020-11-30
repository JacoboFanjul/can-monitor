/** @file impl.h
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 2019
 * Ikerlan
 */

#ifndef _ADEPTNESS_MYSERVICE_H
#define _ADEPTNESS_MYSERVICE_H

/* Includes */
#include "common/adeptnessSvc.h"
#include "common/parson.h"

#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

/* Macro */
#define ERR_CHECK(x)                                          \
    if (x.code)                                               \
    {                                                         \
        fprintf(stderr, "Error: %d: %s\n", x.code, x.reason); \
        adeptness_service_free(service);                      \
        return x.code;                                        \
    }

/* Default values */

#define DEF_GW_NAME "Target-1"
#define DEF_SVC_NAME "adeptness-service"
#define DEF_POLLING_INTERVAL_S 1
#define DEF_MAX_JSON_SIZE 1000
#define DEF_MAX_GW_ID_SIZE 50
#define DEF_BUFF_SIZE 4096

/* Structs */
typedef struct myAdeptnessService_state
{
    char *svcname;
    char gw_id[DEF_MAX_GW_ID_SIZE];
    char json[DEF_MAX_JSON_SIZE];
    uint64_t logical_data;
    unsigned int polling_interval;
    pid_t main_thread_pid;
} myAdeptnessService_state;

bool adeptness_get_random(void *impl);

// /* Service callbacks functions */

void myAdeptnessService_stop(void *impl, bool force);

#endif
