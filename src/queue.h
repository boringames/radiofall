#pragma once

#include <stddef.h>

#define QUEUE_NEXT(i) ((i + 1) % 8)

#define QUEUE_DECLARE(T, TName, name, size)         \
typedef struct TName {                        \
    T data[size];                                \
    int h, t;                                 \
} TName;                                      \
void name##_init(TName *q);                   \
bool name##_enqueue(TName *q, T x);           \
bool name##_dequeue(TName *q, T *out);        \

#define QUEUE_DEFINE(T, TName, name, size)             \
/* initialize a queue. data is not initialized */\
void name##_init(TName *q)                       \
{                                                \
    q->h = q->t = 0;                             \
}                                                \
                                                 \
/* adds an element to the tail of the queue */   \
bool name##_enqueue(TName *q, T x)               \
{                                                \
    if ((q->t + 1) % size == q->h) {                \
        return false;                            \
    }                                            \
    q->data[q->t] = x;                           \
    q->t = (q->t + 1) % size;                       \
    return true;                                 \
}                                                \
                                                 \
/* remove an element from the head */            \
bool name##_dequeue(TName *q, T *out)            \
{                                                \
    if (q->h == q->t) {                          \
        return false;                            \
    }                                            \
    if (out) {                                   \
        *out = q->data[q->h];                    \
    }                                            \
    q->h = (q->h + 1) % size;                       \
    return true;                                 \
}                                                \
                                                 \
/* moves tail and returns a ptr to the new elem */ \
T *name##_enqueue_ptr(TName *q)                  \
{                                                \
    if ((q->t + 1) % size == q->h) {                \
        return NULL;                             \
    }                                            \
    T *x = &q->data[q->t];                       \
    q->t = (q->t + 1) % size;                       \
    return x;                                    \
}                                                \
                                                 \
/* removes an element from tail. probably not safe */ \
bool name##_remove_tail(TName *q, T *out)        \
{                                                \
    if (q->t == q->h) {                          \
        return false;                            \
    }                                            \
    if (out) {                                   \
        *out = q->data[q->t];                    \
    }                                            \
    q->t = (q->t - 1) % size;                    \
    return true;                                 \
}                                                \

