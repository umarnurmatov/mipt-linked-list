
#include "dllist.h"
#include "utils.h"
#include "optutils.h"

static utils_long_opt_t long_opts[] = 
{
    { OPT_ARG_REQUIRED, "log", NULL, 0, 0 },
};

int main(int argc, char* argv[])
{
    utils_long_opt_get(argc, argv, long_opts, SIZEOF(long_opts));

    DLLIST_MAKE(list);
    
#define DLLIST_VERIFY(expr) if(expr != DLLIST_NONE) GOTO_END;

    BEGIN {
        DLLIST_VERIFY(dllist_ctor(&list, 2, long_opts[0].arg));

        DLLIST_VERIFY(dllist_insert_after(&list, 10, 0));
        DLLIST_VERIFY(dllist_insert_after(&list, 10, 0));
        DLLIST_VERIFY(dllist_insert_after(&list, 10, 0));

        list.next[2] = 1000;

        DLLIST_VERIFY(dllist_insert_after(&list, 10, 0));

        dllist_dtor(&list);

        return EXIT_SUCCESS;
    } END;

#undef DLLIST_VERIFY
    
    dllist_dtor(&list);
    return EXIT_FAILURE;
}
