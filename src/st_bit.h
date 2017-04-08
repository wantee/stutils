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

#ifndef  _ST_BIT_H_
#define  _ST_BIT_H_

#ifdef __cplusplus
extern "C" {
#endif

// bit sets from http://c-faq.com/misc/bitsets.html
#include <limits.h>		/* for CHAR_BIT */

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

// n-bit array
#define NBITNSLOTS(nb, u) (((nb) * (u) + CHAR_BIT - 1) / CHAR_BIT)

/*
 * Set value of n-bit array.
 *
 * @param[in] a bit array.
 * @param[in] u number of unit bits.
 * @param[in] pos slot position.
 * @param[in] val specified value.
 */
void st_nbit_set(unsigned char *a, int u, size_t pos, int val);

/*
 * Get value of n-bit array.
 *
 * @param[in] a bit array.
 * @param[in] u number of unit bits.
 * @param[in] pos position to be set.

 * @return the corresponding value.
 */
int st_nbit_get(unsigned char *a, int u, size_t pos);

#ifdef __cplusplus
}
#endif

#endif
