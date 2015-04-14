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

#ifndef  _ST_UTILS_H_
#define  _ST_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "st_macro.h"

long make_long(unsigned int a, unsigned int b);

void split_long(long l, unsigned int *a, unsigned int *b);

void remove_newline(char *line);

void remove_leading_space(char *line);

int split_line(const char *line, char fields[][MAX_LINE_LEN], 
        int max_field, const char *sep);

unsigned int highest_bit_mask(unsigned int num, int overflow);

void iqsort(int *arr, int l, int u,
        int cmp(int *arr, int i, int j, void *args), void *args);

typedef enum _encoding_type_t_
{
    ENCODING_GBK  = 0,
    ENCODING_UTF8 = 1,
} encoding_type_t;

int get_next_char(const char *token, encoding_type_t encoding);

char* get_next_token(char *line, char *token);

void remove_newline(char *line);

uint32_t MurmurHash2 ( const void * key, int len, uint32_t seed );

void st_shuffle(int *a, size_t n);
void st_shuffle_r(int *a, size_t n, unsigned *rand);

int st_readline(FILE *fp, const char *fmt, ...);

int st_escape(const char *str, char *ans, size_t ans_len);

int st_escape_args(int argc, const char *argv[], char *ans, size_t ans_len);

off_t st_fsize(const char *filename);

typedef struct _st_gauss_r_t {
    double mean;
    double stdev;
    unsigned seed;

    double V1;
    double V2; 
    double S;
    int phase;
} st_gauss_r_t;

double st_gaussrand();
double st_normrand(double mean, double stdev);

void st_gauss_r_init(st_gauss_r_t *gauss, double mean,
        double stdev, unsigned seed);
double st_gaussrand_r(st_gauss_r_t *gauss);

#ifdef __cplusplus
}
#endif

#endif

