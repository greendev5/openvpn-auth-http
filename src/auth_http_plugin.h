/* openvpn-auth-http -- An OpenVPN plugin for do accounting.
 *
 * Copyright (C) ****** VPN Service 2014 <i.muzichuk@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef _AUTH_HTTP_PLUGIN_
#define _AUTH_HTTP_PLUGIN_

#include "openvpn-plugin.h"

struct auth_http_plugin_context;
typedef struct auth_http_plugin_context auth_http_plugin_context_t;

extern auth_http_plugin_context_t * create_auth_http_plugin_context(const char *path_to_opt_file);
extern void free_auth_http_plugin_context(auth_http_plugin_context_t *context);

#endif /* _AUTH_HTTP_PLUGIN_ */