/** @file impl.c
 *  @brief Specific adeptness microservice
 *
 * Copyright(c) 2019
 * Ikerlan
 */

/* Include */
#include "impl.h"

/* Functions */

/**
 * @brief  Adeptness  template get random
 *
 * Random measures 
 *
 * @param impl ...
 *
 * @return Result of parsing
 */
bool adeptness_get_random(void *impl)
{
    myAdeptnessService_state *st = (myAdeptnessService_state *)impl;
    //unsigned char buffer[DEF_BUFF_SIZE];

    st->logical_data = rand() % 100;

    return true;
}

/**
 * @brief  Stop
 *
 * Stop performs any final actions before the device service is terminated.
 *
 * @param impl ...
 * @param force ...
 */
void myAdeptnessService_stop(void *impl, bool force)
{
    printf("-- Adeptness service stopped\n");
}
