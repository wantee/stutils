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

#include "st_log.h"
#include "st_string.h"
#include "st_opt.h"

#define INFO_NUM 100
#define HELP_PLUGIN_NUM 100

static void st_opt_consume(int *argc, const char *argv[], unsigned optnum)
{
	memmove(&argv[optnum], &argv[optnum+1],
		sizeof(argv[optnum]) * (*argc-optnum));
	(*argc)--;
}

static int resize_opt_info(st_opt_t *opt)
{
    if (opt->info_num >= opt->info_cap) {
        opt->info_cap += INFO_NUM;
        opt->infos = (st_opt_info_t *)st_realloc(opt->infos,
                    opt->info_cap * sizeof(st_opt_info_t));
        if (opt->infos == NULL) {
            ST_ERROR("Failed to st_realloc st_opt_info.");
            goto ERR;
        }
        memset(opt->infos + opt->info_num, 0,
                INFO_NUM * sizeof(st_opt_info_t));
    }

    return 0;
ERR:
    return -1;
}

static char* st_opt_normalize_key(char *key, bool forprint)
{
    char *p;

    p = key;
    if (forprint) {
        while (*p) {
            if (*p == '_') {
                *p = '-';
            } else if (*p == '/') {
                *p = '.';
            } else {
                *p = tolower(*p);
            }
            p++;
        }
    } else {
        while (*p) {
            if (*p == '-') {
                *p = '_';
            }
            p++;
        }
    }

    return key;
}

st_opt_t* st_opt_create()
{
    st_opt_t *opt = NULL;

    opt = (st_opt_t *) st_malloc(sizeof(st_opt_t));
    if (opt == NULL) {
        ST_ERROR("Failed to st_malloc st_opt.");
        goto ERR;
    }
    memset(opt, 0, sizeof(st_opt_t));

    opt->cmd_conf = st_conf_create();
    if (opt->cmd_conf == NULL) {
        ST_ERROR("Failed to st_opt_create cmd_conf.");
        goto ERR;
    }

    opt->infos = (st_opt_info_t *)st_malloc(sizeof(st_opt_info_t)*INFO_NUM);
    if (opt->infos == NULL) {
        ST_ERROR("Failed to st_malloc st_opt_info.");
        goto ERR;
    }
    memset(opt->infos, 0, sizeof(st_opt_info_t)*INFO_NUM);
    opt->info_num = 0;
    opt->info_cap = INFO_NUM;

    opt->help_plugs = (st_opt_help_plugin_t *)st_malloc(
            sizeof(st_opt_help_plugin_t) * HELP_PLUGIN_NUM);
    if (opt->help_plugs == NULL) {
        ST_ERROR("Failed to st_malloc help_plugs");
        goto ERR;
    }
    memset(opt->help_plugs, 0, sizeof(st_opt_help_plugin_t) * HELP_PLUGIN_NUM);
    opt->help_plugs_num = 0;
    opt->help_plugs_cap = HELP_PLUGIN_NUM;

    return opt;
ERR:
    safe_st_opt_destroy(opt);
    return NULL;
}

void st_opt_destroy(st_opt_t *popt)
{
    if (popt == NULL) {
        return;
    }

    safe_st_conf_destroy(popt->file_conf);
    safe_st_conf_destroy(popt->cmd_conf);

    safe_st_free(popt->infos);
    safe_st_free(popt->help_plugs);
}

void st_opt_show(st_opt_t *popt, const char *header)
{
    char name[MAX_LINE_LEN];

    if (popt == NULL) {
        return;
    }

    if (popt->file_conf != NULL) {
        if (header == NULL) {
            snprintf(name, MAX_LINE_LEN, "-- File");
        } else {
            snprintf(name, MAX_LINE_LEN, "%s -- File", header);
        }
        st_conf_show(popt->file_conf, name);
    }

    if (header == NULL) {
        snprintf(name, MAX_LINE_LEN, "-- Command Line");
    } else {
        snprintf(name, MAX_LINE_LEN, "%s -- Command Line", header);
    }
    st_conf_show(popt->cmd_conf, name);
}

