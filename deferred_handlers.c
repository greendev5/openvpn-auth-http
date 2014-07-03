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
#include <curl/curl.h>

#include "deferred_handlers.h"

/** Writes the result of the authentication to the auth control file (0: failure, 1: success).
 * @param filename The auth control file.
 * @param c The authentication result.
 */
static void write_auth_control_file(const char *filename, int res)
{
    FILE *fp = fopen(filename, "w");
    if (fp) {
        fprintf(fp, "%d", res);
        fclose(fp);
    }
}

/** Sent HTTP post request to url.
 * @return HTTP code from server or 0 in case of error.
 */
static int send_post(const char *url, const char *post_str)
{
    CURL *curl = NULL;
    CURLcode res;
    int r = 0;

    curl = curl_easy_init();
    if (!curl)
        return r;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r);
    } else {
        PLUGIN_DEBUG("curl fail: %s [%d].", curl_easy_strerror(res), res);
    }
    curl_easy_cleanup(curl);
    return r;
}

void default_handler_context_cleaner(void *handler_context)
{
    PLUGIN_DEBUG("Delete context of deferred handler.");
    deferred_handler_context_t *context = (deferred_handler_context_t*)handler_context;
    free_cloned_string_array(context->envp);
    free(context);
}

int notification_handler_func(void *handler_context)
{
    int r = 0;
    deferred_handler_context_t *context = (deferred_handler_context_t*)handler_context;
    
    static const char *exclude_list[] = {NULL}; /* Maybe later */
    char *json_envp = openvpn_envp_to_json((const char **)context->envp, exclude_list);

    PLUGIN_DEBUG("START - notification_handler_func.");
    PLUGIN_DEBUG("Data to post: %s. To URL: %s.", json_envp, context->handler_url);

    send_post(context->handler_url, json_envp);

    PLUGIN_DEBUG("END - notification_handler_func.");
    return r;
}

int auth_user_pass_verify_handler_func(void *handler_context)
{
    int r = 0;
    int http_code;
    deferred_handler_context_t *context = (deferred_handler_context_t*)handler_context;
    const char *auth_control_file = get_openvpn_env(
        "auth_control_file", (const char**)(context->envp));
    const char *username = get_openvpn_env("username", (const char**)(context->envp));
    
    /* I do not see any reasons why web server should know auth_control_file
     * to auth user. */
    static const char *exclude_list[] = {"auth_control_file", NULL};
    char *json_envp = openvpn_envp_to_json((const char **)context->envp, exclude_list);

    if (!username)
        username = "";

    PLUGIN_DEBUG("START - auth_user_pass_verify_handler_func.");
    PLUGIN_DEBUG("Data to post: %s. To URL: %s. auth_control_file: %s.",
                 json_envp, context->handler_url, auth_control_file);

    http_code = send_post(context->handler_url, json_envp);
    PLUGIN_DEBUG("HTTP Response code: %d.", http_code);

    if (http_code == 200) {
        PLUGIN_LOG("Auth for user %s SUCCESS.", username);
        write_auth_control_file(auth_control_file, 1);
    }
    else {
        PLUGIN_LOG("Auth for user %s FAILED.", username);
        write_auth_control_file(auth_control_file, 0);
    }

    free(json_envp);

    PLUGIN_DEBUG("END - auth_user_pass_verify_handler_func.");
    return r;
}

deferred_queue_item_t * create_auth_user_pass_verify_handler(
    const char *envp[], plugin_options_t *options)
{
    deferred_handler_context_t *handler_context = (deferred_handler_context_t*)malloc(
        sizeof(deferred_handler_context_t));
    deferred_queue_item_t *item = NULL;
    
    if (get_plugin_option(options, "auth_url", handler_context->handler_url, URL_MAX_LENGTH)) {
        PLUGIN_ERROR("Can not read \"auth_url\" from options. Auth handler was't created.");
        free(handler_context);
        return NULL;
    }
    handler_context->envp = clone_string_array(envp);

    item = create_deferred_queue_item(
        &auth_user_pass_verify_handler_func,
        &default_handler_context_cleaner,
        handler_context);
    
    return item;
}

deferred_queue_item_t * create_client_connect_handler(
    const char *envp[], plugin_options_t *options)
{
    deferred_handler_context_t *handler_context = (deferred_handler_context_t*)malloc(
        sizeof(deferred_handler_context_t));
    deferred_queue_item_t *item = NULL;
    
    if (get_plugin_option(options, "connect_url", handler_context->handler_url, URL_MAX_LENGTH)) {
        PLUGIN_ERROR("Can not read \"connect_url\" from options. Connect handler was't created.");
        free(handler_context);
        return NULL;
    }
    handler_context->envp = clone_string_array(envp);

    item = create_deferred_queue_item(
        &notification_handler_func,
        &default_handler_context_cleaner,
        handler_context);
    
    return item;
}

deferred_queue_item_t * create_client_disconnect_handler(
    const char *envp[], plugin_options_t *options)
{
    deferred_handler_context_t *handler_context = (deferred_handler_context_t*)malloc(
        sizeof(deferred_handler_context_t));
    deferred_queue_item_t *item = NULL;
    
    if (get_plugin_option(options, "disconnect_url", handler_context->handler_url, URL_MAX_LENGTH)) {
        PLUGIN_ERROR("Can not read \"disconnect_url\" from options. Disconnect handler was't created.");
        free(handler_context);
        return NULL;
    }
    handler_context->envp = clone_string_array(envp);

    item = create_deferred_queue_item(
        &notification_handler_func,
        &default_handler_context_cleaner,
        handler_context);
    
    return item;;
}
