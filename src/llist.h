#pragma once

#define LLIST_DECLARE(T, TVal, name)  \
typedef struct T {                   \
    TVal elem;                       \
    struct T *next;                  \
} T;                                 \
                                     \
void name##_add(T **list, TVal elem);\
void name##_remove(T *list);         \
void name##_free(T *list);           \

#define LLIST_DEFINE(T, TVal, name)       \
void name##_add(T **list, TVal elem)     \
{                                        \
    T *x = malloc(sizeof(T));            \
    x->elem = elem;                      \
    x->next = *list;                     \
    *list = x;                           \
}                                        \
                                         \
void name##_add_next(T *p, TVal elem)    \
{                                        \
    T *x = malloc(sizeof(T));            \
    x->elem = elem;                      \
    x->next = p->next;                   \
    p->next = x;                         \
}                                        \
                                         \
void name##_remove(T *list)              \
{                                        \
    free(list);                          \
}                                        \
                                         \
void name##_free(T *list)                \
{                                        \
    for (T *p = list; p; ) {             \
        T *tmp = p;                      \
        p = p->next;                     \
        free(tmp);                       \
    }                                    \
}                                        \

