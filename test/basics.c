#include "dllist.h"
#include "logutils.h"

int main()
{
    utils_init_log_stream(stderr);

    DLLIST_MAKE(list);
    
    dllist_ctor(&list, 10);

    dllist_insert(&list, 10, 0);

    dllist_dtor(&list);

    utils_end_log();

    return EXIT_SUCCESS;
}