bool st_opt_check(st_opt_t *popt)
{
    bool ret = true;

    if (popt == NULL) {
        return true;
    }

    if (popt->file_conf != NULL) {
        if (! st_conf_check(popt->file_conf, NULL)) {
            ret = false;
        }
    }

    if (! st_conf_check(popt->cmd_conf, st_opt_normalize_key)) {
        ret = false;
    }

    return ret;
}

static int st_opt_info_comp(const void *a, const void *b)
{
    st_opt_info_t *info1;
    st_opt_info_t *info2;

    info1 = (st_opt_info_t *)a;
    info2 = (st_opt_info_t *)b;

    return strcmp(info1->sec_name, info2->sec_name);
}

static char* st_opt_type_str(st_opt_type_t type)
{
    switch (type) {
        case SOT_BOOL:
            return "bool";
            break;
        case SOT_INT:
            return "int";
            break;
        case SOT_UINT:
            return "uint";
            break;
        case SOT_LONG:
            return "long";
            break;
        case SOT_ULONG:
            return "ulong";
            break;
        case SOT_DOUBLE:
            return "double";
            break;
        case SOT_STR:
            return "string";
            break;
        default:
            return "UnkownType";
    }
}

static void st_info_type_print_val(st_opt_info_t *info, FILE *fp)
{
    switch (info->type) {
        case SOT_BOOL:
            if (info->bval) {
                fprintf(fp, "true");
            } else {
                fprintf(fp, "false");
            }
            break;
        case SOT_INT:
            fprintf(fp, "%d", info->ival);
            break;
        case SOT_UINT:
            fprintf(fp, "%u", info->uval);
            break;
        case SOT_LONG:
            fprintf(fp, "%ld", info->lval);
            break;
        case SOT_ULONG:
            fprintf(fp, "%lu", info->ulval);
            break;
        case SOT_DOUBLE:
            fprintf(fp, "%g", info->fval);
            break;
        case SOT_STR:
            fprintf(fp, "\"%s\"", info->sval);
            break;
        default:
            fprintf(fp, "UnkownType");
    }
}

static int st_opt_add_info(st_opt_t *opt, st_opt_type_t type,
        const char *sec_name, const char *key, void *value,
        const char *desc)
{
    st_opt_info_t *info;

    if (resize_opt_info(opt) < 0) {
        ST_ERROR("Failed to resize_opt_info.");
        return -1;
    }

    info = opt->infos + opt->info_num;
    info->type = type;
    if (sec_name == NULL || sec_name[0] == '\0'
            || st_conf_strcmp(info->sec_name, DEF_SEC_NAME) == 0) {
        info->sec_name[0] = '\0';
    } else {
        strncpy(info->sec_name, sec_name, MAX_ST_CONF_LEN);
        info->sec_name[MAX_ST_CONF_LEN - 1] = '\0';
    }

    strncpy(info->name, key, MAX_ST_CONF_LEN);
    info->name[MAX_ST_CONF_LEN - 1] = '\0';

    strncpy(info->desc, desc, MAX_LINE_LEN);
    info->desc[MAX_LINE_LEN - 1] = '\0';

    switch (type) {
        case SOT_BOOL:
            info->bval = *(bool *)value;
            break;
        case SOT_INT:
            info->ival = *(int *)value;
            break;
        case SOT_UINT:
            info->uval = *(uint *)value;
            break;
        case SOT_LONG:
            info->lval = *(long *)value;
            break;
        case SOT_ULONG:
            info->ulval = *(unsigned long *)value;
            break;
        case SOT_DOUBLE:
            info->fval = *(double *)value;
            break;
        case SOT_STR:
            strncpy(info->sval, (char *)value, MAX_ST_CONF_LEN);
            info->sval[MAX_ST_CONF_LEN - 1] = '\0';
            break;
        default:
            ST_ERROR("Unkown Option type");
            return -1;
    }

    opt->info_num++;

    return 0;
}

