/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Wang Jian
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <libgen.h>

#include "st_log.h"
#include "st_io.h"
#include "st_string.h"
#include "st_conf.h"

#define SEC_NUM     10
#define PARAM_NUM    100

int st_conf_strcmp(const char *s1, const char *s2)
{
    int c1, c2;

    if (s1 == NULL && s2 != NULL) {
        return -1;
    } else if (s1 != NULL && s2 == NULL) {
        return 1;
    } else if (s1 == NULL && s2 == NULL) {
        return 0;
    }

    while (1) {
        c1 = tolower((unsigned char) *s1++);
        if (c1 == '-') {
            c1 = '_';
        }
        c2 = tolower((unsigned char) *s2++);
        if (c2 == '-') {
            c2 = '_';
        }
        if (c1 == 0 || c1 != c2) {
            return c1 - c2;
        }
    }
}

static int resize_sec(st_conf_section_t *sec)
{
    if (sec->param_num >= sec->param_cap) {
        sec->param_cap += PARAM_NUM;
        sec->param = (st_conf_param_t *)st_realloc(sec->param,
                    sec->param_cap * sizeof(st_conf_param_t));
        if (sec->param == NULL) {
            ST_ERROR("Failed to st_realloc param for sec.");
            goto ERR;
        }
        memset(sec->param + sec->param_num, 0,
                PARAM_NUM * sizeof(st_conf_param_t));
    }

    if (sec->def_param_num >= sec->def_param_cap) {
        sec->def_param_cap += PARAM_NUM;
        sec->def_param = (st_conf_param_t *)st_realloc(sec->def_param,
                    sec->def_param_cap * sizeof(st_conf_param_t));
        if (sec->def_param == NULL) {
            ST_ERROR("Failed to st_realloc def_param for sec.");
            goto ERR;
        }
        memset(sec->def_param + sec->def_param_num, 0,
                PARAM_NUM * sizeof(st_conf_param_t));
    }

    return 0;
ERR:
    return -1;
}

st_conf_section_t* st_conf_new_sec(st_conf_t *conf, const char *name)
{
    int s;

    for (s = 0; s < conf->sec_num; s++) {
        if (st_conf_strcmp(conf->secs[s].name, name) == 0) {
            return conf->secs + s;
        }
    }

    if (conf->sec_num >= conf->sec_cap) {
        conf->sec_cap += SEC_NUM;
        conf->secs = (st_conf_section_t *)st_realloc(conf->secs,
                    conf->sec_cap * sizeof(st_conf_section_t));
        if (conf->secs == NULL) {
            ST_ERROR("Failed to st_realloc secs.");
            goto ERR;
        }
        memset(conf->secs + conf->sec_num, 0,
                SEC_NUM * sizeof(st_conf_section_t));
    }

    strncpy(conf->secs[conf->sec_num].name, name, MAX_ST_CONF_LEN);
    if (resize_sec(conf->secs + conf->sec_num) < 0) {
        ST_ERROR("Failed to resize_sec.");
        goto ERR;
    }

    conf->sec_num++;
    return conf->secs + (conf->sec_num - 1);

ERR:
    return NULL;
}

int st_conf_add_param(st_conf_section_t *sec, const char *key,
        const char *value)
{
    st_conf_param_t *param;
    int i;

    param = NULL;
    for (i = 0; i < sec->param_num; i++) {
        if (st_conf_strcmp(sec->param[i].key, key) == 0) {
            param = sec->param + i;
            break;
        }
    }

    if (param == NULL) {
        param = sec->param + sec->param_num;

        sec->param_num++;
        if (resize_sec(sec) < 0) {
            ST_ERROR("Failed to resize_sec.");
            return -1;
        }
    }

    param->used = 0;
    strncpy(param->key, key, MAX_ST_CONF_LEN);
    param->key[MAX_ST_CONF_LEN - 1] = 0;
    strncpy(param->value, value, MAX_ST_CONF_LEN);
    param->value[MAX_ST_CONF_LEN - 1] = 0;

    return 0;
}

