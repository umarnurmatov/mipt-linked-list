#include "dllist.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include "memutils.h"
#include "ioutils.h"
#include "assertutils.h"
#include "logutils.h"

#ifdef _DEBUG

#define DLLIST_LOG_FILENAME_ "logfile.htm"
#define IMG_FNAME_PREF_LEN_ 10

#define DLLIST_DUMP_(dllist, err) \
    dllist_dump_(dllist, err, NULL, __FILE__, __LINE__, __func__); 

#define DLLIST_DUMP_MSG_(dllist, err, msg) \
    dllist_dump_(dllist, err, msg, __FILE__, __LINE__, __func__); 

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

#define DLLIST_DUMP_(dllist, err) 

#define DLLIST_DUMP_MSG_(dllist, err) 

#define DLLIST_VERIFY_OR_RETURN_(dllist, err)  \
    if(err != DLLIST_NONE) {                   \
        return err;                            \
    }

#endif // _DEBUG

#define DLLIST_LOGC_ "dllist"

static const dllist_data_t DLLIST_NULL_ = 0;
static const dllist_data_t DLLIST_BEGIN_ = 1;

static dllist_err_t dllist_realloc_arr_(void** ptr, ssize_t nmemb, size_t tsize);

static dllist_err_t dllist_realloc_(dllist_t* dllist, ssize_t nw_cpcty);


#ifdef _DEBUG

static dllist_err_t dllist_verify_(dllist_t* dllist);

void dllist_dump_graphviz_(dllist_t* dllist, char fpref[IMG_FNAME_PREF_LEN_]);

void dllist_dump_(dllist_t* dllist, dllist_err_t err, const char* msg, const char* filename, int line, const char* funcname);

static const char* dllist_strerr_(dllist_err_t err);

#endif // _DEBUG


dllist_err_t dllist_ctor(dllist_t* dllist, ssize_t init_cpcty)
{
    utils_assert(dllist);
    utils_assert(init_cpcty > 0);

    IF_DEBUG(
        utils_init_log_file(DLLIST_LOG_FILENAME_, LOG_DIR);
    )

    dllist_err_t err = DLLIST_NONE;

#define MIN_INIT_CPCTY_ 2l

    err = dllist_realloc_(dllist, init_cpcty < MIN_INIT_CPCTY_ ? MIN_INIT_CPCTY_ : init_cpcty);

#undef MIN_INIT_CPCTY_

    DLLIST_VERIFY_OR_RETURN_(dllist, err);

    dllist->head_ind = DLLIST_NULL_;
    dllist->free_ind = DLLIST_BEGIN_;
    dllist->tail_ind = DLLIST_NULL_;
    
    dllist->size = 0; 

    DLLIST_DUMP_(dllist,err);

    return DLLIST_NONE;
}

void dllist_dtor(dllist_t* dllist)
{
    NFREE(dllist->data);
    NFREE(dllist->next);
    NFREE(dllist->prev);

    utils_end_log();
}

static dllist_err_t dllist_realloc_arr_(void** ptr, ssize_t nmemb, size_t tsize)
{
    utils_assert(ptr);
    utils_assert(nmemb > 0);

    void* tmp = 
        (dllist_data_t*) realloc (*ptr, (unsigned) nmemb * tsize);

    if(!tmp) return DLLIST_ALLOC_FAIL;

    *ptr = tmp; 

    return DLLIST_NONE;
}

static dllist_err_t dllist_realloc_(dllist_t* dllist, ssize_t nw_cpcty)
{
    utils_assert(dllist);
    utils_assert(nw_cpcty > 0);

    dllist_err_t err = DLLIST_NONE;

    err = dllist_realloc_arr_(
        (void**)&dllist->data, 
        nw_cpcty, 
        sizeof(dllist->data[0])
    );
    DLLIST_VERIFY_OR_RETURN_(dllist, err);

    for(ssize_t i = dllist->cpcty; i < nw_cpcty; ++i)
        dllist->data[i] = 0;

    err = dllist_realloc_arr_(
        (void**)&dllist->next, 
        nw_cpcty, 
        sizeof(dllist->next[0])
    );
    DLLIST_VERIFY_OR_RETURN_(dllist, err);

    for(ssize_t i = dllist->cpcty; i < nw_cpcty - 1; ++i)
        dllist->next[i] = i + 1;  
    dllist->next[nw_cpcty - 1] = DLLIST_NULL_;

    err = dllist_realloc_arr_(
        (void**)&dllist->prev, 
        nw_cpcty, 
        sizeof(dllist->prev[0])
    );
    DLLIST_VERIFY_OR_RETURN_(dllist, err);

    for(ssize_t i = dllist->cpcty; i < nw_cpcty; ++i)
        dllist->prev[i] = 0;

    dllist->cpcty = nw_cpcty;

    return DLLIST_NONE;
}

