#pragma once

#define LLIST_DECLARE(T, TVal, name)  \
typedef struct T {                   \
    TVal elem;                       \
    struct T *next;                  \
} T;                                 \
                                     \
void name##_add(T **list, TVal elem);\
void name##_remove(T **list);        \
void name##_free(T *list);           \
T **name##_findf(T **list, bool (*pred)(T *, void *), void *userdata);\

#define LLIST_DEFINE(T, TVal, name)       \
void name##_add(T **list, TVal elem)     \
{                                        \
    T *x = malloc(sizeof(T));            \
    x->elem = elem;                      \
    x->next = *list;                     \
    *list = x;                           \
}                                        \
                                         \
void name##_remove(T **list)             \
{                                        \
    T *to_remove = *list;                \
    *list = (*list)->next;               \
    free(to_remove);                     \
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
                                                                     \
T **name##_findf(T **list, bool (*pred)(T *, void *), void *userdata)\
{                                                                    \
    FallingList **p = list;                                          \
    while (*p && !pred(*p, userdata)) {                              \
        p = &(*p)->next;                                             \
    }                                                                \
    return p;                                                        \
}                                                                    \

