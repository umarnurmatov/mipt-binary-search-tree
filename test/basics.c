#include "tree.h"
#include "optutils.h"
#include "utils.h"
#include "logutils.h"

static utils_long_opt_t long_opts[] = 
{
    { OPT_ARG_REQUIRED, "log", NULL, 0, 0 },
};

int main(int argc, char* argv[])
{
    utils_long_opt_get(argc, argv, long_opts, SIZEOF(long_opts));

    utils_init_log_file(long_opts[0].arg, "log");

    tree_t tree;

    tree_insert(&tree, 6);

    tree_insert(&tree, 2);

    tree_insert(&tree, 0);

    tree_insert(&tree, 4);

    tree_insert(&tree, 8);
    
    tree_insert(&tree, 5);

    tree_dtor(&tree);

    return EXIT_SUCCESS;
}
