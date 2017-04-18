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

#ifndef  _ST_POPEN_H_
#define  _ST_POPEN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <stutils/st_macro.h>

/* This implemetation of popen/pclose is mainly dealt with the memory issure.
   The traditional fork()/exec() implemetation of popen would fail with ENOMEM,
   when the parent process consume large amount of memory. Since the fork()
   fork() call will make a copy of the entire parent process' address space,
   thus double the memory usage. For detailed explanation, refer to
   http://www.oracle.com/technetwork/server-storage/solaris10/subprocess-136439.html

   Here we simply replace the fork() with vfork().
 */

FILE* st_popen(const char *program, const char *type);

#define safe_st_pclose(fp) do {\
    if((fp) != NULL) {\
        st_pclose(fp);\
        (fp) = NULL;\
    }\
    } while(0)

void st_pclose(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