static int resize_opt_help_plugs(st_opt_t *opt)
{
    if (opt->help_plugs_num >= opt->help_plugs_cap) {
        opt->help_plugs_cap += HELP_PLUGIN_NUM;
        opt->help_plugs = (st_opt_help_plugin_t *)st_realloc(opt->help_plugs,
                    opt->help_plugs_cap * sizeof(st_opt_help_plugin_t));
        if (opt->help_plugs == NULL) {
            ST_ERROR("Failed to st_realloc help_plugs.");
            goto ERR;
        }
        memset(opt->help_plugs + opt->help_plugs_num, 0,
                HELP_PLUGIN_NUM * sizeof(st_opt_help_plugin_t));
    }

    return 0;
ERR:
    return -1;
}

int st_opt_add_help_plugin(st_opt_t *opt, const char *sec_name,
        const char *name, const char *desc, void (*help_func)(FILE *fp))
{
    st_opt_help_plugin_t *plug;

    if (resize_opt_help_plugs(opt) < 0) {
        ST_ERROR("Failed to resize_opt_help_plugs.");
        return -1;
    }

    plug = opt->help_plugs + opt->help_plugs_num;
    if (sec_name == NULL || sec_name[0] == '\0'
            || st_conf_strcmp(plug->sec_name, DEF_SEC_NAME) == 0) {
        plug->sec_name[0] = '\0';
    } else {
        strncpy(plug->sec_name, sec_name, MAX_ST_CONF_LEN);
        plug->sec_name[MAX_ST_CONF_LEN - 1] = '\0';
    }

    strncpy(plug->name, name, MAX_ST_CONF_LEN);
    plug->name[MAX_ST_CONF_LEN - 1] = '\0';

    strncpy(plug->desc, desc, MAX_LINE_LEN);
    plug->desc[MAX_LINE_LEN - 1] = '\0';

    plug->help_func = help_func;

    opt->help_plugs_num++;

    return 0;
}

void st_opt_show_usage(st_opt_t *opt, FILE *fp, bool show_format)
{
    char sec[MAX_ST_CONF_LEN];
    int i;
    bool b;

    ST_CHECK_PARAM_VOID(opt == NULL || fp == NULL);

    for (i = 0; i < opt->info_num; i++) {
        st_opt_normalize_key(opt->infos[i].sec_name, true);
        st_opt_normalize_key(opt->infos[i].name, true);
    }

    qsort(opt->infos, opt->info_num, sizeof(st_opt_info_t),
            st_opt_info_comp);

    sec[0] = '\0';
    for (i = 0; i < opt->info_num; i++) {
        if (strcmp(sec, opt->infos[i].sec_name) != 0) {
            strncpy(sec, opt->infos[i].sec_name, MAX_ST_CONF_LEN);
            sec[MAX_ST_CONF_LEN - 1] = '\0';
            fprintf(fp, "\n");
        }

        if (opt->infos[i].sec_name[0] == '\0') {
            fprintf(fp, "  --%-25s: %s (%s, default = ",
                    opt->infos[i].name, opt->infos[i].desc,
                    st_opt_type_str(opt->infos[i].type));
            st_info_type_print_val(opt->infos + i, fp);
            fprintf(fp, ")\n");
        } else {
            fprintf(fp, "  --%s.%*s: %s (%s, default = ",
                    opt->infos[i].sec_name,
                    -(25-1-(int)strlen(opt->infos[i].sec_name)),
                    opt->infos[i].name,
                    opt->infos[i].desc,
                    st_opt_type_str(opt->infos[i].type));
            st_info_type_print_val(opt->infos + i, fp);
            fprintf(fp, ")\n");
        }
    }
    fprintf(fp, "\n");

    if (show_format) {
        fprintf(fp, "Format: --help\n");
        fprintf(fp, "        --key=value\n");
        fprintf(fp, "        --sec.key=value (with section specified)\n");
        fprintf(fp, "        --sec.[<sub1>.<sub2>...<subN>.]key=value (with subsection(s) specified)\n\n");
    }

    if (st_opt_get_bool(opt, NULL, "help-config", &b, false,
                "show help for config file") >= 0 && b) {
        st_conf_help(fp);
    }

    for (i = 0; i < opt->help_plugs_num; i++) {
        if (st_opt_get_bool(opt, opt->help_plugs[i].sec_name,
                    opt->help_plugs[i].name, &b, false,
                    opt->help_plugs[i].desc) >= 0 && b) {
            (*(opt->help_plugs[i].help_func))(fp);
        }
    }
}

