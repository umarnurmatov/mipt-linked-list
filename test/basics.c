#include "dllist.h"
#include "utils.h"
#include <cstdlib>

int main()
{
    DLLIST_MAKE(list);
    
#define DLLIST_VERIFY(expr) if(expr != DLLIST_NONE) GOTO_END;

    BEGIN {
        DLLIST_VERIFY(dllist_ctor(&list, 2));

        DLLIST_VERIFY(dllist_insert_after(&list, 10, 0));
        DLLIST_VERIFY(dllist_insert_after(&list, 10, 0));

        list.next[1] = 1000;

        DLLIST_VERIFY(dllist_insert_after(&list, 10, 0));

        dllist_dtor(&list);

        return EXIT_SUCCESS;
    } END;
    
    dllist_dtor(&list);
    return EXIT_FAILURE;
}
