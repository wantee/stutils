/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Wang Jian
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

#include "stutils/st_utils.h"
#include "stutils/st_string.h"
#include "stutils/st_opt.h"

st_opt_t *g_cmd_opt;


void show_usage(const char *module_name)
{
    st_show_usage(module_name,
        "Show options parsed by st_opt",
        "[--config=<conf-file>] [--opt1=val1] [pos-args1] ...",
        "--config=st.conf --key=value foo bar",
        g_cmd_opt, NULL);
}

int parse_opt(int *argc, const char *argv[])
{
    g_cmd_opt = st_opt_create();
    if (g_cmd_opt == NULL) {
        fprintf(stderr, "Failed to st_opt_create.\n");
        goto ST_OPT_ERR;
    }

    if (st_opt_parse(g_cmd_opt, argc, argv) < 0) {
        fprintf(stderr, "Failed to st_opt_parse.\n");
        goto ST_OPT_ERR;
    }

    return 0;

ST_OPT_ERR:
    return -1;
}


int main(int argc, const char *argv[])
{
    char args[1024] = "";
    int i;

    if (argc <= 1) {
        show_usage(argv[0]);
        return 0;
    }

    (void)st_escape_args(argc, argv, args, 1024);

    if (parse_opt(&argc, argv) < 0) {
        fprintf(stderr, "Failed to parse_opt.\n");
        goto ERR;
    }

    printf("Command-line:\n");
    printf("%s\n", args);

    printf("\nOptions:\n");
    if (st_opt_print_cmd(g_cmd_opt, stdout) < 0) {
        fprintf(stderr, "Failed to st_opt_print_cmd.");
        goto ERR;
    }

    printf("\nPositinal arguments:\n");
    for (i = 1; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    safe_st_opt_destroy(g_cmd_opt);
    return 0;

ERR:
    safe_st_opt_destroy(g_cmd_opt);

    return -1;
}
