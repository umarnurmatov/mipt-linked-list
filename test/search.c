#include <cstdlib>
#include <stdio.h>
#include <time.h>

#include "dllist.h"
#include "utils.h"

int main()
{

    DLLIST_MAKE(list);

    const int ELEMENT_CNT = 5000000;
    const int FIND_CNT = 500000;
    const int LIST_INIT_SIZE = 10000;
    const int SEED = 31415;
    
#define DLLIST_VERIFY(expr) if(expr != DLLIST_NONE) GOTO_END;

    BEGIN {
        DLLIST_VERIFY(dllist_ctor(&list, LIST_INIT_SIZE, ""));

        srand(SEED);

        for(ssize_t i = 0; i < ELEMENT_CNT; ++i)
            dllist_insert_after(&list, rand() % (list.size), (int)i);

        for(ssize_t i = 0; i < FIND_CNT; ++i) {
            int to_find = rand() % ELEMENT_CNT;

            for(ssize_t j = dllist_begin(&list); j < list.size; j = dllist_next(&list, j)) {
                if(list.data[j] == to_find)
                    break;
            }
        }

        dllist_dtor(&list);

        return EXIT_SUCCESS;
    } END;

#undef DLLIST_VERIFY

    dllist_dtor(&list);
    return EXIT_FAILURE;
}
