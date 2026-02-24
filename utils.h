#ifndef UTILS_H_
#define UTILS_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define UT_ARRAY_MIN_CAPACITY 256

// source: https://github.com/tsoding/nob.h
// Increase the capacity of a dynamic array
#define ut_array_reserve(array, expected_capacity) \
    do { \
        if ((expected_capacity) > (array)->capacity) { \
            if ((array)->capacity == 0) { \
                (array)->capacity = UT_ARRAY_MIN_CAPACITY; \
            } \
            while ((expected_capacity) > (array)->capacity) { \
                (array)->capacity *= 2; \
            } \
            (array)->data = realloc((array)->data, (array)->capacity * sizeof(*(array)->data)); \
            assert((array)->data != NULL && "Buy more RAM lol"); \
        } \
    } while (0)

// source: https://github.com/tsoding/nob.h
// Append an item to a dynamic array
#define ut_array_append(array, item) \
    do { \
        ut_array_reserve((array), (array)->size + 1); \
        (array)->data[(array)->size++] = (item); \
    } while (0)

#define ut_array_free(array) free((array)->data)

// Allocates memory with malloc and asserts that the returned pointer is not null.
void *ut_alloc(size_t bytes);

typedef struct {
    size_t size;
    size_t capacity;
    char *data;
} ut_string;

// source: https://github.com/tsoding/nob.h
// Macros for printing a ut_string with printf
#define STRFMT "%.*s"
#define FMTSTR(string) (int)(string).size, (string).data
// usage:
//   ut_string name = ...;
//   printf("Name: " STRFMT "\n", FMTSTR(name));

// These functions create a new string from the inputs. The caller should free
// the allocated memory.
ut_string ut_string_from_cstr(char *cstr);
char *ut_cstr_from_string(ut_string string);
ut_string ut_string_dup(ut_string src);
ut_string ut_string_concat(ut_string a, ut_string b);

#endif // UTILS_H_

#ifdef UTILS_IMPLEMENTATION

void *ut_alloc(size_t bytes) {
    void *mem = malloc(bytes);
    assert(mem != NULL && "Buy more RAM lol");
    return mem;
}

static inline size_t size_t_max(size_t a, size_t b) {
    return a > b ? a : b;
}

ut_string ut_string_from_cstr(char *cstr) {
    size_t size = strlen(cstr);
    size_t capacity = size_t_max(size + 1, UT_ARRAY_MIN_CAPACITY);
    char *cpy = ut_alloc(capacity);
    memcpy(cpy, cstr, size);
    cpy[size] = '\0';
    return (ut_string){
        .size = size,
        .capacity = capacity,
        .data = cpy,
    };
}

char *ut_cstr_from_string(ut_string string) {
    return strndup(string.data, string.size);
}

ut_string ut_string_dup(ut_string src) {
    return (ut_string){
        .size = src.size,
        .capacity = src.capacity,
        .data = strndup(src.data, src.size),
    };
}

ut_string ut_string_concat(ut_string a, ut_string b) {
    size_t capacity = size_t_max(a.size + b.size + 1, UT_ARRAY_MIN_CAPACITY);
    char *cat = ut_alloc(capacity);
    memcpy(cat, a.data, a.size);
    memcpy(cat + a.size, b.data, b.size);
    cat[a.size + b.size] = '\0';
    return (ut_string) {
        .capacity = capacity,
        .size = a.size + b.size,
        .data = cat,
    };
}

#endif // UTILS_IMPLEMENTATION

// LICENSE
// Copyright (c) 2026 Pedro Pesserl
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
