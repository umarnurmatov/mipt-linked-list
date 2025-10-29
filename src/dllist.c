#include "dllist.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include "memutils.h"
#include "ioutils.h"
#include "assertutils.h"
#include "logutils.h"

#ifdef _DEBUG

#define DLLIST_LOG_FILENAME_ "logfile.html"
#define GRAPHVIZ_IMG_FNAME_PREF_LEN_ 10

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

static const dllist_data_t DLLIST_NULL_ = 0;

static dllist_err_t dllist_realloc_arr_(void** ptr, ssize_t nmemb, size_t tsize);

static dllist_err_t dllist_realloc_(dllist_t* dllist, ssize_t nw_cpcty);


#ifdef _DEBUG

static dllist_err_t dllist_verify_(dllist_t* dllist);

void dllist_dump_graphviz_(dllist_t* dllist, char fpref[GRAPHVIZ_IMG_FNAME_PREF_LEN_]);

void dllist_dump_(dllist_t* dllist, dllist_err_t err, char* msg, const char* filename, int line, const char* funcname);

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

    err = dllist_realloc_(dllist, init_cpcty);

    DLLIST_VERIFY_OR_RETURN_(dllist, err);

    dllist->free = init_cpcty > DLLIST_NULL_ ? DLLIST_NULL_ + 1 : DLLIST_NULL_;

    dllist->next[DLLIST_NULL_] = DLLIST_NULL_;
    dllist->prev[DLLIST_NULL_] = DLLIST_NULL_;
    dllist->size               = 0; 

    DLLIST_DUMP_(dllist,err);

    return DLLIST_NONE;
}

void dllist_dtor(dllist_t* dllist)
{
    utils_assert(dllist);

    NFREE(dllist->data);
    NFREE(dllist->next);
    NFREE(dllist->prev);
    
    dllist->size = 0;
    dllist->free = 0;

    utils_end_log();
}

static dllist_err_t dllist_realloc_arr_(void** ptr, ssize_t nmemb, size_t tsize)
{
    utils_assert(ptr);
    utils_assert(nmemb > 0);

    void* tmp = 
        (dllist_data_t*)realloc(*ptr, (unsigned) nmemb * tsize);

    if(!tmp) return DLLIST_ALLOC_FAIL;

    *ptr = tmp; 

    return DLLIST_NONE;
}

static dllist_err_t dllist_realloc_(dllist_t* dllist, ssize_t nw_cpcty)
{
    utils_assert(dllist);
    utils_assert(nw_cpcty > dllist->cpcty);

    dllist_err_t err = DLLIST_NONE;

    err = dllist_realloc_arr_(
        (void**)&dllist->data, 
        nw_cpcty, 
        sizeof(dllist->data[0])
    );
    DLLIST_VERIFY_OR_RETURN_(dllist, err);
    
    memset(
        dllist->data + dllist->cpcty, 
        0, 
        sizeof(dllist->data[0]) * (size_t)(nw_cpcty - dllist->cpcty)
    );

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
    
    memset(
        dllist->prev + dllist->cpcty, 
        0, 
        sizeof(dllist->prev[0]) * (size_t)(nw_cpcty - dllist->cpcty)
    );

    dllist->cpcty = nw_cpcty;

    return DLLIST_NONE;
}

dllist_err_t dllist_insert_after(dllist_t* dllist, dllist_data_t val, ssize_t after)
{
    DLLIST_ASSERT_OK_(dllist);

    dllist_err_t err = DLLIST_NONE;

    IF_DEBUG(
        if(after > dllist->size)
            err = DLLIST_OUT_OF_BOUND;

        else if(after < DLLIST_NULL_)
            err = DLLIST_OUT_OF_BOUND;

        if(err != DLLIST_NONE) {
            DLLIST_DUMP_(dllist, err);
            return err;
        }
    )
    
    if(dllist->free == DLLIST_NULL_) {
        dllist->free = dllist->cpcty;
        err = dllist_realloc_(dllist, dllist->cpcty * 2);
        DLLIST_VERIFY_OR_RETURN_(dllist, err);
    }

    ssize_t cur = dllist->free;

    dllist->data[cur] = val;
    dllist->free      = dllist->next[cur];

    dllist->next[cur]                 = dllist->next[after];
    dllist->prev[cur]                 = after;
    dllist->prev[dllist->next[after]] = cur;
    dllist->next[after]               = cur;

    ++dllist->size;

    DLLIST_DUMP_(dllist, err);

    return DLLIST_NONE;
}


dllist_err_t dllist_delete_at(dllist_t* dllist, ssize_t at)
{
    DLLIST_ASSERT_OK_(dllist);

    dllist_err_t err = DLLIST_NONE;

    IF_DEBUG(
        if(at > dllist->size)
            err = DLLIST_OUT_OF_BOUND;

        else if(at <= DLLIST_NULL_)
            err = DLLIST_OUT_OF_BOUND;

        if(err != DLLIST_NONE) {
            DLLIST_DUMP_(dllist, err);
            return err;
        }
    )

    dllist->next[dllist->prev[at]] = dllist->next[at];
    dllist->prev[dllist->next[at]] = dllist->prev[at];

    dllist->next[at] = dllist->free;
    dllist->free     = at;

    --dllist->size;

    DLLIST_DUMP_(dllist, err);

    return DLLIST_NONE;
}

ssize_t dllist_next(dllist_t* dllist, ssize_t after)
{
    DLLIST_ASSERT_OK_(dllist);

    IF_DEBUG(
        dllist_err_t err = DLLIST_NONE; 

        if(after < 0)
            err = DLLIST_OUT_OF_BOUND;
        else if(after >= dllist->size)
            err = DLLIST_OUT_OF_BOUND;

        if(err != DLLIST_NULL_) {
            DLLIST_DUMP_(dllist, err);
            return err;
        }
    );

    return dllist->next[after];
}

