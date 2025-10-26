#pragma once

#include <stdlib.h>

#ifdef _DEBUG

    #define IF_DEBUG(expr) expr

#else // _DEBUG
    
    #define IF_DEBUG(expr)

#endif // _DEBUG

#define DLLIST_MAKE(varname) \
    dllist_t varname = {     \
        .data     = NULL,    \
        .next     = NULL,    \
        .prev     = NULL,    \
        .head_ind = 0,       \
        .tail_ind = 0,       \
        .free_ind = 0,       \
        .cpcty    = 0,       \
        .size     = 0        \
    }

typedef int dllist_data_t;

typedef enum dllist_err_t
{
    DLLIST_NONE,
    DLLIST_ALLOC_FAIL,
    DLLIST_FIELD_NULLPTR,
    DLLIST_OUT_OF_BOUND
} dllist_err_t;

typedef struct dllist_t
{
    dllist_data_t* data;
    
    size_t* next;
    size_t* prev;

    size_t head_ind;
    size_t tail_ind;
    size_t free_ind;

    size_t cpcty;
    size_t size;

} dllist_t;

dllist_err_t dllist_ctor(dllist_t* dllist, size_t size);

void dllist_dtor(dllist_t* dllist);

dllist_err_t dllist_insert(dllist_t* dllist, dllist_data_t val, size_t after);

