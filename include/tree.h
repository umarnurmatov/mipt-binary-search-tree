#pragma once

#include <stdlib.h>

typedef int tree_data_t;

typedef enum tree_err_t
{
    TREE_ERR_NONE,
    TREE_NULLPTR,
    TREE_ALLOC_FAIL
} tree_err_t;

typedef struct tree_node_t
{
    tree_data_t data;
    tree_node_t* left;
    tree_node_t* right;
} tree_node_t;

typedef struct tree_t
{
    tree_node_t* root;
    ssize_t size;
} tree_t;

tree_err_t tree_insert(tree_t* tree, tree_data_t data);

const char* tree_strerr(tree_err_t err);

void tree_dtor(tree_t* tree);
