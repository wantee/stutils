/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Wang Jian
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <stutils/st_macro.h>
#include "st_varint.h"

#define LEN 16

static int unit_test_st_varint()
{
    uint8_t buf[LEN];

    int i;
    int n = 1000;
    uint64_t value, out;
    int len;
    int ncase;

    fprintf(stderr, " Testing st_varint...\n");

    ncase = 1;
    srand(time(NULL));
    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    for (i = 0; i < n; i++) {
        value = rand();
        value = (value << sizeof(int)) | rand();

        len = st_varint_encode_uint64(value, buf, LEN);
        if (len != st_varint_length_uint64(value)) {
            fprintf(stderr, "Failed. encode length not match for %"PRIu64"\n", value);
            goto FAILED;
        }

        if (st_varint_decode_uint64(buf, &out) != len) {
            fprintf(stderr, "Failed. decode length not match for %"PRIu64"\n", value);
            goto FAILED;
        }

        if (value != out) {
            fprintf(stderr, "Failed. decode value not match for %"PRIu64"\n", value);
            goto FAILED;
        }
    }

    return 0;

FAILED:
    return -1;
}

static int unit_test_st_varint_stream()
{
    FILE *fp = NULL;
    uint64_t *values;
    int *lens;

    int i;
    int n = 1000;
    uint64_t value, out;
    int len;
    int ncase;

    fprintf(stderr, " Testing st_varint_stream...\n");

    ncase = 1;
    srand(time(NULL));
    values = (uint64_t *)malloc(sizeof(uint64_t) * n);
    assert(values != NULL);
    lens = (int *)malloc(sizeof(int) * n);
    assert(lens != NULL);
    fp = tmpfile();
    assert(fp != NULL);

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    for (i = 0; i < n; i++) {
        value = rand();
        value = (value << sizeof(int)) | rand();

        len = st_varint_encode_stream_uint64(value, fp);
        if (len != st_varint_length_uint64(value)) {
            fprintf(stderr, "Failed. encode length not match for %"PRIu64"\n",
                    value);
            goto FAILED;
        }
        values[i] = value;
        lens[i] = len;
    }

    rewind(fp);
    for (i = 0; i < n; i++) {
        if (st_varint_decode_stream_uint64(fp, &out) != lens[i]) {
            fprintf(stderr, "Failed. decode length not match for %"PRIu64"\n",
                    values[i]);
            goto FAILED;
        }

        if (values[i] != out) {
            fprintf(stderr, "Failed. decode value not match for %"PRIu64"\n",
                    values[i]);
            goto FAILED;
        }
    }

    safe_fclose(fp);
    return 0;

FAILED:
    safe_fclose(fp);
    return -1;
}

static int run_all_tests()
{
    int ret = 0;

    if (unit_test_st_varint() != 0) {
        ret = -1;
    }

    if (unit_test_st_varint_stream() != 0) {
        ret = -1;
    }

    return ret;
}

int main(int argc, const char *argv[])
{
    int ret;

    fprintf(stderr, "Start testing...\n");
    ret = run_all_tests();
    if (ret != 0) {
        fprintf(stderr, "Tests failed.\n");
    } else {
        fprintf(stderr, "Tests succeeded.\n");
    }

    return ret;
}
