#include "dllist.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include "memutils.h"
#include "ioutils.h"
#include "assertutils.h"
#include "logutils.h"

#ifdef _DEBUG

#define DLLIST_DUMP_(dllist, err) dllist_dump_(dllist, err, __FILE__, __LINE__, __func__); 

#define DLLIST_ASSERT_OK_(dllist)                    \
    {                                                \
        dllist_err_t err = dllist_verify_(dllist);   \
        utils_assert(err == DLLIST_NONE);            \
    }

#define DLLIST_VERIFY_OR_RETURN_(dllist, err)   \
    if(err != DLLIST_NONE) {                    \
        DLLIST_DUMP_(dllist, err);              \
        return err;                             \
    }

#else // _DEBUG

#define DLLIST_DUMP(dllist, err) 

#define DLLIST_VERIFY_OR_RETURN_(dllist, err)  \
    if(err != DLLIST_NONE) {                   \
        return err;                            \
    }

#endif // _DEBUG

#define DLLIST_LOGC "dllist"

static dllist_err_t dllist_realloc_arr_(void** ptr, size_t nmemb, size_t tsize);

static dllist_err_t dllist_realloc_(dllist_t* dllist, size_t nw_cpcty);


#ifdef _DEBUG

static dllist_err_t dllist_verify_(dllist_t* dllist);

void dllist_dump_graphviz_(dllist_t* dllist);

void dllist_dump_(dllist_t* dllist, dllist_err_t err, const char* filename, int line, const char* funcname);

static const char* dllist_strerr_(dllist_err_t err);

#endif // _DEBUG


dllist_err_t dllist_ctor(dllist_t* dllist, size_t size)
{
    utils_assert(dllist);

    dllist_err_t err = DLLIST_NONE;

#define MIN_INIT_CPCTY_ 2

    err = dllist_realloc_(dllist, size < MIN_INIT_CPCTY_ ? MIN_INIT_CPCTY_ : size);

#undef MIN_INIT_CPCTY_

    DLLIST_VERIFY_OR_RETURN_(dllist, err);

    dllist->head_ind = 0;
    dllist->free_ind = 1;
    dllist->tail_ind = 3;
    
    dllist->size = 0; 

    dllist_dump_graphviz_(dllist);

    return DLLIST_NONE;
}

void dllist_dtor(dllist_t* dllist)
{
    NFREE(dllist->data);
    NFREE(dllist->next);
    NFREE(dllist->prev);
}

static dllist_err_t dllist_realloc_arr_(void** ptr, size_t nmemb, size_t tsize)
{
    utils_assert(ptr);

    void* tmp = 
        (dllist_data_t*) realloc (*ptr, nmemb * tsize);

    if(!tmp) return DLLIST_ALLOC_FAIL;

    *ptr = tmp; 

    return DLLIST_NONE;
}

static dllist_err_t dllist_realloc_(dllist_t* dllist, size_t nw_cpcty)
{
    utils_assert(dllist);

    dllist_err_t err = DLLIST_NONE;

    err = dllist_realloc_arr_(
        (void**)&dllist->data, 
        nw_cpcty, 
        sizeof(dllist->data[0])
    );
    DLLIST_VERIFY_OR_RETURN_(dllist, err);

    for(size_t i = dllist->cpcty; i < nw_cpcty; ++i)
        dllist->data[i] = 0;

    err = dllist_realloc_arr_(
        (void**)&dllist->next, 
        nw_cpcty, 
        sizeof(dllist->next[0])
    );
    DLLIST_VERIFY_OR_RETURN_(dllist, err);

    for(size_t i = dllist->cpcty; i < nw_cpcty; ++i)
        dllist->next[i] = i + 1;  

    err = dllist_realloc_arr_(
        (void**)&dllist->prev, 
        nw_cpcty, 
        sizeof(dllist->prev[0])
    );
    DLLIST_VERIFY_OR_RETURN_(dllist, err);

    for(size_t i = dllist->cpcty; i < nw_cpcty; ++i)
        dllist->prev[i] = 0;

    dllist->cpcty = nw_cpcty;

    return DLLIST_NONE;
}

dllist_err_t dllist_insert(dllist_t* dllist, dllist_data_t val, size_t after)
{
    DLLIST_ASSERT_OK_(dllist);

    dllist_err_t err  = DLLIST_NONE;

    IF_DEBUG(
        if(after > dllist->size) {
            err = DLLIST_OUT_OF_BOUND;
            DLLIST_DUMP_(dllist, err);
        }
    )

    if(dllist->size == 0) {
        size_t new_ind = 1;
        dllist->data[new_ind] = val;
        dllist->tail_ind = new_ind;
        
        dllist->next[new_ind] = dllist->next[after];
        dllist->next[after] = new_ind;

        ++dllist->size;
    }

    return DLLIST_NONE;
}


#ifdef _DEBUG

void dllist_dump_(dllist_t* dllist, dllist_err_t err, const char* filename, int line, const char* funcname)
{
    UTILS_LLOGE(DLLIST_LOGC, "%s", dllist_strerr_(err));
}

void dllist_dump_graphviz_(dllist_t* dllist)
{
    // DLLIST_ASSERT_OK_(dllist);

#define FNAME_ "graphviz"

    FILE* file = open_file(FNAME_ ".txt", "w");

    fprintf(file, "digraph {\n rankdir=LR;\n"); 

    for(size_t node_ind = 0; node_ind < dllist->cpcty; ++node_ind) {
        fprintf(
            file,
            "node_%lu[shape=record,label=\" "
            "data: %d | { prev: %lu | next: %lu } \"]",  
            node_ind,
            dllist->data[node_ind],
            dllist->prev[node_ind],
            dllist->next[node_ind]
        );
    }

    for(size_t node_ind = 0; node_ind < dllist->cpcty - 1; ++node_ind) {
        fprintf(
            file,
            "node_%lu -> node_%lu [weight=1000, style=invis]",
            node_ind,
            node_ind + 1
        );
    }
    
    size_t prev_ind = 0, cur_ind = 0;
    for( ; prev_ind != dllist->tail_ind; cur_ind = dllist->next[prev_ind]) {
        fprintf(
            file,
            "node_%lu -> node_%lu []",
            prev_ind,
            cur_ind
        );
        prev_ind = cur_ind;
    }

    fprintf(file, "}\n");

    fclose(file);
    
    system("dot -T png -o graphviz.o -o" FNAME_ ".png " FNAME_ ".txt");

#undef FNAME_
}

static dllist_err_t dllist_verify_(dllist_t* dllist)
{
    if(!dllist->next)
        return DLLIST_FIELD_NULLPTR;

    else if(!dllist->data)
        return DLLIST_FIELD_NULLPTR;

    else if(!dllist->prev)
        return DLLIST_FIELD_NULLPTR;

    return DLLIST_NONE;
}

static const char* dllist_strerr_(dllist_err_t err)
{
    switch(err) {
        case DLLIST_NONE:
            return "none";
        case DLLIST_ALLOC_FAIL:
            return "allocation failed";
        case DLLIST_FIELD_NULLPTR:
            return "struct field is nullptr";
        case DLLIST_OUT_OF_BOUND:
            return "index out of bound";
        default:
            return "unknown";
    }
}

#endif // _DEBUG