st_conf_section_t* st_conf_get_sec(st_conf_t *conf, const char* sec_name)
{
    int s;

    for (s = 0; s < conf->sec_num; s++) {
        if (st_conf_strcmp(conf->secs[s].name, sec_name) == 0) {
            return conf->secs + s;
        }
    }

    return NULL;
}

st_conf_section_t* st_conf_def_sec(st_conf_t *conf)
{
    return st_conf_get_sec(conf, DEF_SEC_NAME);
}

st_conf_t* st_conf_create()
{
    st_conf_t *pconf = NULL;

    pconf = (st_conf_t *)st_malloc(sizeof(st_conf_t));
    if (pconf == NULL) {
        ST_ERROR("Failed to st_malloc st_conf.");
        goto ERR;
    }
    memset(pconf, 0, sizeof(st_conf_t));

    if (st_conf_new_sec(pconf, DEF_SEC_NAME) == NULL) {
        ST_ERROR("Failed to st_conf_new_sec.");
        goto ERR;
    }

    return pconf;

ERR:
    safe_st_conf_destroy(pconf);
    return NULL;
}

typedef struct _st_conf_line_type_t_ {
    enum {
        IGNORE = 0,
        SECTION,
        PARAM,
    } type;
    char key[MAX_ST_CONF_LEN];
    char value[MAX_ST_CONF_LEN];
} st_conf_line_type_t;

static void st_conf_line_type_clear(st_conf_line_type_t *line_type)
{
    ST_CHECK_PARAM_VOID(line_type == NULL);

    line_type->type = IGNORE;
    line_type->key[0] = '\0';
    line_type->value[0] = '\0';
}

static int st_resolve_line(const char *line, st_conf_line_type_t *line_type)
{
    char buffer[MAX_ST_CONF_LINE_LEN];
    char *work = NULL;
    char *sec_conf = NULL;
    int i;
    int j;
    int len;

    ST_CHECK_PARAM(line == NULL || line_type == NULL, -1);

    st_conf_line_type_clear(line_type);

    work = strrchr(line, '\r');
    if (work != NULL) {
        *work = '\0';
    }
    work = strrchr(line, '\n');
    if (work != NULL) {
        *work = '\0';
    }

    len = (int) strlen(line);
    j = 0;

    for (i = 0; i < len; i++) {
        if (line[i] != ' ' && line[i] != '\t') {
            buffer[j] = line[i];
            j++;
        }
        if (j >= MAX_ST_CONF_LINE_LEN) {
            ST_ERROR("Too long line[%s]", line);
            return -1;
        }
    }
    len = j;
    if (len <= 0) { // ignore empty line
        return 0;
    }
    buffer[len] = '\0';

    if (buffer[0] == '#') { // ignore comment line
        return 0;
    } else if (buffer[0] == '[') {
        work = strrchr(buffer, ']');
        if (work != NULL) {
            *work = '\0';
        } else {
            ST_ERROR("Not closed '['.");
            return -1;
        }

        line_type->type = SECTION;
        work = buffer + 1;

        sec_conf = strchr(work, ':');
        if (sec_conf != NULL) {
            *sec_conf = '\0';
            sec_conf++;
            if (*sec_conf == '\0') {
                ST_ERROR("No section config file provided[%s]", line);
                return -1;
            }

            // section config file
            strncpy(line_type->value, sec_conf, MAX_ST_CONF_LEN);
            line_type->value[MAX_ST_CONF_LEN - 1] = '\0';
        }

        // section name
        strncpy(line_type->key, work, MAX_ST_CONF_LEN);
        line_type->key[MAX_ST_CONF_LEN - 1] = '\0';

        return 0;
    }

    work = strchr(buffer, ':');
    if (work == NULL) {
        ST_ERROR("Invalid line[%s]", line);
        return -1;
    }
    *work = 0;
    work++;

    line_type->type = PARAM;
    strncpy(line_type->key, buffer, MAX_ST_CONF_LEN);
    line_type->key[MAX_ST_CONF_LEN - 1] = '\0';
    strncpy(line_type->value, work, MAX_ST_CONF_LEN);
    line_type->value[MAX_ST_CONF_LEN - 1] = '\0';

    return 0;
}

