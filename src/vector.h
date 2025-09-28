#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

static inline size_t vector_grow_cap(size_t old_cap)
{
    return old_cap < 8 ? 8 : old_cap * 2;
}

typedef void *(*VectorAllocator)(void *ptr, size_t old, size_t new);
typedef int (*VectorComparator)(const void *, const void *);

static inline void *vector_allocator(void *ptr, size_t old, size_t new)
{
    (void) old;
    if (new == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, new);
}

#define VECTOR_INIT() { .size = 0, .cap = 0, .data = NULL, .allocator = vector_allocator }
#define VECTOR_INIT_ALLOCATOR(a) { .size = 0, .cap = 0, .data = NULL, .allocator = a }

#define VECTOR_DECLARE(T, TVal, header)                          \
    typedef struct T {                                           \
        size_t size;                                             \
        size_t cap;                                              \
        TVal *data;                                              \
        VectorAllocator allocator;                               \
    } T;                                                         \
                                                                 \
void header##_init(T *arr);                                      \
void header##_init_allocator(T *arr, VectorAllocator allocator); \
void header##_free(T *arr);                                      \
void header##_add(T *arr, TVal value);                           \

// size_t header##_search(T *arr, TVal elem, VectorComparator comp);

#define VECTOR_DEFINE(T, TVal, header)                           \
void header##_init(T *arr)                                       \
{                                                                \
    arr->size = 0;                                               \
    arr->cap  = 0;                                               \
    arr->data = NULL;                                            \
    arr->allocator = vector_allocator;                           \
}                                                                \
                                                                 \
void header##_init_allocator(T *arr, VectorAllocator allocator)  \
{                                                                \
    arr->size = 0;                                               \
    arr->cap  = 0;                                               \
    arr->data = NULL;                                            \
    arr->allocator = allocator;                                  \
}                                                                \
                                                                 \
void header##_free(T *arr)                                       \
{                                                                \
    arr->allocator(arr->data, arr->cap, 0);                      \
    header##_init(arr);                                          \
}                                                                \
                                                                 \
void header##_add(T *arr, TVal value)                            \
{                                                                \
    if (arr->cap < arr->size + 1) {                              \
        size_t old = arr->cap;                                   \
        arr->cap = vector_grow_cap(old);                         \
        arr->data = arr->allocator(arr->data, old, arr->cap);    \
    }                                                            \
    arr->data[arr->size++] = value;                              \
}                                                                \

// size_t header##_search(T *arr, TVal elem, VectorComparator comp)
// {
//     for (size_t i = 0; i < arr->size; i++) {
//         if (comp(&arr->data[i], &elem)) {
//             return i;
//         }
//     }
//     return j;
// }

#endif