dllist_err_t dllist_insert_after(dllist_t* dllist, dllist_data_t val, ssize_t after)
{
    DLLIST_ASSERT_OK_(dllist);

    dllist_err_t err  = DLLIST_NONE;

    IF_DEBUG(
        if(after > dllist->size + DLLIST_BEGIN_ - 1) {
            err = DLLIST_OUT_OF_BOUND;
            DLLIST_DUMP_(dllist, err);
        }
    )

    ssize_t cur = dllist->free_ind;
    dllist->data[cur] = val;
    dllist->free_ind = dllist->next[cur];
    
    if(after == DLLIST_NULL_) {
        dllist->next[cur] = dllist->head_ind;
        dllist->prev[cur] = DLLIST_NULL_;

        dllist->head_ind = cur;

        if(dllist->size == 0)
            dllist->tail_ind = cur;
        else
            dllist->prev[dllist->next[cur]] = cur;
    }

    else if(after == dllist->tail_ind) {
        dllist->next[cur] = DLLIST_NULL_;
        dllist->prev[cur] = after;
        dllist->next[after] = cur;

        dllist->tail_ind = cur;
    }

    else {
        dllist->next[cur] = dllist->next[after];
        dllist->prev[cur] = after;

        dllist->prev[dllist->next[after]] = cur;

        dllist->next[after] = cur;
    }

    ++dllist->size;

    DLLIST_DUMP_(dllist, err);

    return DLLIST_NONE;
}


dllist_err_t dllist_delete_at(dllist_t* dllist, ssize_t at)
{
    DLLIST_ASSERT_OK_(dllist);

    dllist_err_t err  = DLLIST_NONE;

    IF_DEBUG(
        if(at > dllist->size) {
            err = DLLIST_OUT_OF_BOUND;
            DLLIST_DUMP_(dllist, err);
        }
    )

    if(at == dllist->head_ind) {
        dllist->head_ind = dllist->next[at];
        dllist->prev[dllist->next[at]] = DLLIST_NULL_;
    }

    else if(at == dllist->tail_ind) {
        dllist->tail_ind = dllist->prev[at];
        dllist->next[dllist->prev[at]] = DLLIST_NULL_;
    }
    else {
        dllist->next[dllist->prev[at]] = dllist->next[at];
        dllist->prev[dllist->next[at]] = dllist->prev[at];
    }

    // adding to list of free
    dllist->next[at] = dllist->free_ind;
    dllist->free_ind = at;


    --dllist->size;

    DLLIST_DUMP_(dllist, err);

    return DLLIST_NONE;
}

ssize_t dllist_next(dllist_t* dllist, ssize_t after)
{
    DLLIST_ASSERT_OK_(dllist);
    utils_assert(after >= 0);

    return dllist->next[after];
}

ssize_t dllist_prev(dllist_t* dllist, ssize_t before)
{
    DLLIST_ASSERT_OK_(dllist);
    utils_assert(before > 0);

    return dllist->prev[before];
}

ssize_t dllist_begin(dllist_t* dllist)
{
    DLLIST_ASSERT_OK_(dllist);

    return dllist->head_ind;
}


#ifdef _DEBUG

#define FNAME_ "graphviz"
#define CMD_LEN_ 100

#define CLR_GREEN_LIGHT_ "\"#B0FFB0\""
#define CLR_BLUE_LIGHT_  "\"#B0B0FF\""
#define CLR_GREEN_BOLD_  "\"#00FF00\""
#define CLR_BLUE_BOLD_   "\"#0000FF\""

