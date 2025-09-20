#pragma once

#include <stddef.h>

#define QUEUE_DECLARE(T, TName, name, size)         \
typedef struct TName {                              \
    size_t h, t;                                    \
    T data[size];                                   \
} TName;                                            \
void name##_init(TName *q);                         \
bool name##_enqueue(TName *q, T x);                 \
T *name##_enqueue_ptr(TName *q);                    \
bool name##_dequeue(TName *q, T *out);              \
bool name##_dequeue_tail(TName *q, T *out);         \
size_t name##_size(TName *q);                       \
T *name##_peek(TName *q);                           \

#define QUEUE_DEFINE(T, TName, name, size)             \
/* initialize a queue. data is not initialized */      \
void name##_init(TName *q)                             \
{                                                      \
    q->h = q->t = 0;                                   \
}                                                      \
                                                       \
/* adds an element to the tail of the queue */         \
bool name##_enqueue(TName *q, T x)                     \
{                                                      \
    if ((q->t + 1) % size == q->h) {                   \
        return false;                                  \
    }                                                  \
    q->data[q->t] = x;                                 \
    q->t = (q->t + 1) % size;                          \
    return true;                                       \
}                                                      \
                                                       \
/* moves tail and returns a ptr to the new elem */     \
T *name##_enqueue_ptr(TName *q)                        \
{                                                      \
    if ((q->t + 1) % size == q->h) {                   \
        return NULL;                                   \
    }                                                  \
    T *x = &q->data[q->t];                             \
    q->t = (q->t + 1) % size;                          \
    return x;                                          \
}                                                      \
                                                       \
/* remove an element from the head */                  \
bool name##_dequeue(TName *q, T *out)                  \
{                                                      \
    if (q->h == q->t) {                                \
        return false;                                  \
    }                                                  \
    if (out) {                                         \
        *out = q->data[q->h];                          \
    }                                                  \
    q->h = (q->h + 1) % size;                          \
    return true;                                       \
}                                                      \
                                                       \
/* removes an element from tail. probably not safe */  \
bool name##_dequeue_tail(TName *q, T *out)             \
{                                                      \
    if (q->t == q->h) {                                \
        return false;                                  \
    }                                                  \
    if (out) {                                         \
        *out = q->data[q->t];                          \
    }                                                  \
    q->t = (q->t - 1) % size;                          \
    return true;                                       \
}                                                      \
                                                       \
/* returns the size of the queue */                    \
size_t name##_size(TName *q)                           \
{                                                      \
    return q->t >= q->h                                \
        ? q->t - q->h                                  \
        : (size - q->h) + q->t;                        \
}                                                      \
                                                       \
/* peeks nth element of the queue */                   \
T *name##_peek(size_t n)                               \
{                                                      \
    return &q->data[(q->t + n) % size];                \
}                                                      \
                                                       \
/* peeks at the first item about to get dequeued */    \
T *name##_peek(TName *q) {                             \
    if (q->h == q->t) return NULL;                     \
    return &q->data[q->h];                             \
}                                                      \