/* Returns 1 if argument consumed, 0 if all done, -1 on error. */
int st_opt_parse_one(st_opt_t *opt, int *argc, const char *argv[])
{
    char key_value[2*MAX_LINE_LEN];
    char sec_key[MAX_LINE_LEN];
    char *key, *p;

    st_conf_section_t *sec;
	unsigned arg;
    int num_kv;

    for (arg = 1; arg < *argc; arg++) {
        if (strncmp(argv[arg], "--", 2) == 0) {
            break;
        }
    }

	if (arg >= *argc) {
		return 0;
    }

	/* Special arg terminator option. */
	if (strcmp(argv[arg], "--") == 0) {
		st_opt_consume(argc, argv, arg);
		return 0;
	}

    num_kv = split_line(argv[arg] + 2, key_value, 2, MAX_LINE_LEN, "=");
    if (num_kv != 1 && num_kv != 2) {
        ST_ERROR("Failed error option[%s]", argv[arg]);
        return -1;
    }
    if (num_kv == 1) {
        if (st_conf_strcmp(key_value, "help") == 0) {
            strcpy(key_value + MAX_LINE_LEN, "true");
        } else {
            ST_ERROR("Error Format(--key=value): [%s]", argv[arg]);
            return -1;
        }
    }

    strncpy(sec_key, key_value, MAX_LINE_LEN);
    sec_key[MAX_LINE_LEN - 1] = '\0';

    key = strrchr(sec_key, '.');
    if (key != NULL) {
        *key = '\0';
        ++key;

        p = sec_key;
        while (*p != '\0') {
            if (*p == '.') {
                *p = '/';
            }
            ++p;
        }

        st_opt_normalize_key(sec_key, false);
        sec = st_conf_new_sec(opt->cmd_conf, sec_key);
        if (sec == NULL) {
            ST_ERROR("Failed to st_conf_new_sec.");
            return -1;
        }

        st_opt_normalize_key(key, false);
        if (st_conf_add_param(sec, key, key_value + MAX_LINE_LEN) < 0) {
            ST_ERROR("Failed to st_conf_add_param. key[%s], value[%s]",
                    sec_key + MAX_LINE_LEN, key_value + MAX_LINE_LEN);
            return -1;
        }
    } else {
        sec = st_conf_def_sec(opt->cmd_conf);
        if (sec == NULL) {
            ST_ERROR("ERROR: No default section");
            return -1;
        }

        st_opt_normalize_key(sec_key, false);
        if (st_conf_add_param(sec, sec_key,
                    key_value + MAX_LINE_LEN) < 0) {
            ST_ERROR("Failed to st_conf_add_param. key[%s], value[%s]",
                    sec_key, key_value + MAX_LINE_LEN);
            return -1;
        }
    }

    st_opt_consume(argc, argv, arg);
	return 1;
}

int st_opt_parse(st_opt_t *opt, int *argc, const char *argv[])
{
    char file[MAX_DIR_LEN];
	int ret;

    do {
        ret = st_opt_parse_one(opt, argc, argv);
    } while (ret == 1);

    if (ret == 0) {
        if (st_opt_get_str(opt, NULL, "config",
                    file, MAX_DIR_LEN, "", "config file. type '--help-config=true' to see detailed help.") >= 0
                && file[0] != '\0') {

            opt->file_conf = st_conf_create();
            if (opt->file_conf == NULL) {
                ST_ERROR("Failed to st_opt_create file_conf.");
                return -1;
            }

            if (st_conf_load(opt->file_conf, file) < 0) {
                ST_ERROR("Failed to st_conf_load from [%s].", file);
                return -1;
            }
        }
    }

	return ret;
}

