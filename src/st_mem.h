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

#ifndef  _ST_MEM_H_
#define  _ST_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _ST_MEM_DEBUG_
/*
 * wrapper of st_malloc
 *
 * @param[in] size size of bytes for alloc.
 * @param[in] file source file name.
 * @param[in] line source file line.
 * @param[in] func caller function name.
 * @return pointer to the alloced memory, NULL if any error.
 */
void* st_malloc_wrapper(size_t size,
        const char *file, size_t line, const char *func);

/*
 * wrapper of st_realloc
 *
 * @param[in] ptr original block. This pointer must be the one returned
 *                from st_malloc or st_realloc.
 *                If ptr == NULL, it will return a new block.
 * @param[in] size new size of block.
 * @param[in] file source file name.
 * @param[in] line source file line.
 * @param[in] func caller function name.
 * @return pointer to the alloced memory, NULL if any error.
 */
void* st_realloc_wrapper(void *ptr, size_t size,
        const char *file, size_t line, const char *func);

/*
 * warpper of st_aligned_malloc
 *
 * @param[in] size size of bytes for alloc.
 * @param[in] alignment size of alignment. Must be power of 2.
 * @param[in] file source file name.
 * @param[in] line source file line.
 * @param[in] func caller function name.
 * @return pointer to the alloced memory, NULL if any error.
 */
void* st_aligned_malloc_wrapper(size_t size, size_t alignment,
        const char *file, size_t line, const char *func);

/*
 * warpper of st_aligned_realloc
 * @param[in] ptr original block. This pointer must be the one returned
 *                from st_aligned_malloc or st_aligned_realloc.
 *                If ptr == NULL, it will return a new aligned block.
 * @param[in] size new size of block.
 * @param[in] alignment size of alignment. Must be power of 2. If it is not
 *                      equal the alignment of ptr, it will be realigned.
 * @param[in] file source file name.
 * @param[in] line source file line.
 * @param[in] func caller function name.
 * @return pointer to the realloced memory, NULL if any error.
 */
void* st_aligned_realloc_wrapper(void *ptr, size_t size, size_t alignment,
        const char *file, size_t line, const char *func);

#define st_malloc(sz) st_malloc_wrapper(sz, __FILE__, __LINE__, __func__)
#define st_realloc(p, sz) st_realloc_wrapper(p, sz, __FILE__, __LINE__, __func__)
#define st_aligned_malloc(sz, ali) st_aligned_malloc_wrapper(sz, ali, __FILE__, __LINE__, __func__)
#define st_aligned_realloc(p, sz, ali) st_aligned_realloc_wrapper(p, sz, ali, __FILE__, __LINE__, __func__)

#else //_ST_MEM_DEBUG_

#define st_malloc st_malloc_impl
#define st_realloc st_realloc_impl
#define st_aligned_malloc st_aligned_malloc_impl
#define st_aligned_realloc st_aligned_realloc_impl

#endif // _ST_MEM_DEBUG_

/*
 * alloc memory block.
 *
 * @param[in] size size of bytes for alloc.
 * @return pointer to the alloced memory, NULL if any error.
 */
void* st_malloc_impl(size_t size);

/*
 * realloc memory block.
 *
 * @param[in] ptr original block. This pointer must be the one returned
 *                from st_malloc or st_realloc.
 *                If ptr == NULL, it will return a new block.
 * @param[in] size new size of block.
 * @return pointer to the realloced memory, NULL if any error.
 */
void* st_realloc_impl(void *ptr, size_t size);

#define safe_st_free(ptr) do {\
    if((ptr) != NULL) {\
        st_free(ptr);\
        (ptr) = NULL;\
    }\
    } while(0)
/*
 * free a memory block.
 *
 * @param[in] ptr memory block. This pointer must be the one returned
 *                from st_malloc or st_realloc.
 */
void st_free(void *p);

/*
 * Get size of a memory block.
 *
 * @param[in] ptr memory block. This pointer must be the one returned
 *                from st_malloc or st_realloc.
 * @return the size of block
 */
size_t st_mem_size(void *p);

/*
 * initialize memory usage statistics
 *
 * @return non-zero if any error.
 */
int st_mem_usage_init();

/*
 * report memory usage statistics
 *
 */
void st_mem_usage_report();

/*
 * destroy memory usage statistics
 *
 */
void st_mem_usage_destroy();

/*
 * alloc aligned memory block.
 *
 * @param[in] size size of bytes for alloc.
 * @param[in] alignment size of alignment. Must be power of 2.
 * @return pointer to the alloced memory, NULL if any error.
 */
void* st_aligned_malloc_impl(size_t size, size_t alignment);

/*
 * realloc aligned memory block.
 *
 * @param[in] ptr original block. This pointer must be the one returned
 *                from st_aligned_malloc or st_aligned_realloc.
 *                If ptr == NULL, it will return a new aligned block.
 * @param[in] size new size of block.
 * @param[in] alignment size of alignment. Must be power of 2. If it is not
 *                      equal the alignment of ptr, it will be realigned.
 * @return pointer to the realloced memory, NULL if any error.
 */
void* st_aligned_realloc_impl(void *ptr, size_t size, size_t alignment);

#define safe_st_aligned_free(ptr) do {\
    if((ptr) != NULL) {\
        st_aligned_free(ptr);\
        (ptr) = NULL;\
    }\
    } while(0)
/*
 * free a aligned memory block.
 *
 * @param[in] ptr memory block. This pointer must be the one returned
 *                from st_aligned_malloc or st_aligned_realloc.
 */
void st_aligned_free(void *p);

/*
 * Get alignment of a aligned memory block.
 *
 * @param[in] ptr memory block. This pointer must be the one returned
 *                from st_aligned_malloc or st_aligned_realloc.
 * @return the alignment of block
 */
size_t st_aligned_alignment(void *p);

/*
 * Get size of a aligned memory block.
 *
 * @param[in] ptr memory block. This pointer must be the one returned
 *                from st_aligned_malloc or st_aligned_realloc.
 * @return the size of block
 */
size_t st_aligned_size(void *p);

#ifdef __cplusplus
}
#endif

#endif
