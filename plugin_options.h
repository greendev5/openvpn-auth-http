/* openvpn-auth-http -- An OpenVPN plugin for do accounting.
 *
 * Copyright (C) ****** VPN Service 2014 <i.muzichuk@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef _PLUGIN_OPTIONS_
#define _PLUGIN_OPTIONS_

/**
 * @brief Options object.
 *
 * This struct is used to represent storage of plugin options.
 */
struct plugin_options;
typedef struct plugin_options plugin_options_t;

/**
 * @brief Allocate and create new options object.
 *
 * @param options_file_path Full path to file with plugin options.
 * @return New object or NULL if was error during reading options_file_path.
 */
extern plugin_options_t * open_plugin_options(const char *options_file_path);

/**
 * @brief Releases all memory held by a option object.
 *
 * @param opt Pointer to a valid options object.
 */
extern void free_plugin_options(plugin_options_t *opt);

/**
 * @brief Returns the value associated with the supplied key.
 *
 * @param opt Pointer to a valid options object.
 * @param key A pointer to a null-terminated C string.
 * @param out_buf A pointer to an output buffer which will contain the value.
 * @param out_buf_len The size of the output buffer in bytes.
 *
 * @return If out_buf is set to null and out_buf_len is set to 0 the return
 * value will be the number of bytes required to store the value (if it exists),
 * or -1 if value with this key not exists. For all other parameter configurations 
 * the return value is 0 if an associated value was found and completely copied 
 * into the output buffer, not zero value otherwise.
 */
extern int get_plugin_option(
    plugin_options_t *opt, const char *key, char *out_buf, unsigned int out_buf_len);

#endif /*_PLUGIN_OPTIONS_ */