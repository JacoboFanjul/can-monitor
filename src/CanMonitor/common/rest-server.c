/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "rest-server.h"
#include "microhttpd.h"
#include "errorlist.h"

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "../rest_server/rest_server_impl.h"

typedef struct handler_list
{
    const char *url;
    uint32_t methods;
    void *context;
    http_method_handler_fn handler;
    struct handler_list *next;
} handler_list;

struct adeptness_rest_server
{
    struct MHD_Daemon *daemon;
    handler_list *handlers;
    pthread_mutex_t lock;
};

typedef struct http_context_s
{
    char *m_data;
    size_t m_size;
} http_context_t;

static adeptness_http_method method_from_string(const char *str)
{
    if (strcmp(str, "GET") == 0)
    {
        return GET;
    }
    if (strcmp(str, "POST") == 0)
    {
        return POST;
    }
    if (strcmp(str, "PUT") == 0)
    {
        return PUT;
    }
    if (strcmp(str, "PATCH") == 0)
    {
        return PATCH;
    }
    if (strcmp(str, "DELETE") == 0)
    {
        return DELETE;
    }
    return UNKNOWN;
}

static char *normalizeUrl(const char *url)
{
    /* Only deduplication of '/' is performed */

    char *res = malloc(strlen(url) + 1);
    const char *upos = url;
    char *rpos = res;
    while (*upos)
    {
        if ((*rpos++ = *upos++) == '/')
        {
            while (*upos == '/')
            {
                upos++;
            }
        }
    }
    *rpos = '\0';
    return res;
}

query_pairs *query_pairs_new(const char *name, const char *value, query_pairs *list)
{
    query_pairs *result = malloc(sizeof(query_pairs));
    result->name = strdup(name);
    result->value = strdup(value);
    result->next = list;
    return result;
}

static const char *query_paramlist[] = QUERY_KEYS;

static int queryIterator(void *p, enum MHD_ValueKind kind, const char *key, const char *value)
{
    unsigned i;
    for (i = 0; i < sizeof(query_paramlist) / sizeof(*query_paramlist); i++)
    {
        if (strcmp(key, query_paramlist[i]) == 0)
        {
            break;
        }
    }
    if (i == sizeof(query_paramlist) / sizeof(*query_paramlist))
    {
        return MHD_YES;
    }

    query_pairs **list = (query_pairs **)p;
    *list = query_pairs_new(key, value ? value : "", *list);

    return MHD_YES;
}