static int st_conf_load_one_file(st_conf_t *st_conf,
        st_conf_section_t *parent_sec, const char *conf_file)
{
    char line[MAX_ST_CONF_LINE_LEN];
    char sec_name[MAX_ST_CONF_LEN];
    char conf_dirname[MAX_ST_CONF_LEN];
    char sec_conf[MAX_ST_CONF_LEN];
    st_conf_line_type_t line_type;
    st_conf_section_t *cur_sec = NULL;
    FILE *cur_fp = NULL;

    ST_CHECK_PARAM(st_conf == NULL || conf_file == NULL, -1);

    if (parent_sec == NULL) {
        cur_sec = st_conf_def_sec(st_conf);
        if (cur_sec == NULL) {
            ST_ERROR("Error: No default section.");
            goto ERR;
        }
    } else {
        cur_sec = parent_sec;
    }

    cur_fp = st_fopen(conf_file, "rb");
    if (cur_fp == NULL) {
        ST_ERROR("Failed to st_fopen[%s]", conf_file);
        goto ERR;
    }

    while (fgets(line, MAX_ST_CONF_LINE_LEN, cur_fp)) {
        if (st_resolve_line(line, &line_type) < 0) {
            ST_ERROR("Failed to st_resolve_line.");
            goto ERR;
        }

        switch (line_type.type) {
            case IGNORE:
                break;
            case SECTION:
                if (parent_sec != NULL) {
                    snprintf(sec_name, MAX_ST_CONF_LEN, "%s/%s",
                            parent_sec->name, line_type.key);
                } else {
                    snprintf(sec_name, MAX_ST_CONF_LEN, "%s", line_type.key);
                }
                cur_sec = st_conf_new_sec(st_conf, sec_name);
                if (cur_sec == NULL) {
                    ST_ERROR("Failed to st_conf_new_sec[%s].", sec_name);
                    goto ERR;
                }

                if (line_type.value[0] != '\0') {
                    strncpy(conf_dirname, conf_file, MAX_ST_CONF_LEN);
                    conf_dirname[MAX_ST_CONF_LEN - 1] = '\0';
                    if (st_str_replace(sec_conf, MAX_ST_CONF_LEN,
                            line_type.value, ORIGIN_PATH_VAR,
                            dirname(conf_dirname), -1) < 0) {
                        ST_ERROR("Failed to st_str_replace[%s] [%s -> %s].",
                                line_type.value, ORIGIN_PATH_VAR,
                                dirname(conf_dirname));
                        goto ERR;
                    }

                    if (st_conf_load_one_file(st_conf, cur_sec, sec_conf) < 0) {
                        ST_ERROR("Failed to st_conf_load_one_file[%s] for section[%s].",
                                sec_conf, sec_name);
                        goto ERR;
                    }
                }
                break;
            case PARAM:
                if (st_conf_add_param(cur_sec, line_type.key, line_type.value) < 0) {
                    ST_ERROR("Failed to st_conf_add_param. "
                             "sec[%s], key[%s], value[%s]", cur_sec->name,
                             line_type.key, line_type.value);
                    return -1;
                }
                break;
            default:
                ST_ERROR("Unknown line_type[%d]", line_type.type);
                goto ERR;
        }
    }

    safe_st_fclose(cur_fp);

    return 0;

ERR:
    safe_st_fclose(cur_fp);
    return -1;
}

int st_conf_load(st_conf_t *st_conf, const char *conf_file)
{
    ST_CHECK_PARAM(st_conf == NULL || conf_file == NULL, -1);

    if (st_conf_load_one_file(st_conf, NULL, conf_file) < 0) {
        ST_ERROR("Failed to st_conf_load_one_file[%s].", conf_file);
        return -1;
    }

    return 0;
}

