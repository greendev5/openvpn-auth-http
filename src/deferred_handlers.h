/* openvpn-auth-http -- An OpenVPN plugin for do accounting.
 *
 * Copyright (C) ****** VPN Service 2014 <i.muzichuk@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef _DEFERRED_HANDLERS_
#define _DEFERRED_HANDLERS_

#include "plugin_utils.h"
#include "plugin_options.h"
#include "deferred_queue.h"

#define URL_MAX_LENGTH 256

typedef struct deferred_handler_context
{
    char **envp;
    char handler_url[URL_MAX_LENGTH];

} deferred_handler_context_t;

deferred_queue_item_t * create_auth_user_pass_verify_handler(
    const char *envp[], plugin_options_t *options);

deferred_queue_item_t * create_client_connect_handler(
    const char *envp[], plugin_options_t *options);

deferred_queue_item_t * create_client_disconnect_handler(
    const char *envp[], plugin_options_t *options);

#endif /* _DEFERRED_HANDLERS_ */