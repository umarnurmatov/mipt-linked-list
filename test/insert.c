#include <stdio.h>
#include <time.h>

#include "dllist.h"
#include "utils.h"
#include "optutils.h"

int main(int argc, char* argv[])
{

    DLLIST_MAKE(list);

    const int LIST_SIZE = 50000000;
    const int LIST_INIT_SIZE = 10000;
    
#define DLLIST_VERIFY(expr) if(expr != DLLIST_NONE) GOTO_END;

    BEGIN {
        DLLIST_VERIFY(dllist_ctor(&list, LIST_INIT_SIZE, ""));

        for(size_t i = 0; i < LIST_SIZE; ++i)
            dllist_insert_after(&list, 0, 0);

        dllist_dtor(&list);

        return EXIT_SUCCESS;
    } END;

#undef DLLIST_VERIFY

    dllist_dtor(&list);
    return EXIT_FAILURE;
}