int st_conf_get_str(st_conf_t *pconf, const char *sec_name,
        const char *key, char *value, int vlen, int *sec_i)
{
    char name[MAX_ST_CONF_LEN];
    int s;
    int p;

    if (pconf == NULL || key == NULL || value == NULL || vlen <= 0) {
        if (sec_i != NULL) {
            *sec_i = -1;
        }
        return -1;
    }

    if (sec_name == NULL || sec_name[0] == '\0') {
        strncpy(name, DEF_SEC_NAME, MAX_ST_CONF_LEN);
    } else {
        strncpy(name, sec_name, MAX_ST_CONF_LEN);
    }
    name[MAX_ST_CONF_LEN - 1] = '\0';

    for (s = 0; s < pconf->sec_num; s++) {
        if (st_conf_strcmp(pconf->secs[s].name, name) == 0) {
            for (p = 0; p < pconf->secs[s].param_num; p++) {
                if (st_conf_strcmp(pconf->secs[s].param[p].key, key) == 0) {
                    pconf->secs[s].param[p].used = 1;
                    strncpy(value, pconf->secs[s].param[p].value, vlen);
                    value[vlen - 1] = 0;
                    break;
                }
            }

            if (p >= pconf->secs[s].param_num) {
                if (sec_i != NULL) {
                    *sec_i = s;
                }
                return -1;
            }

            break;
        }
    }

    if (s >= pconf->sec_num) {
        if (sec_i != NULL) {
            *sec_i = -1;
        }
        return -1;
    }

    if (sec_i != NULL) {
        *sec_i = s;
    }
    return 0;
}

int st_conf_get_str_def(st_conf_t *pconf, const char *sec_name,
        const char *key, char *value, int vlen, const char *default_value)
{
    st_conf_section_t *sec;
    int sec_i = -1;

    if (st_conf_get_str(pconf, sec_name, key, value, vlen, &sec_i) < 0) {
        strncpy(value, default_value, vlen);
        value[vlen - 1] = 0;

        if (sec_i < 0) {
            sec = st_conf_new_sec(pconf, sec_name);
            if (sec == NULL) {
                ST_ERROR("Failed to st_conf_new_sec.");
                return -1;
            }
        } else {
            sec = pconf->secs + sec_i;
        }

        strncpy(sec->def_param[sec->def_param_num].key, key,
                MAX_ST_CONF_LEN);
        sec->def_param[sec->def_param_num].key[MAX_ST_CONF_LEN - 1] = 0;
        strncpy(sec->def_param[sec->def_param_num].value, default_value,
                MAX_ST_CONF_LEN);
        sec->def_param[sec->def_param_num].value[MAX_ST_CONF_LEN - 1] = 0;
        sec->def_param_num++;

        if (resize_sec(sec) < 0) {
            ST_ERROR("Failed to resize_sec.");
            return -1;
        }
    }

    return 0;
}

int st_conf_get_bool(st_conf_t *pconf, const char *sec_name,
        const char *key, bool *value, int *sec_i)
{
    char v[MAX_ST_CONF_LINE_LEN];

    if (st_conf_get_str(pconf, sec_name, key, v,
                MAX_ST_CONF_LINE_LEN, sec_i) < 0) {
        return -1;
    }

    if (v[0] == '\0' // for no argument options, e.g. --help
            || strncmp(v, "1", 2) == 0
            || strncasecmp(v, "T", 2) == 0
            || strncasecmp(v, "TRUE", 5) == 0) {
        (*value) = true;
    } else if (strncmp(v, "0", 2) == 0
            || strncasecmp(v, "F", 2) == 0
            || strncasecmp(v, "FALSE", 6) == 0) {
        (*value) = false;
    } else {
        ST_ERROR("Unkown bool value[%s], should be \"True\" or \"False\".",
                v);
        return -1;
    }

    return 0;
}

