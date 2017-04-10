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
#include "st_varint.h"

static const char MSB = 0x80;
static const char MSBALL = ~0x7F;

static const unsigned long long N1 = 128; // 2 ^ 7
static const unsigned long long N2 = 16384;
static const unsigned long long N3 = 2097152;
static const unsigned long long N4 = 268435456;
static const unsigned long long N5 = 34359738368;
static const unsigned long long N6 = 4398046511104;
static const unsigned long long N7 = 562949953421312;
static const unsigned long long N8 = 72057594037927936;
static const unsigned long long N9 = 9223372036854775808U;


int st_varint_length_uint64(uint64_t n)
{
	return (
			n < N1 ? 1
			: n < N2 ? 2
			: n < N3 ? 3
			: n < N4 ? 4
			: n < N5 ? 5
			: n < N6 ? 6
			: n < N7 ? 7
			: n < N8 ? 8
			: n < N9 ? 9
			:         10
		   );
}

int st_varint_encode_uint64(uint64_t value, uint8_t *buf, size_t buf_len)
{
	uint8_t* ptr;

	ST_CHECK_PARAM(buf == NULL, -1);

	ptr = buf;
	while (value & MSBALL) {
		*(ptr++) = (value & 0xFF) | MSB;
		value = value >> 7;
		if ((ptr - buf) >= buf_len) {
            ST_WARNING("buf overflow");
            return -1;
        }
	}
	*ptr = value;

	return ptr - buf + 1;
}

int st_varint_decode_uint64(uint8_t *buf, uint64_t *value)
{
	uint8_t *ptr;
	int len = 0;
	uint64_t u;

	ST_CHECK_PARAM(buf == NULL || value == NULL, -1);

    *value = 0;
    ptr = buf;
	while (*ptr & MSB) {
		u = *ptr;
		*value += ((u & 0x7F) << len);
		++ptr;
		len += 7;
	}
	u = *ptr;
	*value += ((u & 0x7F) << len);

	return  ptr - buf + 1;
}

int st_varint_encode_stream_uint64(uint64_t value, FILE *fp)
{
	uint8_t c;
    int len;

	ST_CHECK_PARAM(fp == NULL, -1);

    len = 0;
	while (value & MSBALL) {
		c = (value & 0xFF) | MSB;
        if (fwrite(&c, sizeof(uint8_t), 1, fp) != 1) {
            ST_WARNING("Failed to write encoded stream.");
            return -1;
        }
        ++len;
		value = value >> 7;
	}
    c = value;
    if (fwrite(&c, sizeof(uint8_t), 1, fp) != 1) {
        ST_WARNING("Failed to write encoded stream.");
        return -1;
    }
    ++len;

	return len;
}

int st_varint_decode_stream_uint64(FILE *fp, uint64_t *value)
{
	uint64_t u;
    uint8_t c;

    int bits;
	int len;

	ST_CHECK_PARAM(fp == NULL || value == NULL, -1);

    bits = 0;
    len = 0;
    *value = 0;

    if (fread(&c, sizeof(uint8_t), 1, fp) != 1) {
        ST_WARNING("Failed to read decoded stream.");
        return -1;
    }
    ++len;
	while (c & MSB) {
		u = c;
		*value += ((u & 0x7F) << bits);
        if (fread(&c, sizeof(uint8_t), 1, fp) != 1) {
            ST_WARNING("Failed to read decoded stream.");
            return -1;
        }
        ++len;
		bits += 7;
	}
	u = c;
	*value += ((u & 0x7F) << bits);

	return  len;
}
