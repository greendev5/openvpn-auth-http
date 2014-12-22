/* openvpn-auth-http -- An OpenVPN plugin for do accounting.
 *
 * Copyright (C) ****** VPN Service 2014 <i.muzichuk@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plugin_options.h"

#define MAX_OPT_COUNT   100

#define MAX_KEYCHARS    256
#define MAX_VALUECHARS  256
#define MAX_LINECHARS   (MAX_KEYCHARS + MAX_VALUECHARS - 10)

#define COMMENT_CHAR    '#'
#define KEY_VALUE_SEPARATOR_CHAR    ' '

typedef struct key_value_pair
{
    char key[MAX_KEYCHARS];
    char value[MAX_VALUECHARS];

    struct key_value_pair *next_pair;

} key_value_pair_t;

struct plugin_options
{
    key_value_pair_t *head_pair;
    size_t pair_count;
};

static int is_blank_char(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}


static int is_blank_str(const char *str)
{
    while (*str != '\0') {
        if (!is_blank_char(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

static void trim_str(char *out_str)
{
    static char tmp_buf[MAX_LINECHARS];
    memset(tmp_buf, 0, MAX_LINECHARS);
    memcpy(tmp_buf, out_str, strlen(out_str));

    char *str = tmp_buf;
    char *s0;
    size_t len = 0;

    /* Get position of the first not blank char. */
    while (*str != '\0' && is_blank_char(*str))
        str++;
    s0 = str;

    /* Get left size to the end of line. */
    while (*str != '\0') {
        len++;
        str++;
    }
    
    if (len > 0)
        str--;
    
    /* Get the last blank char. */
    while (is_blank_char(*str)) {
        str--;
        len--;
    }
    
    memset(out_str, 0, len);
    memcpy(out_str, s0, len);
    out_str[len] = '\0';
}

static void parse_line(const char *line, size_t len, char *key, char *value)
{
    size_t sep_pos = 0;
    size_t end_pos = len;
    size_t i;

    for (i = 0; i < len; i++) {
        
        if (line[i] == COMMENT_CHAR) {
            end_pos = i;
            break;
        } else if (line[i] == KEY_VALUE_SEPARATOR_CHAR) {
            if (!sep_pos)
                sep_pos = i;
        }
    }

    if (!sep_pos) {
        memcpy(key, line, end_pos);
    } else {
        memcpy(key, line, sep_pos);
        sep_pos++;
        memcpy(value, line + sep_pos, end_pos - sep_pos);
    }

    trim_str(key);
    trim_str(value);
}

extern plugin_options_t * open_plugin_options(const char *options_file_path)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    key_value_pair_t *pair = NULL;
    plugin_options_t *opt;

    FILE *fp = fopen(options_file_path, "r");
    if (!fp)
        return NULL;

    opt = (plugin_options_t*)malloc(sizeof(plugin_options_t));
    memset(opt, 0, sizeof(plugin_options_t));

    while ((read = getline(&line, &len, fp)) != -1) {
        
        if ((len == 0) || line[0] == COMMENT_CHAR)
            continue;
        if (is_blank_str(line))
            continue;
        if (len > MAX_LINECHARS)
            goto error;
        if (opt->pair_count > MAX_OPT_COUNT)
            goto error;
        
        pair = (key_value_pair_t*)malloc(sizeof(key_value_pair_t));
        memset(pair, 0, sizeof(key_value_pair_t));
        parse_line(line, len, pair->key, pair->value);
        pair->next_pair = opt->head_pair;
        opt->head_pair = pair;
        opt->pair_count++;
    }
    
    free(line);
    fclose(fp);
    return opt;

error:
    if (line)
        free(line);
    fclose(fp);
    free_plugin_options(opt);
    return NULL;
}

extern void free_plugin_options(plugin_options_t *opt)
{
    key_value_pair_t *pair;
    while (opt->head_pair) {
        pair = opt->head_pair;
        opt->head_pair = opt->head_pair->next_pair;
        free(pair);
    }
    free(opt);
}

extern int get_plugin_option(
    plugin_options_t *opt, const char *key, char *out_buf, unsigned int out_buf_len)
{
    unsigned char search = !out_buf && !out_buf_len ? 1 : 0;
    int r = -1;
    int value_len = 0;

    key_value_pair_t *pair = opt->head_pair;
    while (pair) {
        if (!strcmp(key, pair->key)) {
            
            value_len = strlen(pair->value);
            if (search) {
                return value_len;
            } else {
                if (value_len < out_buf_len) {
                    memcpy(out_buf, pair->value, value_len);
                    out_buf[value_len] = '\0';
                    return 0;
                }
                return -1;
            }

        }
        pair = pair->next_pair;
    }

    return r;
}

void some_test()
{
    key_value_pair_t *pair = (key_value_pair_t*)malloc(sizeof(key_value_pair_t));
    memset(pair, 0, sizeof(key_value_pair_t));

    const char *line = "f1 http://sfsdfsdfds fsdfsdf\t\t\t\t\n #2value";
    parse_line(line, strlen(line), pair->key, pair->value);

    printf(":::%s:::%s:::\n", pair->key, pair->value);
}