int st_conf_get_bool_def(st_conf_t *pconf, const char *sec_name,
        const char *key, bool *value, bool default_value)
{
    st_conf_section_t *sec;
    int sec_i = -1;

    if (st_conf_get_bool(pconf, sec_name, key, value, &sec_i) < 0) {
        *value = default_value;

        if (sec_i < 0) {
            sec = st_conf_new_sec(pconf, sec_name);
            if (sec == NULL) {
                ST_ERROR("Failed to st_conf_new_sec.");
                return -1;
            }
        } else {
            sec = pconf->secs + sec_i;
        }

        strncpy(sec->def_param[sec->def_param_num].key, key,
                MAX_ST_CONF_LEN);
        sec->def_param[sec->def_param_num].key[MAX_ST_CONF_LEN - 1] = 0;
        if (default_value) {
            snprintf(sec->def_param[sec->def_param_num].value,
                    MAX_ST_CONF_LEN, "%s", "True");
        } else {
            snprintf(sec->def_param[sec->def_param_num].value,
                    MAX_ST_CONF_LEN, "%s", "False");
        }
        sec->def_param[sec->def_param_num].value[MAX_ST_CONF_LEN - 1] = 0;
        sec->def_param_num++;

        if (resize_sec(sec) < 0) {
            ST_ERROR("Failed to resize_sec.");
            return -1;
        }
    }
    return 0;
}

int st_conf_get_int(st_conf_t *pconf, const char *sec_name,
        const char *key, int *value, int *sec_i)
{
    char v[MAX_ST_CONF_LINE_LEN];

    if (st_conf_get_str(pconf, sec_name, key, v,
                MAX_ST_CONF_LINE_LEN, sec_i) < 0) {
        return -1;
    }

    if (v[0] == '\0') {
        if (sec_name == NULL || sec_name[0] == '\0') {
            ST_ERROR("Int option[%s] should have arguement", key);
        } else {
            ST_ERROR("Int option[%s.%s] should have arguement",
                    sec_name, key);
        }

        return -1;
    }

    (*value) = atoi(v);
    return 0;
}

int st_conf_get_int_def(st_conf_t *pconf, const char *sec_name,
        const char *key, int *value, int default_value)
{
    st_conf_section_t *sec;
    int sec_i = -1;

    if (st_conf_get_int(pconf, sec_name, key, value, &sec_i) < 0) {
        *value = default_value;

        if (sec_i < 0) {
            sec = st_conf_new_sec(pconf, sec_name);
            if (sec == NULL) {
                ST_ERROR("Failed to st_conf_new_sec.");
                return -1;
            }
        } else {
            sec = pconf->secs + sec_i;
        }

        strncpy(sec->def_param[sec->def_param_num].key, key,
                MAX_ST_CONF_LEN);
        sec->def_param[sec->def_param_num].key[MAX_ST_CONF_LEN - 1] = 0;
        snprintf(sec->def_param[sec->def_param_num].value,
                 MAX_ST_CONF_LEN, "%d", default_value);
        sec->def_param[sec->def_param_num].value[MAX_ST_CONF_LEN - 1] = 0;
        sec->def_param_num++;

        if (resize_sec(sec) < 0) {
            ST_ERROR("Failed to resize_sec.");
            return -1;
        }
    }
    return 0;
}

int st_conf_get_uint(st_conf_t *pconf, const char *sec_name,
        const char *key, unsigned int *value, int *sec_i)
{
    char v[MAX_ST_CONF_LINE_LEN];

    if (st_conf_get_str(pconf, sec_name, key, v,
                MAX_ST_CONF_LINE_LEN, sec_i) < 0) {
        return -1;
    }

    if (v[0] == '\0') {
        if (sec_name == NULL || sec_name[0] == '\0') {
            ST_ERROR("Uint option[%s] should have arguement", key);
        } else {
            ST_ERROR("Uint option[%s.%s] should have arguement",
                    sec_name, key);
        }

        return -1;
    }

    sscanf(v, "%u", value);
    return 0;
}

