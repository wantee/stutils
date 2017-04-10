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
#include <string.h>

#include <stutils/st_macro.h>
#include "st_log.h"
#include "st_bit.h"

static inline size_t st_nbit_slot(int u, size_t pos)
{
    return u * pos / CHAR_BIT;
}

static inline size_t st_nbit_slot_begin(int u, size_t pos)
{
    return (u * pos) % CHAR_BIT;
}

static inline unsigned char st_get_nbit_val_in_byte(unsigned char b, int s, int n)
{
    int a = CHAR_BIT - s;
    return (b & ((1 << a) - 1)) >> (a - n);
}

static inline unsigned char st_clear_nbit_in_byte(unsigned char b, int s, int n)
{
    int a = CHAR_BIT - s;
    return b & ~(((1 << a) - 1) & (0xFF << (a - n)));
}

void st_nbit_set(unsigned char *a, int u, size_t pos, int val)
{
    size_t slot;
    int v;
    int begin;

    int n;

    v = val & ((1 << u) - 1);
    begin = st_nbit_slot_begin(u, pos);
    // set last bits to last byte
    slot = st_nbit_slot(u, pos);
    slot += (begin + u - 1) / CHAR_BIT;
    n = (u - (CHAR_BIT - begin)) % CHAR_BIT;
    if (n > 0) {
        a[slot] = st_clear_nbit_in_byte(a[slot], 0, n);
        a[slot] |= (v & ((1 << n) - 1)) << (CHAR_BIT - n);
        v >>= n;
        --slot;
    } else {
        n = 0;
    }

    // set intermediate bits to intermediate bytes
    while (u - n >= CHAR_BIT) {
        a[slot] = v & 0x00FF;
        v >>= CHAR_BIT;
        n += CHAR_BIT;
        --slot;
    }

    // set first bits to first byte
    if (n < u) {
        a[slot] = st_clear_nbit_in_byte(a[slot], begin, u - n);
        a[slot] |= v << (CHAR_BIT - (u - n) - begin);
    }
}

int st_nbit_get(unsigned char *a, int u, size_t pos)
{
    size_t slot;

    int begin;
    int val;
    int n;

    val = 0;
    // get first bits from first byte
    begin = st_nbit_slot_begin(u, pos);
    slot = st_nbit_slot(u, pos);
    n = min(u, CHAR_BIT - begin);
    val |= st_get_nbit_val_in_byte(a[slot], begin, n);
    ++slot;

    // get intermediate bits from intermediate bytes
    while (n <= u - CHAR_BIT) {
        val = val << CHAR_BIT | a[slot];
        n += CHAR_BIT;
        ++slot;
    }

    // get last bits from last byte
    if (n < u) {
        val = val << (u - n) | st_get_nbit_val_in_byte(a[slot], 0, u - n);
    }

    return val;
}