int st_opt_get_str(st_opt_t *popt, const char *sec_name,
        const char *key, char *value, int vlen, const char *default_value,
        const char *desc)
{
    int ret;

    if (popt->file_conf != NULL) {
        ret = st_conf_get_str_def(popt->file_conf, sec_name, key,
                value, vlen, default_value);

        if (st_conf_get_str(popt->cmd_conf, sec_name, key,
                    value, vlen, NULL) < 0) {
            if (ret < 0) {
                return -1;
            }
        }
    } else {
        if (st_conf_get_str_def(popt->cmd_conf, sec_name, key,
                    value, vlen, default_value) < 0) {
            return -1;
        }
    }

    ret = st_opt_add_info(popt, SOT_STR, sec_name, key,
            (void *)default_value, desc);

    return ret;
}

int st_opt_get_bool(st_opt_t *popt, const char *sec_name, const char *key,
        bool *value, bool default_value, const char *desc)
{
    int ret;

    if (popt->file_conf != NULL) {
        ret = st_conf_get_bool_def(popt->file_conf, sec_name, key,
                value, default_value);

        if (st_conf_get_bool(popt->cmd_conf, sec_name, key,
                    value, NULL) < 0) {
            if (ret < 0) {
                return -1;
            }
        }
    } else {
        if (st_conf_get_bool_def(popt->cmd_conf, sec_name, key,
                    value, default_value) < 0) {
            return -1;
        }
    }

    ret = st_opt_add_info(popt, SOT_BOOL, sec_name, key,
            (void *)&default_value, desc);

    return ret;
}

int st_opt_get_int(st_opt_t *popt, const char *sec_name,
        const char *key, int *value, int default_value, const char *desc)
{
    int ret;

    if (popt->file_conf != NULL) {
        ret = st_conf_get_int_def(popt->file_conf, sec_name, key,
                value, default_value);

        if (st_conf_get_int(popt->cmd_conf, sec_name, key,
                    value, NULL) < 0) {
            if (ret < 0) {
                return -1;
            }
        }
    } else {
        if (st_conf_get_int_def(popt->cmd_conf, sec_name, key,
                    value, default_value) < 0) {
            return -1;
        }
    }

    ret = st_opt_add_info(popt, SOT_INT, sec_name, key,
            (void *)&default_value, desc);

    return ret;
}

int st_opt_get_uint(st_opt_t *popt, const char *sec_name,
        const char *key, unsigned int *value, unsigned int default_value,
        const char *desc)
{
    int ret;

    if (popt->file_conf != NULL) {
        ret = st_conf_get_uint_def(popt->file_conf, sec_name, key,
                value, default_value);

        if (st_conf_get_uint(popt->cmd_conf, sec_name, key,
                    value, NULL) < 0) {
            if (ret < 0) {
                return -1;
            }
        }
    } else {
        if (st_conf_get_uint_def(popt->cmd_conf, sec_name, key,
                    value, default_value) < 0) {
            return -1;
        }
    }

    ret = st_opt_add_info(popt, SOT_UINT, sec_name, key,
            (void *)&default_value, desc);

    return ret;
}

int st_opt_get_long(st_opt_t *popt, const char *sec_name,
        const char *key, long *value, long default_value, const char *desc)
{
    int ret;

    if (popt->file_conf != NULL) {
        ret = st_conf_get_long_def(popt->file_conf, sec_name, key,
                value, default_value);

        if (st_conf_get_long(popt->cmd_conf, sec_name, key,
                    value, NULL) < 0) {
            if (ret < 0) {
                return -1;
            }
        }
    } else {
        if (st_conf_get_long_def(popt->cmd_conf, sec_name, key,
                    value, default_value) < 0) {
            return -1;
        }
    }

    ret = st_opt_add_info(popt, SOT_LONG, sec_name, key,
            (void *)&default_value, desc);

    return ret;
}

