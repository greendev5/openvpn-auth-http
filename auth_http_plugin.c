/* openvpn-auth-http -- An OpenVPN plugin for do accounting.
 *
 * Copyright (C) ****** VPN Service 2014 <i.muzichuk@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#include "plugin_utils.h"
#include "deferred_handlers.h"
#include "auth_http_plugin.h"

struct auth_http_plugin_context
{
    deferred_queue_t *c_queue;
    plugin_options_t *c_optns;
};

extern auth_http_plugin_context_t * create_auth_http_plugin_context(const char *path_to_opt_file)
{
    auth_http_plugin_context_t *context = (auth_http_plugin_context_t*)malloc(
        sizeof(auth_http_plugin_context_t));
    memset(context, 0, sizeof(auth_http_plugin_context_t));

    context->c_queue = create_deferred_queue();
    if (!context->c_queue) {
        PLUGIN_ERROR("Can not create deferred queue.");
        goto error;
    }

    context->c_optns = open_plugin_options(path_to_opt_file);
    if (!context->c_optns) {
        PLUGIN_ERROR("Can not open plugin options file: %s.", path_to_opt_file);
        goto error;
    }

    return context;

error:
    if (context->c_queue)
        free_deferred_queue(context->c_queue);
    if (context->c_optns)
        free_plugin_options(context->c_optns);
    free(context);
    return NULL;
}

extern void free_auth_http_plugin_context(auth_http_plugin_context_t *context)
{
    free_deferred_queue(context->c_queue);
    free_plugin_options(context->c_optns);
    free(context);
}

OPENVPN_PLUGIN_DEF openvpn_plugin_handle_t OPENVPN_PLUGIN_FUNC(openvpn_plugin_open_v2)(
    unsigned int *type_mask, 
    const char *argv[],
    const char *envp[],
    struct openvpn_plugin_string_list **return_list)
{
    const char *verb_str = NULL;
    auth_http_plugin_context_t *context = NULL;

    if (curl_global_init(CURL_GLOBAL_ALL)) {
        PLUGIN_ERROR("Can not init curl.");
        return NULL;
    }

    /* Init logginig */
    verb_str = get_openvpn_env("verb", envp);
    if (verb_str)
        init_plugin_logging(atoi(verb_str));
    else
        init_plugin_logging(3);

    if (string_array_len(argv) < 2) {
        PLUGIN_ERROR("Missed path to file with settings.");
        goto error;
    }

     context = create_auth_http_plugin_context(argv[1]);
     if (!context)
        goto error;

    /* Intercept the --auth-user-pass-verify, --client-connect and --client-disconnect callback. */
    *type_mask = OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_AUTH_USER_PASS_VERIFY)
               | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_CLIENT_CONNECT)
               | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_CLIENT_DISCONNECT);

    return (openvpn_plugin_handle_t)context;

error:
    if (context)
        free(context);
    clear_plugin_logging();
    curl_global_cleanup();
    return NULL;
}

OPENVPN_PLUGIN_DEF int OPENVPN_PLUGIN_FUNC(openvpn_plugin_func_v2)(
    openvpn_plugin_handle_t handle,
    const int type,
    const char *argv[],
    const char *envp[],
    void *per_client_context,
    struct openvpn_plugin_string_list **return_list)
{
    auth_http_plugin_context_t *context = (auth_http_plugin_context_t*)handle;
    deferred_queue_item_t *queue_item = NULL;

    if (type == OPENVPN_PLUGIN_AUTH_USER_PASS_VERIFY) {

        PLUGIN_DEBUG("openvpn_plugin_func_v2::OPENVPN_PLUGIN_AUTH_USER_PASS_VERIFY");

        /* We need to be sure that auth_control_file variable is in envp. */
        if (get_openvpn_env("auth_control_file", envp)) {
            queue_item = create_auth_user_pass_verify_handler(envp, context->c_optns);
            if (queue_item) {
                /* If item was created we are adding it to queue, and queue
                 * execute it in second thread without  blocking of openvpn
                 * process. For comunication with openvpn we use auth_control_file
                 * variable. OPENVPN_PLUGIN_FUNC_DEFERRED - notify openvpn 
                 * to wait when auth_control_file will be chaged
                 */
                if (add_item_to_deferred_queue(context->c_queue, queue_item) == DEFERRED_QUEUE_ADD_SUCCESS)
                    return OPENVPN_PLUGIN_FUNC_DEFERRED;
                else
                    free_deferred_queue_item(queue_item);
            }
        }

    } else if (type == OPENVPN_PLUGIN_CLIENT_CONNECT) {

        PLUGIN_DEBUG("openvpn_plugin_func_v2::OPENVPN_PLUGIN_CLIENT_CONNECT");

        queue_item = create_client_connect_handler(envp, context->c_optns);
        if (queue_item) {
            if (add_item_to_deferred_queue(context->c_queue, queue_item) == DEFERRED_QUEUE_ADD_SUCCESS)
                return OPENVPN_PLUGIN_FUNC_SUCCESS;
            else
                free_deferred_queue_item(queue_item);
        }
        return OPENVPN_PLUGIN_FUNC_SUCCESS; /* This handler is not required. */

    } else if (type == OPENVPN_PLUGIN_CLIENT_DISCONNECT) {

        PLUGIN_DEBUG("openvpn_plugin_func_v2::OPENVPN_PLUGIN_CLIENT_DISCONNECT");

        queue_item = create_client_disconnect_handler(envp, context->c_optns);
        if (queue_item) {
            if (add_item_to_deferred_queue(context->c_queue, queue_item) == DEFERRED_QUEUE_ADD_SUCCESS)
                return OPENVPN_PLUGIN_FUNC_SUCCESS;
            else
                free_deferred_queue_item(queue_item);
        }
        return OPENVPN_PLUGIN_FUNC_SUCCESS; /* This handler is not required. */

    }

    PLUGIN_DEBUG("openvpn_plugin_func_v2 return OPENVPN_PLUGIN_FUNC_ERROR.");
    /* Type is not registred in type_mask from openvpn_plugin_open_v2. */
    return OPENVPN_PLUGIN_FUNC_ERROR;
}

OPENVPN_PLUGIN_DEF void OPENVPN_PLUGIN_FUNC(openvpn_plugin_close_v1)(
    openvpn_plugin_handle_t handle)
{
    auth_http_plugin_context_t *context = (auth_http_plugin_context_t*)handle;
    free_auth_http_plugin_context(context);
    clear_plugin_logging();
    curl_global_cleanup();
}

