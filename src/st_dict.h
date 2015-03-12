#ifndef _ST_DICT_H_
#define _ST_DICT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "st_macro.h"

#define ST_DICT_REALLOC_NUM 1000000

typedef unsigned int st_dict_sign_t;

typedef unsigned int st_dict_id_t;
#define ST_DICT_BAD_NODE            (st_dict_id_t)-1

typedef struct _st_dict_node_t
{
    st_dict_sign_t sign1;
    st_dict_sign_t sign2;
    union
    {
        unsigned int uint1;
        float        float1;
    };
    st_dict_id_t   next;
} st_dict_node_t;

struct _st_dict_t;
typedef st_dict_id_t (*st_dict_hash_fun_t)(struct _st_dict_t *,
    st_dict_node_t *);

typedef bool (*st_dict_node_eq_fun_t)(st_dict_node_t *node1,
    st_dict_node_t *node2, void *args);
typedef int (*st_dict_update_func_t)(st_dict_node_t *node, float data);
typedef int (*st_dict_trav_func_t)(st_dict_node_t *p, void *arg);

typedef struct _st_dict_t
{
    st_dict_node_t     *first_level_node;
    st_dict_id_t       hash_num;
    st_dict_id_t       realloc_node_num;
    
    st_dict_node_t     *node_pool;
    st_dict_id_t       cur_index;
    st_dict_id_t       max_pool_num;

    st_dict_id_t       node_num;

    st_dict_id_t       addr_mask;
    st_dict_hash_fun_t hash_func;
    st_dict_node_eq_fun_t node_eq_func;

    st_dict_id_t       *clear_nodes;
    st_dict_id_t       clear_node_num;
} st_dict_t;

st_dict_t* st_dict_create(st_dict_id_t hash_num,
    st_dict_id_t realloc_node_num, st_dict_hash_fun_t hash_func,
    st_dict_node_eq_fun_t node_eq_func, bool need_clear);

#define safe_st_dict_destroy(ptr) do {\
    if(ptr != NULL) {\
        st_dict_destroy(ptr);\
        safe_free(ptr);\
        ptr = NULL;\
    }\
    } while(0)
void st_dict_destroy(st_dict_t *wd);

int st_dict_add(st_dict_t *wd, st_dict_node_t *pnode, void *node_eq_arg);
int st_dict_add_no_seek(st_dict_t *wd, st_dict_node_t *pnode);
int st_dict_seek(st_dict_t *wd, st_dict_node_t *pnode, void *node_eq_arg);

int st_dict_traverse(st_dict_t *wd, st_dict_trav_func_t trav, void *args);
int st_dict_clear(st_dict_t *wd, st_dict_trav_func_t trav, void *args);

int st_dict_update(st_dict_t *wd, st_dict_node_t *pnode, void *node_eq_arg,
        st_dict_update_func_t update_data);

int st_dict_save(st_dict_t *wd, FILE *fp);

st_dict_t* st_dict_load_from_bin(FILE *fp);

st_dict_id_t st_dict_hash_simple(st_dict_t *wd, st_dict_node_t *pnode);
st_dict_id_t st_dict_hash_sign1l16(st_dict_t *wd, st_dict_node_t *pnode);
st_dict_id_t st_dict_hash_sign1(st_dict_t *wd, st_dict_node_t *pnode);

#ifdef __cplusplus
}
#endif

#endif