int st_conf_get_uint_def(st_conf_t *pconf, const char *sec_name,
        const char *key, unsigned int *value, unsigned int default_value)
{
    st_conf_section_t *sec;
    int sec_i = -1;

    if (st_conf_get_uint(pconf, sec_name, key, value, &sec_i) < 0) {
        *value = default_value;

        if (sec_i < 0) {
            sec = st_conf_new_sec(pconf, sec_name);
            if (sec == NULL) {
                ST_ERROR("Failed to st_conf_new_sec.");
                return -1;
            }
        } else {
            sec = pconf->secs + sec_i;
        }
        strncpy(sec->def_param[sec->def_param_num].key, key,
                MAX_ST_CONF_LEN);
        sec->def_param[sec->def_param_num].key[MAX_ST_CONF_LEN - 1] = 0;
        snprintf(sec->def_param[sec->def_param_num].value,
                 MAX_ST_CONF_LEN, "%u", default_value);
        sec->def_param[sec->def_param_num].value[MAX_ST_CONF_LEN - 1] = 0;
        sec->def_param_num++;

        if (resize_sec(sec) < 0) {
            ST_ERROR("Failed to resize_sec.");
            return -1;
        }
    }
    return 0;
}

int st_conf_get_long(st_conf_t *pconf, const char *sec_name,
        const char *key, long *value, int *sec_i)
{
    char v[MAX_ST_CONF_LINE_LEN];

    if (st_conf_get_str(pconf, sec_name, key, v,
                MAX_ST_CONF_LINE_LEN, sec_i) < 0) {
        return -1;
    }

    if (v[0] == '\0') {
        if (sec_name == NULL || sec_name[0] == '\0') {
            ST_ERROR("Long option[%s] should have arguement", key);
        } else {
            ST_ERROR("Long option[%s.%s] should have arguement",
                    sec_name, key);
        }

        return -1;
    }

    (*value) = atoi(v);
    return 0;
}

int st_conf_get_long_def(st_conf_t *pconf, const char *sec_name,
        const char *key, long *value, long default_value)
{
    st_conf_section_t *sec;
    int sec_i = -1;

    if (st_conf_get_long(pconf, sec_name, key, value, &sec_i) < 0) {
        *value = default_value;

        if (sec_i < 0) {
            sec = st_conf_new_sec(pconf, sec_name);
            if (sec == NULL) {
                ST_ERROR("Failed to st_conf_new_sec.");
                return -1;
            }
        } else {
            sec = pconf->secs + sec_i;
        }

        strncpy(sec->def_param[sec->def_param_num].key, key,
                MAX_ST_CONF_LEN);
        sec->def_param[sec->def_param_num].key[MAX_ST_CONF_LEN - 1] = 0;
        snprintf(sec->def_param[sec->def_param_num].value,
                 MAX_ST_CONF_LEN, "%ld", default_value);
        sec->def_param[sec->def_param_num].value[MAX_ST_CONF_LEN - 1] = 0;
        sec->def_param_num++;

        if (resize_sec(sec) < 0) {
            ST_ERROR("Failed to resize_sec.");
            return -1;
        }
    }
    return 0;
}

int st_conf_get_ulong(st_conf_t *pconf, const char *sec_name,
        const char *key, unsigned long *value, int *sec_i)
{
    char v[MAX_ST_CONF_LINE_LEN];

    if (st_conf_get_str(pconf, sec_name, key, v,
                MAX_ST_CONF_LINE_LEN, sec_i) < 0) {
        return -1;
    }

    if (v[0] == '\0') {
        if (sec_name == NULL || sec_name[0] == '\0') {
            ST_ERROR("Ulong option[%s] should have arguement", key);
        } else {
            ST_ERROR("Ulong option[%s.%s] should have arguement",
                    sec_name, key);
        }

        return -1;
    }

    sscanf(v, "%lu", value);
    return 0;
}

int st_conf_get_ulong_def(st_conf_t *pconf, const char *sec_name,
        const char *key, unsigned long *value, unsigned long default_value)
{
    st_conf_section_t *sec;
    int sec_i = -1;

    if (st_conf_get_ulong(pconf, sec_name, key, value, &sec_i) < 0) {
        *value = default_value;

        if (sec_i < 0) {
            sec = st_conf_new_sec(pconf, sec_name);
            if (sec == NULL) {
                ST_ERROR("Failed to st_conf_new_sec.");
                return -1;
            }
        } else {
            sec = pconf->secs + sec_i;
        }
        strncpy(sec->def_param[sec->def_param_num].key, key,
                MAX_ST_CONF_LEN);
        sec->def_param[sec->def_param_num].key[MAX_ST_CONF_LEN - 1] = 0;
        snprintf(sec->def_param[sec->def_param_num].value,
                 MAX_ST_CONF_LEN, "%lu", default_value);
        sec->def_param[sec->def_param_num].value[MAX_ST_CONF_LEN - 1] = 0;
        sec->def_param_num++;

        if (resize_sec(sec) < 0) {
            ST_ERROR("Failed to resize_sec.");
            return -1;
        }
    }
    return 0;
}

