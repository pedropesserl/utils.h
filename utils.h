#ifndef UTILS_H_
#define UTILS_H_

#include <assert.h>
#include <ctype.h> // tolower and toupper
#include <stdbool.h>
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

// source: https://github.com/tsoding/nob.h
// Append several items to a dynamic array
#define ut_array_append_many(array, items, items_count) \
    do { \
        ut_array_reserve((array), (array)->size + (items_count)); \
        memcpy((array)->data + (array)->size, (items), (items_count) * sizeof(*(array)->data)); \
        (array)->size += (items_count); \
    } while (0)

// source: https://github.com/tsoding/nob.h
#define ut_array_remove_unordered(array, i) \
    do { \
        size_t j = (i); \
        assert(j < (array)->size); \
        (array)->data[j] = (array)->data[--(array)->size]; \
    } while(0)

#define ut_array_free(array) free((array)->data)

// Allocates memory with malloc and asserts that the returned pointer is not null.
void *ut_alloc(size_t bytes);

typedef struct {
    size_t size;
    size_t capacity;
    char *data;
} ut_string;

typedef struct {
    size_t size;
    size_t capacity;
    ut_string *data;
} ut_string_array;

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
ut_string ut_string_to_lower(ut_string string);
ut_string ut_string_to_upper(ut_string string);
ut_string ut_string_slice(ut_string string, size_t index, size_t size);

// concats s with other, reallocating the contents of s
void ut_string_concat_in_place(ut_string *s, ut_string other);

// Returns the index of the first appearance of character c in string or -1 if not found
int ut_string_find(ut_string string, char c);

ut_string_array ut_string_split(ut_string string, const char *delimiters);

void ut_string_free(ut_string *string);

// Returns false if there was an error when reading or writing the file
bool ut_read_file_to_string(ut_string *string, const char *filepath);

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

ut_string ut_string_to_lower(ut_string string) {
    ut_string result = {0};
    for (size_t i = 0; i < string.size; i++) {
        ut_array_append(&result, tolower(string.data[i]));
    }
    return result;
}

ut_string ut_string_to_upper(ut_string string) {
    ut_string result = {0};
    for (size_t i = 0; i < string.size; i++) {
        ut_array_append(&result, toupper(string.data[i]));
    }
    return result;
}

ut_string ut_string_slice(ut_string string, size_t index, size_t size) {
    ut_string result = {0};
    if (index >= string.size) {
        return result;
    }
    if (index + size > string.size) {
        size = string.size - index;
    }
    ut_array_append_many(&result, string.data + index, size);
    return result;
}

void ut_string_concat_in_place(ut_string *s, ut_string other) {
    ut_array_append_many(s, other.data, other.size);
}

int ut_string_find(ut_string string, char c) {
    for (size_t i = 0; i < string.size; i++) {
        if (string.data[i] == c) {
            return (int)i;
        }
    }
    return -1;
}

ut_string_array ut_string_split(ut_string string, const char *delimiters) {
    ut_string delims = ut_string_from_cstr((char*)delimiters);
    ut_string_array arr = {0};
    // using <= here allows to get an empty string after the last delimiter if it's the last character of the string
    for (size_t i = 0; i <= string.size; i++) {
        ut_string s = {0};
        while (i < string.size && ut_string_find(delims, string.data[i]) == -1) {
            ut_array_append(&s, string.data[i]);
            i++;
        }
        ut_array_append(&arr, s);
    }
    ut_array_free(&delims);
    return arr;
}

ut_string ut_string_array_concat(ut_string_array array) {
    ut_string string = {0};
    for (size_t i = 0; i < array.size; i++) {
        ut_string_concat_in_place(&string, array.data[i]);
    }
    return string;
}

void ut_string_free(ut_string *string) {
    ut_array_free(string);
}

bool ut_read_file_to_string(ut_string *string, const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        return false;
    }
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    ut_string_free(string);
    string->capacity = size_t_max(filesize, UT_ARRAY_MIN_CAPACITY);
    string->size = filesize;
    string->data = ut_alloc(filesize);
    fread(string->data, 1, filesize, file);
    fclose(file);

    return true;
}

#endif // UTILS_IMPLEMENTATION

// stolen from https://github.com/tsoding/nob.h
#ifdef UTILS_STRIP_PREFIXES

#define array_reserve ut_array_reserve
#define array_append ut_array_append
#define array_append_many ut_array_append_many
#define array_remove_unordered ut_array_remove_unordered
#define array_free ut_array_free
typedef ut_string string;
typedef ut_string_array string_array;
#define string_from_cstr ut_string_from_cstr
#define cstr_from_string ut_cstr_from_string
#define string_dup ut_string_dup
#define string_concat ut_string_concat
#define string_to_lower ut_string_to_lower
#define string_to_upper ut_string_to_upper
#define string_slice ut_string_slice
#define string_concat_in_place ut_string_concat_in_place
#define string_find ut_string_find
#define string_split ut_string_split
#define string_array_concat ut_string_array_concat
#define string_free ut_string_free
#define read_file_to_string ut_read_file_to_string
#define write_string_to_file ut_write_string_to_file

#endif // UTILS_STRIP_PREFIXES

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
