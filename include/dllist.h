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
    DLLIST_OUT_OF_BOUND,
    DLLIST_NULLPTR
} dllist_err_t;

typedef struct dllist_t
{
    dllist_data_t* data;
    
    ssize_t* next;
    ssize_t* prev;

    ssize_t head_ind;
    ssize_t tail_ind;
    ssize_t free_ind;

    ssize_t cpcty;
    ssize_t size;

} dllist_t;

dllist_err_t dllist_ctor(dllist_t* dllist, ssize_t init_cpcty);

void dllist_dtor(dllist_t* dllist);

dllist_err_t dllist_insert_after(dllist_t* dllist, dllist_data_t val, ssize_t after);

dllist_err_t dllist_delete_at(dllist_t* dllist, ssize_t at);

ssize_t dllist_next(dllist_t* dllist, ssize_t after);

ssize_t dllist_prev(dllist_t* dllist, ssize_t before);

ssize_t dllist_begin(dllist_t* dllist);

