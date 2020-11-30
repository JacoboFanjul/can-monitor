#ifndef _REST_SERVER_H
#define _REST_SERVER_H

#include "../common/adeptnessSvc.h"

// /* Service callbacks functions */

adeptness_rest_response myAdeptnessService_get_handler(void *impl, const char *devname, char *url, char **readings);
adeptness_rest_response myAdeptnessService_put_handler(void *impl, const char *devname, char *url, char **values);
adeptness_rest_response myAdeptnessService_post_handler(void *impl, const char *devname, char *url, char **values);

#endif