int st_conf_get_double(st_conf_t *pconf, const char *sec_name,
        const char *key, double *value, int *sec_i)
{
    char v[MAX_ST_CONF_LINE_LEN];

    if (st_conf_get_str(pconf, sec_name, key, v,
                MAX_ST_CONF_LINE_LEN, sec_i) < 0) {
        return -1;
    }

    if (v[0] == '\0') {
        if (sec_name == NULL || sec_name[0] == '\0') {
            ST_ERROR("Float option[%s] should have arguement", key);
        } else {
            ST_ERROR("Float option[%s.%s] should have arguement",
                    sec_name, key);
        }

        return -1;
    }

    (*value) = atof(v);
    return 0;
}

int st_conf_get_double_def(st_conf_t *pconf, const char *sec_name,
        const char *key, double *value, double default_value)
{
    st_conf_section_t *sec;
    int sec_i = -1;

    if (st_conf_get_double(pconf, sec_name, key, value, &sec_i) < 0) {
        *value = default_value;

        if (sec_i < 0) {
            sec = st_conf_new_sec(pconf, sec_name);
            if (sec == NULL) {
                ST_ERROR("Failed to st_conf_new_sec.");
                return -1;
            }
        } else {
            sec = pconf->secs + sec_i;
        }
        strncpy(sec->def_param[sec->def_param_num].key, key,
                MAX_ST_CONF_LEN);
        sec->def_param[sec->def_param_num].key[MAX_ST_CONF_LEN - 1] = 0;
        snprintf(sec->def_param[sec->def_param_num].value,
                 MAX_ST_CONF_LEN, "%g", default_value);
        sec->def_param[sec->def_param_num].value[MAX_ST_CONF_LEN - 1] = 0;
        sec->def_param_num++;
        if (resize_sec(sec) < 0) {
            ST_ERROR("Failed to resize_sec.");
            return -1;
        }
    }
    return 0;
}

void st_conf_destroy(st_conf_t * pconf)
{
    int i;
    if (pconf != NULL) {
        for (i = 0; i < pconf->sec_num; i++) {
            safe_st_free(pconf->secs[i].param);
            pconf->secs[i].param_cap = 0;
            pconf->secs[i].param_num = 0;

            safe_st_free(pconf->secs[i].def_param);
            pconf->secs[i].def_param_cap = 0;
            pconf->secs[i].def_param_num = 0;
        }
        safe_st_free(pconf->secs);
        pconf->sec_cap = 0;
        pconf->sec_num = 0;
    }
}

static char* st_conf_normalize_key(char *key, bool forprint)
{
    char *p;

    p = key;
    while (*p) {
        if (*p == '-') {
            *p = '_';
        } else {
            *p = toupper(*p);
        }
        p++;
    }

    return key;
}

void st_conf_show(st_conf_t *pconf, const char *header)
{
    int s;
    int p;

    if (pconf == NULL) {
        return;
    }
    if (header != NULL) {
        ST_CLEAN("%s", header);
    }
    for (s = 0; s < pconf->sec_num; s++) {
        ST_CLEAN("[%s]", st_conf_normalize_key(pconf->secs[s].name, true));
        for (p = 0; p < pconf->secs[s].param_num; p++) {
            ST_CLEAN("%s{%s : %s}", pconf->secs[s].param[p].used ? "" : "*",
                    st_conf_normalize_key(pconf->secs[s].param[p].key, true),
                    pconf->secs[s].param[p].value);
        }
        for (p = 0; p < pconf->secs[s].def_param_num; p++) {
            ST_CLEAN("{%s : %s}#",
                    st_conf_normalize_key(pconf->secs[s].def_param[p].key, true),
                    pconf->secs[s].def_param[p].value);
        }
        ST_CLEAN("");
    }
}

