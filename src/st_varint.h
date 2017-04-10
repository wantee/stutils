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

#ifndef  _ST_VARINT_H_
#define  _ST_VARINT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <inttypes.h>

/*
 * LEB128 encoding. see https://en.wikipedia.org/wiki/LEB128
 */

/*
 * Get encoded length of a value for uint64.
 *
 * @param[in] value value to be encoded.
 *
 * @return encoded length, -1 if any error.
 */
int st_varint_length_uint64(uint64_t value);

/*
 * Encode uint64 to a byte buffer.
 *
 * @param[in] value value to be encoded.
 * @param[out] buf buffer of encoded bytes.
 * @param[in] buf_len length of buffer.
 *
 * @return encoded length, -1 if any error.
 */
int st_varint_encode_uint64(uint64_t value, uint8_t *buf, size_t buf_len);

/*
 * Decode uint64 from a byte buffer.
 *
 * @param[in] buf buffer contains the encoded bytes.
 * @param[out] value value decoded.
 *
 * @return decoded length, -1 if any error.
 */
int st_varint_decode_uint64(uint8_t *buf, uint64_t *value);

/*
 * Encode uint64 to a FILE stream.
 *
 * @param[in] value value to be encoded.
 * @param[out] fp FILE stream to write to.
 *
 * @return encoded length, -1 if any error.
 */
int st_varint_encode_stream_uint64(uint64_t value, FILE *fp);

/*
 * Decode uint64 from a FILE stream.
 *
 * @param[in] fp FILE stream to read from.
 * @param[out] value value decoded.
 *
 * @return decoded length, -1 if any error.
 */
int st_varint_decode_stream_uint64(FILE *fp, uint64_t *value);

#ifdef __cplusplus
}
#endif

#endif
