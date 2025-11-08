#include "bst.h"
#include "optutils.h"
#include "utils.h"
#include "logutils.h"
#include <cstring>

static utils_long_opt_t long_opts[] = 
{
    { OPT_ARG_REQUIRED, "log", NULL, 0, 0 },
};

int cmp_func(bst_data_t* a, bst_data_t* b)
{
    return a >= b;
}

int main(int argc, char* argv[])
{
    utils_long_opt_get(argc, argv, long_opts, SIZEOF(long_opts));

    utils_init_log_file(long_opts[0].arg, "log");

    bst_t bst = {
        .root = NULL,
        .cmp_func = NULL,
        .size = 0,
    };

    bst_err_t err = BST_ERR_NONE;

    bst_ctor(&bst, (bst_cmp_f)cmp_func);

    bst_insert(&bst, 1);
    

    bst_dtor(&bst);

    utils_end_log();

    return EXIT_SUCCESS;
}
