#pragma once

#include <stddef.h>

#define QUEUE_NEXT(i) ((i + 1) % 8)

#define QUEUE_DECLARE(T, TName, name)         \
typedef struct TName {                        \
    T data[8];                                \
    int h, t;                                 \
} TName;                                      \
void name##_init(TName *q);                   \
bool name##_enqueue(TName *q, T x);           \
bool name##_dequeue(TName *q, T *out);        \

#define QUEUE_DEFINE(T, TName, name)             \
void name##_init(TName *q)                       \
{                                                \
    q->h = q->t = 0;                             \
}                                                \
                                                 \
bool name##_enqueue(TName *q, T x)               \
{                                                \
    if (QUEUE_NEXT(q->t) == q->h) {              \
        return false;                            \
    }                                            \
    q->data[q->t] = x;                           \
    q->t = QUEUE_NEXT(q->t);                     \
    return true;                                 \
}                                                \
                                                 \
bool name##_dequeue(TName *q, T *out)            \
{                                                \
    if (q->h == q->t) {                          \
        return false;                            \
    }                                            \
    if (out) {                                   \
        *out = q->data[q->h];                    \
    }                                            \
    q->h = QUEUE_NEXT(q->h);                     \
    return true;                                 \
}                                                \
                                                 \
T *name##_enqueue_ptr(TName *q)                  \
{                                                \
    if (QUEUE_NEXT(q->t) == q->h) {              \
        return NULL;                             \
    }                                            \
    T *x = &q->data[q->t];                       \
    q->t = QUEUE_NEXT(q->t);                     \
    return x;                                    \
}                                                \

