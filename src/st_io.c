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

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <stutils/st_macro.h>
#include "st_log.h"
#include "st_mem.h"
#include "st_popen.h"
#include "st_io.h"

FILE* st_fopen(const char *name, const char *mode)
{
    char cmd[MAX_NAME_LEN];
    char m[5];

    ST_CHECK_PARAM(name == NULL || mode == NULL, NULL);

    if (name[0] == '-' && name[1] == '\0') {
        if (mode[0] == 'r') {
            return stdin;
        } else if (mode[0] == 'w' || mode[0] == 'a') {
            return stdout;
        } else {
            ST_ERROR("Unkown mode[%s].", mode);
            return NULL;
        }
    } else if (name[0] == '|' && (mode[0] == 'w' || mode[0] == 'a')) {
        m[0] = 'w';
        m[1] = '\0';
        return st_popen(name + 1, m);
    } else if (name[strlen(name) - 1] == '|' && mode[0] == 'r') {
        if (strlen(name) >= MAX_NAME_LEN) {
            ST_ERROR("Too long name[%s]", name);
            return NULL;
        }
        strcpy(cmd, name);
        cmd[strlen(name) - 1] = '\0';
        m[0] = 'r';
        m[1] = '\0';
        return st_popen(cmd, m);
    } else {
        return fopen(name, mode);
    }
}

void st_fclose(FILE *fp)
{
    struct stat s;

    if (fp == stdin || fp == stdout) {
        return;
    }

    if(fstat(fileno(fp), &s) != 0) {
        ST_ERROR("Failed to fstat[%m]");
        return;
    }

    if ((s.st_mode & S_IFMT) == S_IFIFO) {
        st_pclose(fp);
    } else {
        fclose(fp);
    }
}

char* st_fgets(char **line, size_t *sz, FILE *fp, bool *err)
{
    char *ptr;
    size_t old_sz;
    size_t n;

    int ch;

    ST_CHECK_PARAM(line == NULL || sz == NULL || fp == NULL, NULL);

    if (err != NULL) {
        *err = false;
    }

    if (*line == NULL || *sz <= 0) {
        *line = (char *)st_malloc(MAX_LINE_LEN);
        if (*line == NULL) {
            ST_ERROR("Failed to st_malloc line");
            goto ERR;
        }
        *sz = MAX_LINE_LEN;
    }

    ptr = *line;
    n = *sz;

    while (--n) {
        if ((ch = getc(fp)) == EOF) {
            if (ptr == *line) {
                return NULL;
            }
            break;
        }

        if ((*ptr++ = ch) == '\n') {
            break;
        }
    }

    if (n > 0) {
        *ptr = '\0';
        return *line;
    }

    do {
        old_sz = *sz;
        *sz *= 2;

        *line = (char *)st_realloc(*line, *sz);
        if (*line == NULL) {
            ST_ERROR("Failed to st_realloc line. size[%zu -> %zu]",
                    old_sz, *sz);
            goto ERR;
        }

        ptr = (*line) + old_sz - 1;
        n = old_sz + 1;

        while (--n) {
            if ((ch = getc(fp)) == EOF) {
                break;
            }

            if ((*ptr++ = ch) == '\n') {
                break;
            }
        }
    } while (n == 0);

    *ptr = '\0';

    return *line;
ERR:
    *sz = 0;
    if (err != NULL) {
        *err = true;
    }
    return NULL;
}

int st_readline(FILE *fp, const char *fmt, ...)
{
    char *line = NULL;
    size_t sz = 0;

    va_list args;
    int ret;

    if (st_fgets(&line, &sz, fp, NULL) == NULL) {
        ST_ERROR("Failed to read line.");
        goto ERR;
    }

    va_start (args, fmt);
    ret = vsscanf(line, fmt, args);
    va_end (args);

    safe_st_free(line);
    return ret;

ERR:
    safe_st_free(line);
    return -1;
}

off_t st_fsize(const char *filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

size_t st_count_lines(const char *filename)
{
    FILE *fp = NULL;
    size_t num_lines;
    int ch;

    fp = st_fopen(filename, "r");
    if (fp == NULL) {
        ST_ERROR("Failed to open file[%s]", filename);
        goto ERR;
    }

    num_lines = 0;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            ++num_lines;
        }
    }
    if (ferror(fp)) {
        ST_ERROR("Read error for file[%s]", filename);
        goto ERR;
    }

    safe_fclose(fp);

    return num_lines;

ERR:
    safe_fclose(fp);
    return (size_t)-1;
}
