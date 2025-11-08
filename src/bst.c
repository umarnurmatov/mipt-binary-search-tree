#include "bst.h"

#include <cstdlib>
#include <string.h>
#include <time.h>

#include "logutils.h"
#include "memutils.h"
#include "ioutils.h"

#ifdef _DEBUG

#define BST_DUMP_(bst, err) \
    bst_dump_(bst, err, NULL, __FILE__, __LINE__, __func__); 

#define BST_DUMP_MSG_(bst, err, msg) \
    bst_dump_(bst, err, msg, __FILE__, __LINE__, __func__); 

#define BST_ASSERT_OK_(dllist)                   \
    {                                            \
        bst_err_t err = bst_verify_(bst);        \
        if(err != BST_ERR_NONE) {                \
            BST_DUMP_(bst, err);                 \
            utils_assert(err == BST_ERR_NONE);   \
        }                                        \
    }

#define BST_VERIFY_OR_RETURN_(bst, err)    \
    if(err != BST_ERR_NONE) {              \
        BST_DUMP_(bst, err);               \
        return err;                        \
    }

#else // _DEBUG

// FIXME

#endif // _DEBUG

bst_err_t bst_allocate_new_(bst_node_t** ptr);

void bst_dump_(bst_t* bst, bst_err_t err, const char* msg, const char* file, int line, const char* funcname);

char* bst_dump_graphviz_(bst_t* bst);

int bst_dump_node_graphviz_(FILE* file, bst_node_t* node, int rank);

#ifdef _DEBUG

bst_err_t bst_verify_(bst_t* bst);

void bst_node_dtor_(bst_node_t* node);

#endif // _DEBUG

bst_err_t bst_ctor(bst_t* bst, bst_cmp_f cmp_func)
{
    utils_assert(bst);
    utils_assert(cmp_func);

    bst->cmp_func = cmp_func;
    bst->size = 0;

    return BST_ERR_NONE;
}

bst_err_t bst_insert(bst_t* bst, bst_data_t data)
{
    BST_ASSERT_OK_(bst);

    bst_err_t err = BST_ERR_NONE;

    bst_node_t* parent = bst->root;
    bst_node_t* child = bst->root;

    while(child != NULL) {
        parent = child;

        if(bst->cmp_func(&child->data, &parent->data) >= 0)
            child = parent->left;
        else
            child = parent->right;
    }
    
    bst_node_t* new_elem;
    err = bst_allocate_new_(&new_elem);
    BST_VERIFY_OR_RETURN_(bst, err);
    
    new_elem->data = data;
    
    if(!parent) {
        bst->root = new_elem;
    }
    else {
        if(bst->cmp_func(&child->data, &parent->data) >= 0)
            parent->left = new_elem;
        else
            parent->right = new_elem;
    }

    ++bst->size;

    BST_DUMP_(bst, err);

    return BST_ERR_NONE;
}


const char* bst_strerr(bst_err_t err)
{
    switch(err) {
        case BST_ERR_NONE:
            return "none";
        case BST_NULLPTR:
            return "passed a nullptr";
        case BST_ALLOC_FAIL:
            return "memory allocation failed";
        default:
            return "unknown";
    }
}

void bst_dtor(bst_t* bst)
{
    utils_assert(bst);

    bst_node_dtor_(bst->root); 
}

bst_err_t bst_allocate_new_(bst_node_t** ptr)
{
    utils_assert(ptr);

    bst_node_t* ptr_tmp = (bst_node_t*)calloc(1, sizeof(ptr_tmp[0]));

    if(!ptr_tmp)
        return BST_ALLOC_FAIL;

    *ptr = ptr_tmp;

    return BST_ERR_NONE;
}

void bst_node_dtor_(bst_node_t* node)
{
    utils_assert(node);

    if(node->left)
        bst_node_dtor_(node->left);
    if(node->right)
        bst_node_dtor_(node->right);

    NFREE(node);
}

#ifdef _DEBUG

#define GRAPHVIZ_FNAME_ "graphviz"
#define GRAPHVIZ_CMD_LEN_ 100

#define CLR_RED_LIGHT_   "\"#FFB0B0\""
#define CLR_GREEN_LIGHT_ "\"#B0FFB0\""
#define CLR_BLUE_LIGHT_  "\"#B0B0FF\""

