/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Wang Jian
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

#include <stutils/st_macro.h>
#include "st_bit.h"

static int unit_test_st_nbit()
{
    unsigned char *A = NULL;
    size_t nb;
    int u;

    size_t i;
    int ncase;

    fprintf(stderr, " Testing st_nbit...\n");

    ncase = 1;
    nb = 100;
    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    u = 1;

    A = (unsigned char*)realloc(A, NBITNSLOTS(nb, u));
    assert(A != NULL);
    memset(A, 0, NBITNSLOTS(nb, u));

    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != 0) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)(i % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)(i % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)((i * 2) % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)((i * 2) % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }
    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    u = 2;

    A = (unsigned char*)realloc(A, NBITNSLOTS(nb, u));
    assert(A != NULL);
    memset(A, 0, NBITNSLOTS(nb, u));

    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != 0) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)(i % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)(i % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)((i * 2) % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)((i * 2) % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    u = 4;

    A = (unsigned char*)realloc(A, NBITNSLOTS(nb, u));
    assert(A != NULL);
    memset(A, 0, NBITNSLOTS(nb, u));

    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != 0) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)(i % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)(i % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)((i * 2) % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)((i * 2) % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    u = 8;

    A = (unsigned char*)realloc(A, NBITNSLOTS(nb, u));
    assert(A != NULL);
    memset(A, 0, NBITNSLOTS(nb, u));

    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != 0) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)(i % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)(i % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)((i * 2) % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)((i * 2) % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    u = 16;

    A = (unsigned char*)realloc(A, NBITNSLOTS(nb, u));
    assert(A != NULL);
    memset(A, 0, NBITNSLOTS(nb, u));

    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != 0) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)(i % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)(i % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)((i * 2) % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)((i * 2) % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    u = 3;

    A = (unsigned char*)realloc(A, NBITNSLOTS(nb, u));
    assert(A != NULL);
    memset(A, 0, NBITNSLOTS(nb, u));

    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != 0) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)(i % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)(i % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)((i * 2) % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)((i * 2) % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    u = 12;

    A = (unsigned char*)realloc(A, NBITNSLOTS(nb, u));
    assert(A != NULL);
    memset(A, 0, NBITNSLOTS(nb, u));

    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != 0) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)(i % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)(i % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)((i * 2) % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)((i * 2) % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    u = 21;

    A = (unsigned char*)realloc(A, NBITNSLOTS(nb, u));
    assert(A != NULL);
    memset(A, 0, NBITNSLOTS(nb, u));

    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != 0) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)(i % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)(i % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    for (i = 0; i < nb; i++) {
        st_nbit_set(A, u, i, (int)((i * 2) % (1 << u)));
    }
    for (i = 0; i < nb; i++) {
        if (st_nbit_get(A, u, i) != (int)((i * 2) % (1 << u))) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }

    safe_free(A);

    return 0;

FAILED:
    safe_free(A);
    return -1;
}

static int run_all_tests()
{
    int ret = 0;

    if (unit_test_st_nbit() != 0) {
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