ssize_t dllist_prev(dllist_t* dllist, ssize_t before)
{
    DLLIST_ASSERT_OK_(dllist);

    IF_DEBUG(
        dllist_err_t err = DLLIST_NONE; 

        if(before <= 0)
            err = DLLIST_OUT_OF_BOUND;
        else if(before > dllist->size)
            err = DLLIST_OUT_OF_BOUND;

        if(err != DLLIST_NULL_) {
            DLLIST_DUMP_(dllist, err);
            return err;
        }
    );

    return dllist->prev[before];
}

ssize_t dllist_begin(dllist_t* dllist)
{
    DLLIST_ASSERT_OK_(dllist);

    return dllist->next[DLLIST_NULL_];
}

ssize_t dllist_end(dllist_t* dllist)
{
    DLLIST_ASSERT_OK_(dllist);

    return dllist->prev[DLLIST_NULL_];
}


#ifdef _DEBUG

#define GRAPHVIZ_FNAME_ "graphviz"
#define GRAPHVIZ_CMD_LEN_ 100

#define CLR_RED_LIGHT_   "\"#FFB0B0\""
#define CLR_GREEN_LIGHT_ "\"#B0FFB0\""
#define CLR_BLUE_LIGHT_  "\"#B0B0FF\""

#define CLR_RED_BOLD_    "\"#FF0000\""
#define CLR_GREEN_BOLD_  "\"#00FF00\""
#define CLR_BLUE_BOLD_   "\"#0000FF\""

void dllist_dump_(dllist_t* dllist, dllist_err_t err, char* msg, const char* filename, int line, const char* funcname)
{
    static char fpref[GRAPHVIZ_IMG_FNAME_PREF_LEN_] = "";
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

        utils_log_fprintf("size: %ld", dllist->size);

        if(err == DLLIST_FIELD_NULLPTR)
            GOTO_END;
        
        utils_log_fprintf("\ndata[%p]:  ", dllist->data);
        for(ssize_t i = 0; i < dllist->cpcty; ++i)
            utils_log_fprintf("%5d ", dllist->data[i]);

        utils_log_fprintf("\nnext[%p]:  ", dllist->next);
        for(ssize_t i = 0; i < dllist->cpcty; ++i)
            utils_log_fprintf("%5ld ", dllist->next[i]);

        utils_log_fprintf("\nprev[%p]:  ", dllist->prev);
        for(ssize_t i = 0; i < dllist->cpcty; ++i)
            utils_log_fprintf("%5ld ", dllist->prev[i]);

        utils_log_fprintf("\n");

        sprintf(fpref, "%u", fpref_cnt++);

        dllist_dump_graphviz_(dllist, fpref);

        utils_log_fprintf("\n<img src=" IMG_DIR "/%s.svg width=500em\n", fpref);

    } END;

    utils_log_fprintf("</pre>\n");

}

void dllist_dump_graphviz_(dllist_t* dllist, char fpref[GRAPHVIZ_IMG_FNAME_PREF_LEN_])
{
    FILE* file = open_file(LOG_DIR "/" GRAPHVIZ_FNAME_ ".txt", "w");

    fprintf(file, "digraph {\n rankdir=LR;\nsplines=ortho;\n"); 
    fprintf(file, "nodesep=0.9;\nranksep=0.75;\nnode [fontname=\"Fira Mono\"];\n");

    fprintf(
        file,
        "node_%d[shape=record,"
        "label=\"data: %d | { prev: %ld | next: %ld } \","
        "color=" CLR_RED_BOLD_ ","
        "style=\"filled,bold,rounded\","
        "fillcolor=" CLR_RED_LIGHT_ "];\n",
        DLLIST_NULL_,
        dllist->data[DLLIST_NULL_],
        dllist->prev[DLLIST_NULL_],
        dllist->next[DLLIST_NULL_]
    );

    for(ssize_t node_ind = DLLIST_NULL_ + 1; node_ind < dllist->cpcty; ++node_ind) {
        fprintf(
            file,
            "node_%ld[shape=record,"
            "label=\" data: %d | { prev: %ld | next: %ld } \","
            "constraint=false];\n",  
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
    
    ssize_t prev_ind = dllist->free;
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

    prev_ind = DLLIST_NULL_;
    cur_ind = dllist_begin(dllist);
    do {
        if(prev_ind == DLLIST_NULL_ || cur_ind == DLLIST_NULL_)
            fprintf(
                file,
                "node_%ld -> node_%ld [color=" CLR_RED_BOLD_ "];\n",
                prev_ind,
                cur_ind
            );
        else
            fprintf(
                file,
                "node_%ld -> node_%ld [];\n",
                prev_ind,
                cur_ind
            );
        prev_ind = cur_ind;
        cur_ind = dllist->next[prev_ind];
    } while(prev_ind != DLLIST_NULL_);

    fprintf(
        file,
        "node_free [label=free,color=" CLR_BLUE_BOLD_ ","
        "shape=rectangle,"
        "style=\"filled,rounded\","
        "fillcolor=" CLR_BLUE_LIGHT_ "];\n"
        "node_free -> node_%ld [color=" CLR_BLUE_BOLD_ "]\n",
        dllist->free
    );

    fprintf(file, "}\n");

    fclose(file);

    static char strbuf[GRAPHVIZ_CMD_LEN_ + GRAPHVIZ_IMG_FNAME_PREF_LEN_]= "";

    sprintf(
        strbuf, 
        "dot -T svg -o" LOG_DIR "/" IMG_DIR "/%s.svg " LOG_DIR "/" GRAPHVIZ_FNAME_ ".txt", fpref
    );

    system(strbuf);

}

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
