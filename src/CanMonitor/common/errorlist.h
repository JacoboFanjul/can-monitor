/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _ADEPTNESS_ERRORLIST_H_
#define _ADEPTNESS_ERRORLIST_H_ 1

#define ADEPTNESS_OK                    (adeptness_error) { .code = 0, .reason = "Success" }
#define ADEPTNESS_NO_CONF_FILE          (adeptness_error) { .code = 1, .reason = "Unable to open configuration file" }
#define ADEPTNESS_CONF_PARSE_ERROR      (adeptness_error) { .code = 2, .reason = "Error while parsing configuration file" }
#define ADEPTNESS_NO_DEVICE_IMPL        (adeptness_error) { .code = 3, .reason = "Device implementation data was null" }
#define ADEPTNESS_BAD_CONFIG            (adeptness_error) { .code = 4, .reason = "Configuration is invalid" }
#define ADEPTNESS_HTTP_SERVER_FAIL      (adeptness_error) { .code = 5, .reason = "Failed to start HTTP server" }
#define ADEPTNESS_NO_DEVICE_NAME        (adeptness_error) { .code = 6, .reason = "No Device name was specified" }
#define ADEPTNESS_NO_DEVICE_VERSION     (adeptness_error) { .code = 7, .reason = "No Device version was specified" }
#define ADEPTNESS_NO_CTX                (adeptness_error) { .code = 8, .reason = "No connection context supplied" }
#define ADEPTNESS_INVALID_ARG           (adeptness_error) { .code = 9, .reason = "Invalid argument" }
// errors 10..13 superceded by ADEPTNESS_HTTP_ERROR
#define ADEPTNESS_DRIVER_UNSTART        (adeptness_error) { .code = 14, .reason = "Protocol driver initialization failed" }
#define ADEPTNESS_REMOTE_SERVER_DOWN    (adeptness_error) { .code = 15, .reason = "Remote server unresponsive" }
#define ADEPTNESS_PROFILE_PARSE_ERROR   (adeptness_error) { .code = 16, .reason = "Error while parsing device profile" }
#define ADEPTNESS_HTTP_CONFLICT         (adeptness_error) { .code = 17, .reason = "HTTP 409 Conflict" }
#define ADEPTNESS_CONSUL_RESPONSE       (adeptness_error) { .code = 18, .reason = "Unable to process response from consul" }
#define ADEPTNESS_PROFILES_DIRECTORY    (adeptness_error) { .code = 19, .reason = "Problem scanning profiles directory" }
#define ADEPTNESS_ASSERT_FAIL           (adeptness_error) { .code = 20, .reason = "A reading did not match a specified assertion string" }
#define ADEPTNESS_HTTP_ERROR            (adeptness_error) { .code = 21, .reason = "HTTP request failed" }
#endif