static int http_handler(void *this, struct MHD_Connection *conn, const char *url, const char *methodname, const char *version, const char *upload_data, size_t *upload_data_size, void **context)
{
    int status = MHD_HTTP_OK;
    http_context_t *ctx = (http_context_t *)*context;
    adeptness_rest_server *svr = (adeptness_rest_server *)this;
    struct MHD_Response *response = NULL;
    void *reply = NULL;
    size_t reply_size = 0;
    const char *reply_type = NULL;
    handler_list *h;

    /* First call used to create call context */

    if (ctx == 0)
    {
        ctx = (http_context_t *)malloc(sizeof(*ctx));
        ctx->m_size = 0;
        ctx->m_data = NULL;
        *context = (void *)ctx;
        return MHD_YES;
    }

    /* Subsequent calls transfer data */

    if (*upload_data_size)
    {
        ctx->m_data = (char *)realloc(ctx->m_data, ctx->m_size + (*upload_data_size) + 1);
        memcpy(ctx->m_data + ctx->m_size, upload_data, (*upload_data_size) + 1);
        ctx->m_size += *upload_data_size;
        *upload_data_size = 0;
        return MHD_YES;
    }
    *context = 0;

    /* Last call with no data handles request */

    adeptness_http_method method = method_from_string(methodname);

    if (strlen(url) == 0 || strcmp(url, "/") == 0)
    {
        if (method == GET)
        {
            /* List available handlers */
            reply_size = 0;
            pthread_mutex_lock(&svr->lock);
            for (h = svr->handlers; h; h = h->next)
            {
                reply_size += strlen(h->url) + 1;
            }
            char *buff = malloc(reply_size + 1);
            buff[0] = '\0';
            for (h = svr->handlers; h; h = h->next)
            {
                strcat(buff, h->url);
                strcat(buff, "\n");
            }
            reply = buff;
            pthread_mutex_unlock(&svr->lock);
        }
        else
        {
            status = MHD_HTTP_METHOD_NOT_ALLOWED;
        }
    }
    else
    {
        status = MHD_HTTP_NOT_FOUND;
        char *nurl = normalizeUrl(url);
        pthread_mutex_lock(&svr->lock);
        for (h = svr->handlers; h; h = h->next)
        {
            if ((h->url[strlen(h->url) - 1] == '/') ? (strncmp(nurl, h->url, strlen(h->url)) == 0) : (strcmp(nurl, h->url) == 0))
            {
                break;
            }
        }
        pthread_mutex_unlock(&svr->lock);
        if (h)
        {
            if (method & h->methods)
            {
                query_pairs *params = NULL;
                MHD_get_connection_values(conn, MHD_GET_ARGUMENT_KIND, queryIterator, &params);

                status = h->handler(h->context, nurl + strlen(h->url), method, params, ctx->m_data, ctx->m_size, &reply, &reply_size, &reply_type);
            }
            else
            {
                status = MHD_HTTP_METHOD_NOT_ALLOWED;
            }
        }
        free(nurl);
    }

    /* Send reply */

    if (reply_type == NULL)
    {
        reply_type = "text/plain";
    }
    if (reply == NULL)
    {
        reply = strdup("");
        reply_size = 0;
    }
    response = MHD_create_response_from_buffer(reply_size, reply, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, "Content-Type", reply_type);
    MHD_queue_response(conn, status, response);
    MHD_destroy_response(response);

    /* Clean up */

    if (ctx->m_data)
    {
        free(ctx->m_data);
    }
    free(ctx);
    //edgex_device_free_crlid ();
    return MHD_YES;
}

adeptness_rest_server *adeptness_rest_server_create(uint16_t port, adeptness_error *err)
{
    adeptness_rest_server *svr;
    uint16_t flags = MHD_USE_THREAD_PER_CONNECTION;
    /* config: flags |= MHD_USE_IPv6 ? */

    svr = malloc(sizeof(adeptness_rest_server));

    svr->handlers = NULL;
    pthread_mutex_init(&svr->lock, NULL);

    /* Start http server */

    //iot_log_debug (lc, "Starting HTTP server on port %d", port);
    printf("-- Starting HTTP server on port: %d\n", port);
    svr->daemon = MHD_start_daemon(flags, port, 0, 0, http_handler, svr, MHD_OPTION_END);
    if (svr->daemon == NULL)
    {
        *err = ADEPTNESS_HTTP_SERVER_FAIL;
        //iot_log_debug (lc, "MHD_start_daemon failed");
        printf("MHD_start_daemon failed \n");
        adeptness_rest_server_destroy(svr);
        return NULL;
    }
    else
    {
        return svr;
    }
}

void adeptness_rest_server_register_handler(adeptness_rest_server *svr, const char *url, uint32_t methods, void *context, http_method_handler_fn handler)
{
    handler_list *entry = malloc(sizeof(handler_list));
    entry->handler = handler;
    entry->url = url;
    entry->methods = methods;
    entry->context = context;
    pthread_mutex_lock(&svr->lock);
    entry->next = svr->handlers;
    svr->handlers = entry;
    pthread_mutex_unlock(&svr->lock);
}

void adeptness_rest_server_destroy(adeptness_rest_server *svr)
{
    handler_list *tmp;
    if (svr->daemon)
    {
        MHD_stop_daemon(svr->daemon);
    }
    while (svr->handlers)
    {
        tmp = svr->handlers->next;
        free(svr->handlers);
        svr->handlers = tmp;
    }
    pthread_mutex_destroy(&svr->lock);
    free(svr);
}