#define CLR_RED_BOLD_    "\"#FF0000\""
#define CLR_GREEN_BOLD_  "\"#03c03c\""
#define CLR_BLUE_BOLD_   "\"#0000FF\""

void bst_dump_(bst_t* bst, bst_err_t err, const char* msg, const char* filename, int line, const char* funcname)
{
    utils_log_fprintf(
        "<style>"
        "table {"
          "border-collapse: collapse;"
          "border: 1px solid;"
          "font-size: 0.9em;"
        "}"
        "th,"
        "td {"
          "border: 1px solid rgb(160 160 160);"
          "padding: 8px 10px;"
        "}"
        "</style>\n"
    );

    utils_log_fprintf("<pre>\n"); 

    time_t cur_time = time(NULL);
    struct tm* iso_time = localtime(&cur_time);
    char time_buff[100];
    strftime(time_buff, sizeof(time_buff), "%F %T", iso_time);

    if(err != BST_ERR_NONE) {
        utils_log_fprintf("<h3 style=\"color:red;\">[ERROR] [%s] from %s:%d: %s() </h3>", time_buff, filename, line, funcname);
        utils_log_fprintf("<h4><font color=\"red\">err: %s </font></h4>", bst_strerr(err));
    }
    else
        utils_log_fprintf("<h3>[DEBUG] [%s] from %s:%d: %s() </h3>\n", time_buff, filename, line, funcname);

    if(msg)
        utils_log_fprintf("what: %s\n", msg);

    char* img_pref = bst_dump_graphviz_(bst);

    utils_log_fprintf(
        "\n<img src=" IMG_DIR "/%s.svg width=50%%\n", 
        strrchr(img_pref, '/') + 1
    );

    utils_log_fprintf("</pre>\n"); 

    utils_log_fprintf("<hr color=\"black\" />\n");

    NFREE(img_pref);
}

char* bst_dump_graphviz_(bst_t* bst)
{

    FILE* file = open_file(LOG_DIR "/" GRAPHVIZ_FNAME_ ".txt", "w");
    if(!file)
        exit(EXIT_FAILURE);

    fprintf(file, "digraph {\n rankdir=TB;\n"); 
    fprintf(file, "nodesep=0.9;\nranksep=0.75;\n");

    bst_dump_node_graphviz_(file, bst->root, 1);

    fprintf(file, "};");

    fclose(file);
    
    create_dir(LOG_DIR "/" IMG_DIR);
    char* img_tmpnam = tempnam(LOG_DIR "/" IMG_DIR, "img-");
    utils_assert(img_tmpnam);

    static char strbuf[GRAPHVIZ_CMD_LEN_]= "";

    sprintf(
        strbuf, 
        "dot -T svg -o %s.svg " LOG_DIR "/" GRAPHVIZ_FNAME_ ".txt", 
        img_tmpnam
    );

    system(strbuf);

    return img_tmpnam;
}

int bst_dump_node_graphviz_(FILE* file, bst_node_t* node, int rank)
{
    utils_assert(file);
    utils_assert(node);

    static int node_cnt = 0;

    if(rank == 1)
        node_cnt = 0;

    int node_cnt_left = 0, node_cnt_right = 0;
    
    if(node->left)
        node_cnt_left = bst_dump_node_graphviz_(file, node->left, rank + 1);
    if(node->right)
        node_cnt_right = bst_dump_node_graphviz_(file, node->right, rank + 1);
    
    fprintf(
        file, 
        "node_%d["
        "shape=record,"
        "label=\" { addr: %p | data: %d | { L: %p | R: %p } } \","
        "rank=%d"
        "];\n",
        node_cnt,
        node,
        node->data,
        node->left,
        node->right,
        rank
    );

    if(node->left)
        fprintf(
            file,
            "node_%d -> node_%d;\n",
            node_cnt, 
            node_cnt_left
        );

    if(node->right)
        fprintf(
            file,
            "node_%d -> node_%d;\n",
            node_cnt, 
            node_cnt_right
        );

    return node_cnt++;
}

bst_err_t bst_verify_(bst_t* bst)
{
    if(!bst)
        return BST_NULLPTR;

    if(!bst->cmp_func)
        return BST_FIELD_NULL;

    
    return BST_ERR_NONE;
}


#endif // _DEBUG