int st_opt_get_ulong(st_opt_t *popt, const char *sec_name,
        const char *key, unsigned long *value, unsigned long default_value,
        const char *desc)
{
    int ret;

    if (popt->file_conf != NULL) {
        ret = st_conf_get_ulong_def(popt->file_conf, sec_name, key,
                value, default_value);

        if (st_conf_get_ulong(popt->cmd_conf, sec_name, key,
                    value, NULL) < 0) {
            if (ret < 0) {
                return -1;
            }
        }
    } else {
        if (st_conf_get_ulong_def(popt->cmd_conf, sec_name, key,
                    value, default_value) < 0) {
            return -1;
        }
    }

    ret = st_opt_add_info(popt, SOT_ULONG, sec_name, key,
            (void *)&default_value, desc);

    return ret;
}

int st_opt_get_double(st_opt_t *popt, const char *sec_name,
        const char *key, double *value, double default_value,
        const char *desc)
{
    int ret;

    if (popt->file_conf != NULL) {
        ret = st_conf_get_double_def(popt->file_conf, sec_name, key,
                value, default_value);

        if (st_conf_get_double(popt->cmd_conf, sec_name, key,
                    value, NULL) < 0) {
            if (ret < 0) {
                return -1;
            }
        }
    } else {
        if (st_conf_get_double_def(popt->cmd_conf, sec_name, key,
                    value, default_value) < 0) {
            return -1;
        }
    }

    ret = st_opt_add_info(popt, SOT_DOUBLE, sec_name, key,
            (void *)&default_value, desc);

    return ret;
}

void st_show_usage(const char *module_name, const char *header,
        const char *usage, const char *eg,
        st_opt_t *opt, const char *trailer)
{
    fprintf(stderr, "Usage: %s [options] %s\n", module_name, usage);
    if (eg != NULL) {
        fprintf(stderr, "e.g.: \n");
        fprintf(stderr, "  %s %s\n", module_name, eg);
    }
    fprintf(stderr, "\n");
    if (opt != NULL) {
        fprintf(stderr, "Options: \n");
        st_opt_show_usage(opt, stderr, true);
    }
    if (trailer != NULL) {
        fprintf(stderr, "\n%s\n", trailer);
    }
}

int st_opt_print_cmd(st_opt_t *opt, FILE *fp)
{
    char leading[MAX_ST_CONF_LEN];
    st_conf_t *conf = NULL;
    st_conf_section_t *sec;
    int s, p;

    ST_CHECK_PARAM(opt == NULL || fp == NULL, -1);

    conf = st_conf_create();
    if (conf == NULL) {
        ST_ERROR("Failed to st_opt_create conf.");
        goto ERR;
    }

    if (opt->file_conf != NULL) {
        sec = st_conf_def_sec(opt->cmd_conf);
        if (sec != NULL) {
            for (p = 0; p < sec->param_num; p++) {
                if (st_conf_strcmp(sec->param[p].key, "config") == 0) {
                    fprintf(fp, "--config=%s\n", sec->param[p].value);
                }
            }
        }
        if (st_conf_merge(conf, opt->file_conf) < 0) {
            ST_ERROR("Failed to st_conf_merge file_conf.");
            return -1;
        }
    }

    if (st_conf_merge(conf, opt->cmd_conf) < 0) {
        ST_ERROR("Failed to st_conf_merge cmd_conf.");
        return -1;
    }

    st_conf_sort_secs(conf);

    for (s = 0; s < conf->sec_num; s++) {
        sec = conf->secs + s;
        if (st_conf_strcmp(sec->name, DEF_SEC_NAME) != 0) {
            snprintf(leading, MAX_ST_CONF_LEN, "--%s.",
                    st_opt_normalize_key(sec->name, true));
        } else {
            snprintf(leading, MAX_ST_CONF_LEN, "--");
        }
        for (p = 0; p < sec->param_num; p++) {
            if (st_conf_strcmp(sec->name, DEF_SEC_NAME) == 0
                    && st_conf_strcmp(sec->param[p].key, "config") == 0) {
                continue;
            }
            fprintf(fp, "%s%s=%s\n", leading, st_opt_normalize_key(sec->param[p].key, true),
                    sec->param[p].value);
        }
    }

    safe_st_conf_destroy(conf);
    return 0;

ERR:
    safe_st_conf_destroy(conf);
    return -1;
}