bool st_conf_check(st_conf_t *pconf,
        char* (*norm_key_func)(char *, bool forprint))
{
    int s;
    int p;

    char sec_sep;
    bool ret = true;

    if (pconf == NULL) {
        return true;
    }

    if (norm_key_func == NULL) {
        norm_key_func = st_conf_normalize_key;
        sec_sep = '/';
    } else {
        sec_sep = '.';
    }

    for (s = 0; s < pconf->sec_num; s++) {
        for (p = 0; p < pconf->secs[s].param_num; p++) {
            if (! pconf->secs[s].param[p].used) {
                if (st_conf_strcmp(pconf->secs[s].name, DEF_SEC_NAME) == 0) {
                    fprintf(stderr, "Unknown option: %s\n",
                            norm_key_func(pconf->secs[s].param[p].key, true));
                } else {
                    fprintf(stderr, "Unknown option: %s%c%s\n", pconf->secs[s].name,
                            sec_sep,
                            norm_key_func(pconf->secs[s].param[p].key, true));
                }

                ret = false;
            }
        }
    }

    return ret;
}

void st_conf_help(FILE *fp)
{
    ST_CHECK_PARAM_VOID(fp == NULL);

    fprintf(fp, "Config file format: \n");
    fprintf(fp, "--------------------\n");
    fprintf(fp, "GLOBAL_KEY : VALUE\n");
    fprintf(fp, "# comments. // '#' must be in start of line.\n\n");
    fprintf(fp, "[SEC1] // section configs\n");
    fprintf(fp, "SEC1_KEY : VALUE\n\n");
    fprintf(fp, "[SEC1/SUB1/SUB2/.../SUBn] // subsection configs\n");
    fprintf(fp, "SUBSEC1_KEY : VALUE\n\n");
    fprintf(fp, "[SEC2/SUB1/SUB2/.../SUBn:%s/subsec2.conf] // load section/subsection config from the other config file\n\n", ORIGIN_PATH_VAR);
    fprintf(fp, "// Note: all whitespace will not be stripped\n");
    fprintf(fp, "--------------------\n");
}


int st_conf_merge(st_conf_t *dst, st_conf_t *src)
{
    st_conf_section_t *sec;
    int s, p;

    for (s = 0; s < src->sec_num; s++) {
        sec = st_conf_get_sec(dst, src->secs[s].name);
        if (sec == NULL) {
            sec = st_conf_new_sec(dst, src->secs[s].name);
            if (sec == NULL) {
                ST_ERROR("Failed to st_conf_new_sec[%s].", src->secs[s].name);
                return -1;
            }

        }

        for (p = 0; p < src->secs[s].param_num; p++) {
            if (st_conf_add_param(sec, src->secs[s].param[p].key,
                        src->secs[s].param[p].value) < 0) {
                ST_ERROR("Failed to st_conf_add_param.");
                return -1;
            }
        }
    }

    return 0;
}

static int st_conf_sec_comp(const void *a, const void *b)
{
    st_conf_section_t *sec1;
    st_conf_section_t *sec2;
    int ret;

    sec1 = (st_conf_section_t *)a;
    sec2 = (st_conf_section_t *)b;

    ret = st_conf_strcmp(sec1->name, sec2->name);
    if (ret == 0) {
        return 0;
    }

    // put DEF_SEC_NAME and --config in the front
    if (st_conf_strcmp(sec1->name, DEF_SEC_NAME) == 0) {
        return -1;
    }
    if (st_conf_strcmp(sec2->name, DEF_SEC_NAME) == 0) {
        return 1;
    }

    return ret;
}

void st_conf_sort_secs(st_conf_t *conf) {
    qsort(conf->secs, conf->sec_num, sizeof(st_conf_section_t),
            st_conf_sec_comp);
}
