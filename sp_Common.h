#ifndef sp_Common_h
#define sp_Common_h

#define fnoreturn _Noreturn

#if __GNUC__ >= 3
    #define pnoreturn __attribute__((noreturn))
#else
    #define pnoreturn
#endif

#define EVAL(A) A
#define _CSTR(C) #C
#define CSTR(C) _CSTR(C)
#define _CCAT(A, B) A ## B
#define CCAT(A, B) _CCAT(A, B)
#define SRC_LOCATION (CSTR(__FILE__) ":" CSTR(__LINE__))

// link/unlink a node from a next/link form of linked list
#define NL_LIST_LINK(NODE, LIST) do {                           \
    (NODE)->next = *(LIST);                                     \
    (NODE)->link = (LIST);                                      \
    *(NODE)->link = (NODE);                                     \
    if((NODE)->next)                                            \
        (NODE)->next->link = &(NODE)->next;                     \
} while(0)

#define NL_LIST_UNLINK(NODE) do {                               \
    *(NODE)->link = (NODE)->next;                               \
    if((NODE)->next)                                            \
        (NODE)->next->link = (NODE)->link;                      \
} while(0)

#include <stdbool.h>
#include <stddef.h>

#endif // sp_Common_h