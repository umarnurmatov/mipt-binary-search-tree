#pragma once

#include <stdlib.h>

struct bst_node_t;
struct bst_t;

typedef int bst_data_t;

typedef int (*bst_cmp_f) (bst_data_t*, bst_data_t*);

typedef enum bst_err_t
{
    BST_ERR_NONE,
    BST_NULLPTR,
    BST_FIELD_NULL,
    BST_ALLOC_FAIL
} bst_err_t;

typedef struct bst_node_t
{
    bst_data_t data;
    bst_node_t* left;
    bst_node_t* right;
} bst_node_t;

typedef struct bst_t
{
    bst_node_t* root;
    bst_cmp_f cmp_func;
    size_t size;
} bst_t;

bst_err_t bst_ctor(bst_t* bst, bst_cmp_f cmp_func);

bst_err_t bst_insert(bst_t* bst, bst_data_t data);

const char* bst_strerr(bst_err_t err);

void bst_dtor(bst_t* bst);