void dllist_dump_(dllist_t* dllist, dllist_err_t err, const char* msg, const char* filename, int line, const char* funcname)
{
    static char fpref[IMG_FNAME_PREF_LEN_] = "";
    static unsigned int fpref_cnt = 0;

    utils_log_fprintf("<pre>\n"); 
    if(err != DLLIST_NONE) {
        utils_log_fprintf("<h3 style=\"color:red;\"> [ERROR] from %s:%d: %s() </h3>\n", filename, line, funcname);
        utils_log_fprintf("err: %s\n", dllist_strerr_(err));
    }
    else
        utils_log_fprintf("<h3> [DEBUG] from %s:%d: %s() </h3>\n", filename, line, funcname);

    if(msg)
        utils_log_fprintf("what: %s\n", msg);

    BEGIN {
        if(err == DLLIST_NULLPTR) 
            GOTO_END;

        utils_log_fprintf("capacity: %ld\n", dllist->cpcty);

        utils_log_fprintf("size: %ld\n", dllist->size);

        if(err == DLLIST_FIELD_NULLPTR)
            GOTO_END;
        
        utils_log_fprintf("data[%p]:\n  ", dllist->data);
        for(ssize_t i = 0; i < dllist->cpcty; ++i)
            utils_log_fprintf("%d ", dllist->data[i]);

        sprintf(fpref, "%u", fpref_cnt++);

        dllist_dump_graphviz_(dllist, fpref);

        utils_log_fprintf("\n<img src=" IMG_DIR "/%s.png width=1000em\n", fpref);

    } END;

    utils_log_fprintf("</pre>\n");

}

void dllist_dump_graphviz_(dllist_t* dllist, char fpref[IMG_FNAME_PREF_LEN_])
{
    FILE* file = open_file(LOG_DIR "/" FNAME_ ".txt", "w");

    fprintf(file, "digraph {\n rankdir=LR;\nsplines=ortho;\n"); 
    fprintf(file, "nodesep=1;\nranksep=0.75\n"); 

    for(ssize_t node_ind = 0; node_ind < dllist->cpcty; ++node_ind) {
        fprintf(
            file,
            "node_%ld[shape=record,label=\" "
            "data: %d | { prev: %ld | next: %ld } \"];\n",  
            node_ind,
            dllist->data[node_ind],
            dllist->prev[node_ind],
            dllist->next[node_ind]
        );
    }

    for(ssize_t node_ind = 0; node_ind < dllist->cpcty - 1; ++node_ind) {
        fprintf(
            file,
            "node_%ld -> node_%ld [weight=1000, style=invis];\n",
            node_ind,
            node_ind + 1
        );
    }
    
    ssize_t prev_ind = dllist->free_ind;
    ssize_t cur_ind = dllist->next[prev_ind];
    for( ; prev_ind != DLLIST_NULL_; cur_ind = dllist->next[prev_ind]) {
        fprintf(
            file,
            "node_%ld -> node_%ld [color=" CLR_BLUE_BOLD_ "];\n",
            prev_ind,
            cur_ind
        );
        prev_ind = cur_ind;
    }

    prev_ind = dllist->head_ind;
    cur_ind = dllist->next[prev_ind];
    for( ; prev_ind != dllist->tail_ind; cur_ind = dllist->next[prev_ind]) {
        fprintf(
            file,
            "node_%ld -> node_%ld [];\n",
            prev_ind,
            cur_ind
        );
        prev_ind = cur_ind;
    }

    fprintf(
        file,
        "node_head [label=head];"
        "node_head -> node_%ld;\n",
        dllist->head_ind
    );

    fprintf(
        file,
        "node_tail [label=tail];"
        "node_tail -> node_%ld;\n",
        dllist->tail_ind
    );

    fprintf(
        file,
        "node_free [label=free, color=" CLR_BLUE_BOLD_ ","
        "fillcolor=" CLR_BLUE_LIGHT_ "];"
        "node_free -> node_%ld [color=" CLR_BLUE_BOLD_ "]\n",
        dllist->free_ind
    );

    fprintf(file, "}\n");

    fclose(file);

    static char strbuf[CMD_LEN_ + IMG_FNAME_PREF_LEN_]= "";

    sprintf(strbuf, "dot -T png -o" LOG_DIR "/" IMG_DIR "/%s.png " LOG_DIR "/" FNAME_ ".txt", fpref);

    system(strbuf);

}

#undef FILENAME_PREF_LEN_
#undef CMD_LEN_
#undef FNAME_

static dllist_err_t dllist_verify_(dllist_t* dllist)
{
    if(!dllist)
        return DLLIST_NULLPTR;

    else if(!dllist->next)
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
        case DLLIST_NULLPTR:
            return "nullptr";
        default:
            return "unknown";
    }
}

#endif // _DEBUG
