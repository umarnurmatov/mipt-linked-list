
#include <cstdlib>
#include <stdio.h>
#include <time.h>

#include "dllist.h"
#include "utils.h"

int main()
{

    DLLIST_MAKE(list);

    const int ELEMENT_CNT = 50000000;
    const int DELETE_CNT = 5000000;
    const int LIST_INIT_SIZE = 10000;
    const int SEED = 31415;
    
#define DLLIST_VERIFY(expr) if(expr != DLLIST_NONE) GOTO_END;

    BEGIN {
        DLLIST_VERIFY(dllist_ctor(&list, LIST_INIT_SIZE, ""));

        for(size_t i = 0; i < ELEMENT_CNT; ++i)
            dllist_insert_after(&list, 0, 0);

        srand(SEED);

        unsigned char* deleted = (unsigned char*)calloc(ELEMENT_CNT, sizeof(unsigned char));

        int deleted_cnt = 0;
        while(deleted_cnt < DELETE_CNT) {
            int i = rand() % (ELEMENT_CNT - 1);
            if(!deleted[i] && i > 0) {
                deleted[i] = 1;
                dllist_delete_at(&list, i);
                deleted_cnt++;
            }
        }

        dllist_dtor(&list);

        return EXIT_SUCCESS;
    } END;

#undef DLLIST_VERIFY

    dllist_dtor(&list);
    return EXIT_FAILURE;
}
