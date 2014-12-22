/* openvpn-auth-http -- An OpenVPN plugin for do accounting.
 *
 * Copyright (C) ****** VPN Service 2014 <i.muzichuk@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "plugin_utils.h"

/* Some tricks for thread safe logging */
 typedef struct __plugin_logging__ {
    int verbosity;
    pthread_mutex_t mutex;
 } __plugin_logging_t__;

static __plugin_logging_t__ __plugin_loging_data__;

int __lock_plugin_logging__(void)
{
    pthread_mutex_lock(&__plugin_loging_data__.mutex);
    return __plugin_loging_data__.verbosity;
}

void __unlock_plugin_logging__(void)
{
    pthread_mutex_unlock(&__plugin_loging_data__.mutex);
}

void init_plugin_logging(int verbosity)
{
    __plugin_loging_data__.verbosity = verbosity;
    pthread_mutex_init(&__plugin_loging_data__.mutex, NULL);
}

void clear_plugin_logging(void)
{
    pthread_mutex_destroy(&__plugin_loging_data__.mutex);
}
/****************************/

size_t string_array_len(const char *array[])
{
    size_t i = 0;
    if (array) {
        while (array[i])
            ++i;
    }
    return i;
}

char ** clone_string_array(const char *array[])
{
    size_t i;
    size_t array_len = string_array_len(array);
    size_t string_len = 0;
    char **new_array = (char**)malloc(sizeof(char*) * (array_len + 1));
    new_array[array_len] = NULL;
    
    for (i = 0; i < array_len; i++) {
        string_len = strlen(array[i]);
        new_array[i] = (char*)malloc(sizeof(char) * (string_len + 1));
        memcpy(new_array[i], array[i], string_len);
        new_array[i][string_len] = '\0'; 
    }

    return new_array;
}

void free_cloned_string_array(char *array[])
{
    size_t i;
    for (i = 0; array[i]; i++)
        free(array[i]);
    free(array);
}

const char * get_openvpn_env(const char *name, const char *envp[])
{
    size_t i;
    const size_t namelen = strlen(name);
    const char *cp = NULL;

    if (envp) {
        
        for (i = 0; envp[i]; i++) {
            if(!strncmp(envp[i], name, namelen)) {
                cp = envp[i] + namelen;
                if ( *cp == '=' )
                    return cp + 1;
            }
        }
    }
    return NULL;
}

static unsigned char is_in_list(const char *value, size_t value_len, const char *exclude[])
{
    size_t i;
    if (!exclude)
        return 0;
    for (i = 0; exclude[i]; i++) {
        if (!strncmp(value, exclude[i], value_len))
            return 1;
    }
    return 0;
}

char * openvpn_envp_to_json(const char *envp[], const char *exclude[])
{
    size_t i, j;
    size_t buffer_size = 0;
    size_t buffer_pos = 0;
    const size_t str_space = 15;
    char *json_str;
    const char *value_pos;

    if (!envp)
        return NULL;

    for (i = 0; envp[i]; i++)
        buffer_size += strlen(envp[i]) + str_space;

    json_str = (char*)malloc(sizeof(char) * buffer_size);
    memset(json_str, 0, buffer_size);

    json_str[buffer_pos] = '{'; buffer_pos++;
    for (i = 0; envp[i]; i++) {

        value_pos = NULL;
        for (j = 0; envp[i][j]; j++) {
            if (envp[i][j] == '=') {
                value_pos = envp[i] + j + 1;
                break;
            }
        }
        if (!value_pos)
            continue;
        if (is_in_list(envp[i], j, exclude))
            continue;

        /* Add ", " to string, if we are adding first element */
        if (buffer_pos != 1) {
            json_str[buffer_pos] = ','; buffer_pos++;
            json_str[buffer_pos] = ' '; buffer_pos++;
        }
        
        json_str[buffer_pos] = '\"'; buffer_pos++;
        
        memcpy(json_str + buffer_pos, envp[i], j); 
        buffer_pos += j;
        
        json_str[buffer_pos] = '\"'; buffer_pos++;
        json_str[buffer_pos] = ':'; buffer_pos++;
        json_str[buffer_pos] = '\"'; buffer_pos++;
        
        memcpy(json_str + buffer_pos, value_pos, strlen(value_pos)); 
        buffer_pos += strlen(value_pos);
        
        json_str[buffer_pos] = '\"'; buffer_pos++;

    }
    json_str[buffer_pos] = '}'; buffer_pos++;

    return json_str;
}
