/* openvpn-auth-http -- An OpenVPN plugin for do accounting.
 *
 * Copyright (C) ****** VPN Service 2014 <i.muzichuk@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef _PLUGIN_UTILS_
#define _PLUGIN_UTILS_

#include <stddef.h>
#include <stdio.h>

#define PLUGIN_LOG(format, ...) {                                                     \
    __lock_plugin_logging__();                                                        \
    fprintf(stderr, "OPENVPN_HTTP_AUTH_PLUGIN: [INFO]: " format "\n", ##__VA_ARGS__); \
    __unlock_plugin_logging__();                                                      \
}

#define PLUGIN_ERROR(format, ...) {                                                    \
    __lock_plugin_logging__();                                                         \
    fprintf(stderr, "OPENVPN_HTTP_AUTH_PLUGIN: [ERROR]: " format "\n", ##__VA_ARGS__); \
    __unlock_plugin_logging__();                                                       \
}

#define PLUGIN_DEBUG(format, ...) {                                                        \
    int verb = __lock_plugin_logging__();                                                  \
    if (verb > 3)                                                                          \
        fprintf(stderr, "OPENVPN_HTTP_AUTH_PLUGIN: [DEBUG]: " format "\n", ##__VA_ARGS__); \
    __unlock_plugin_logging__();                                                           \
}

/** 
 * @brief Return the length of a string array.
 *
 * @param Array
 * @return The length of the array.
 */
size_t string_array_len(const char *array[]);

/**
 *
 */
char ** clone_string_array(const char *array[]);

/**
 *
 */
void free_cloned_string_array(char *array[]);

/** @brief Search for env variable in openvpn format.
 *
 * Given an environmental variable name, search
 * the envp array for its value, returning it
 * if found or NULL otherwise.
 * A field in the envp-array looks like: name=user1
 *
 * @param The name of the variable.
 * @param The array with the enviromental variables.
 * @return A poniter to the variable value or NULL, if the varaible was not found.
 */
const char * get_openvpn_env(const char *name, const char *envp[]);

/**
 * Copy openvpn envp like array to json string.
 * @param envp The array with the enviromental variables, NULL term.
 * @param exclude The array with name if vars for exclude from json, NULL term.
 * @return Json representation string. I had to clear mem ofthis str with free.
 */
char * openvpn_envp_to_json(const char *envp[], const char *exclude[]);

/**
 *
 */
void init_plugin_logging(int verbosity);

/**
 *
 */
void clear_plugin_logging(void);

/**
 */
int __lock_plugin_logging__(void);
void __unlock_plugin_logging__(void);

#endif /* _PLUGIN_UTILS_ */