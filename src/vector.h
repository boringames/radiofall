#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <stdlib.h>
#include <string.h>

typedef void *(*vector_allocator)(void *ptr, size_t old, size_t nnew);

static inline void *vector_default_allocator(void *ptr, size_t old, size_t nnew)
{
    (void) old;
    if (nnew == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, nnew);
}

#define VECTOR_INIT() { .size = 0, .cap = 0, .data = NULL, .allocator = vector_default_allocator }
#define VECTOR_INIT_ALLOCATOR(a) { .size = 0, .cap = 0, .data = NULL, .allocator = a }

#define VECTOR_DECLARE(T, TVal, header)                                      \
    typedef struct T {                                                       \
        ptrdiff_t size;                                                      \
        ptrdiff_t cap;                                                       \
        TVal *data;                                                          \
        vector_allocator allocator;                                          \
    } T;                                                                     \
                                                                             \
void header##_init(T *arr);                                                  \
void header##_init_allocator(T *arr, vector_allocator allocator);            \
void header##_free(T *arr);                                                  \
void header##_add(T *arr, TVal value);                                       \
TVal header##_remove(T *arr);                                                \
ptrdiff_t header##_find(T *arr, TVal elem, bool (*comp)(TVal, TVal));        \
ptrdiff_t header##_findf(T *arr, bool (*pred)(TVal, void *), void *userdata);\
void header##_remove_i(T *arr, ptrdiff_t i);                                 \

#define VECTOR_DEFINE(T, TVal, header)                                            \
void header##_init(T *arr)                                                        \
{                                                                                 \
    arr->size = 0;                                                                \
    arr->cap  = 0;                                                                \
    arr->data = NULL;                                                             \
    arr->allocator = vector_default_allocator;                                    \
}                                                                                 \
                                                                                  \
void header##_init_allocator(T *arr, vector_allocator allocator)                  \
{                                                                                 \
    arr->size = 0;                                                                \
    arr->cap  = 0;                                                                \
    arr->data = NULL;                                                             \
    arr->allocator = allocator;                                                   \
}                                                                                 \
                                                                                  \
void header##_free(T *arr)                                                        \
{                                                                                 \
    arr->allocator(arr->data, arr->cap, 0);                                       \
    header##_init(arr);                                                           \
}                                                                                 \
                                                                                  \
void header##_add(T *arr, TVal value)                                             \
{                                                                                 \
    if (arr->cap < arr->size + 1) {                                               \
        ptrdiff_t old = arr->cap;                                                 \
        arr->cap = arr->cap < 8 ? 8 : arr->cap * 2;                               \
        arr->data = arr->allocator(arr->data, old,                                \
                arr->cap * sizeof(TVal));                                         \
    }                                                                             \
    arr->data[arr->size++] = value;                                               \
}                                                                                 \
                                                                                  \
TVal header##_remove(T *arr)                                                      \
{                                                                                 \
    return arr->data[--arr->size];                                                \
}                                                                                 \
                                                                                  \
ptrdiff_t header##_find(T *arr, TVal elem, bool (*comp)(TVal, TVal))              \
{                                                                                 \
    for (ptrdiff_t i = 0; i < arr->size; i++) {                                   \
        if (comp(arr->data[i], elem)) {                                           \
            return i;                                                             \
        }                                                                         \
    }                                                                             \
    return -1;                                                                    \
}                                                                                 \
                                                                                  \
ptrdiff_t header##_findf(T *arr, bool (*pred)(TVal, void *), void *userdata)      \
{                                                                                 \
    for (ptrdiff_t i = 0; i < arr->size; i++) {                                   \
        if (pred(arr->data[i], userdata)) {                                       \
            return i;                                                             \
        }                                                                         \
    }                                                                             \
    return -1;                                                                    \
}                                                                                 \
                                                                                  \
void header##_remove_i(T *arr, ptrdiff_t i)                                       \
{                                                                                 \
    memmove(&arr->data[i], &arr->data[i+1], (arr->cap - i) * sizeof(TVal));       \
    arr->size--;                                                                  \
}                                                                                 \

#endif
