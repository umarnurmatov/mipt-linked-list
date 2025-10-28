#include "dllist.h"

int main()
{
    DLLIST_MAKE(list);
    
    // dllist_ctor(&list, 10);
    //
    // dllist_insert_after(&list, 10, 0);
    // dllist_insert_after(&list, 30, 1);
    // dllist_insert_after(&list, 20, 2);
    // dllist_insert_after(&list, 15, 1);
    //
    // dllist_delete_at(&list, 1);
    //
    // dllist_insert_after(&list, 10, 0);
    //
    // dllist_delete_at(&list, 4);


    dllist_ctor(&list, 2);

    dllist_insert_after(&list, 10, 0);
    dllist_insert_after(&list, 10, 0);
    dllist_insert_after(&list, 10, 0);
    dllist_insert_after(&list, 10, 0);

    dllist_dtor(&list);

    return EXIT_SUCCESS;
}
