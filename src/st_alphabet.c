/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Wang Jian
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>

#include "st_utils.h"
#include "st_log.h"
#include "st_io.h"
#include "st_alphabet.h"

void get_sign(const char *psrc, int slen, st_dict_sign_t *sign1, st_dict_sign_t *sign2)
{
    *sign1 = 0;
    *sign2 = 0;
    if (slen <= 4) {
        memcpy(sign1, psrc, slen);
    } else {
        if (slen <= 8) {
            memcpy(sign1, psrc,4);
            memcpy(sign2, psrc + 4, slen - 4);
        } else {
            *sign1 = MurmurHash2(psrc, slen / 2, 1);
            *sign2 = MurmurHash2(psrc + slen / 2, slen - slen / 2, 2);
        }
    }
}

void st_alphabet_destroy(st_alphabet_t *alphabet)
{
    if (alphabet == NULL) {
        return;
    }

    safe_st_free(alphabet->labels);

    safe_st_dict_destroy(alphabet->index_dict);
}

static st_alphabet_t* st_alphabet_alloc()
{
    st_alphabet_t *alphabet;

    alphabet = (st_alphabet_t *)st_malloc(sizeof(st_alphabet_t));
    if (alphabet == NULL) {
        ST_ERROR("Failed to alloc alphabet.");
        return NULL;
    }

    alphabet->labels = NULL;
    alphabet->label_num = 0;
    alphabet->index_dict = NULL;

    return alphabet;
}

typedef struct _index_dict_eq_args_t_ {
    st_alphabet_t *alphabet;
    const char *label;
} index_dict_eq_args_t;

static bool index_dict_node_eq(st_dict_node_t *node1,
    st_dict_node_t *node2, void *args)
{
    index_dict_eq_args_t *arg;
    st_alphabet_t *alphabet;

    if (node1->sign1 != node2->sign1 || node1->sign2 != node2->sign2) {
        return false;
    }

    arg = (index_dict_eq_args_t *)args;
    alphabet = arg->alphabet;

    if (alphabet == NULL) {
        ST_ERROR("alphabet == NULL.");
        return false;
    }

    if (node1->uint1 >= alphabet->label_num) {
        ST_ERROR("node->uint1 overflow[%u/%u].", node1->uint1, alphabet->label_num);
        return false;
    }

    return (strncmp(alphabet->labels[node1->uint1].label, arg->label, MAX_SYM_LEN) == 0);
}

st_alphabet_t* st_alphabet_create(int max_label_num)
{
    st_alphabet_t *alphabet = NULL;
    int i;

    ST_CHECK_PARAM(max_label_num <= 0, NULL);

    alphabet = st_alphabet_alloc();
    if (alphabet == NULL) {
        ST_ERROR("Failed to alphabet_alloc.");
        goto ERR;
    }

    alphabet->max_label_num = max_label_num;
    alphabet->labels = (st_label_t *)
        st_malloc(max_label_num * sizeof(st_label_t));
    if (alphabet->labels == NULL) {
        ST_ERROR("Failed to allocate memory for labels.");
        goto ERR;
    }

    for(i = 0; i < max_label_num; i++) {
        alphabet->labels[i].symid = -1;
        alphabet->labels[i].label[0] = 0;
    }

    if ((alphabet->index_dict = st_dict_create((int)(max_label_num * 1.5),
        ST_DICT_REALLOC_NUM, NULL, index_dict_node_eq, false)) == NULL) {
        ST_ERROR("Failed to alloc index_dict");
        goto ERR;
    }

    return alphabet;

ERR:
    safe_st_alphabet_destroy(alphabet);
    return NULL;
}

int st_alphabet_add_label(st_alphabet_t *alphabet, const char *label_)
{
    st_dict_node_t snode;
    int ret = 0;

    ret = st_alphabet_get_index(alphabet, label_);
    if (ret >= 0) {
        return ret;
    }

    if (alphabet->max_label_num <= alphabet->label_num) {
        ST_ERROR("label overflow[%d/%d]", alphabet->label_num,
                alphabet->max_label_num);
        return -1;
    }

    alphabet->labels[alphabet->label_num].symid = alphabet->label_num;
    strncpy(alphabet->labels[alphabet->label_num].label, label_, MAX_SYM_LEN);
    alphabet->labels[alphabet->label_num].label[MAX_SYM_LEN - 1] = 0;

    get_sign((char *)label_, strlen(label_), &snode.sign1, &snode.sign2);
    snode.uint1 = alphabet->label_num;

    if (st_dict_add_no_seek(alphabet->index_dict, &snode) < 0) {
        ST_ERROR("Failed to add label[%s] into dict", label_);
        return -1;
    }

    alphabet->label_num++;
    return alphabet->label_num - 1;
}

int st_alphabet_get_label_num(st_alphabet_t *alphabet)
{
    ST_CHECK_PARAM(alphabet == NULL, -1);

    return alphabet->label_num;
}

char *st_alphabet_get_label(st_alphabet_t *alphabet, int index)
{
    ST_CHECK_PARAM_EX(alphabet == NULL || index < 0
        || index > alphabet->label_num, NULL, "%d/%d", index,
        alphabet->label_num);

    return alphabet->labels[index].label;
}

int st_alphabet_get_index(st_alphabet_t *alphabet, const char *label)
{
    index_dict_eq_args_t arg;
    st_dict_node_t snode;

    ST_CHECK_PARAM(alphabet == NULL || label == NULL, -1);

    arg.alphabet = alphabet;
    arg.label = label;
    get_sign((char *)label, strlen(label), &snode.sign1, &snode.sign2);
    if (st_dict_seek(alphabet->index_dict, &snode, &arg) < 0) {
        return -1;
    }

    return (int)snode.uint1;
}

int st_alphabet_save_bin(st_alphabet_t *alphabet, FILE *fp)
{
    int ret = 0;

    ST_CHECK_PARAM(alphabet == NULL || fp == NULL, -1);

    ret = fwrite(&alphabet->label_num, sizeof(int), 1, fp);
    if (ret != 1) {
        ST_ERROR("Failed to write label_num");
        return -1;
    }

    ret = fwrite(alphabet->labels, sizeof(st_label_t),
        alphabet->label_num, fp);
    if (ret != alphabet->label_num) {
        ST_ERROR("Failed to write labels");
        return -1;
    }

#ifdef _ST_ALPHABET_SAVE_DICT_
    if (st_dict_save(alphabet->index_dict, fp) < 0) {
        ST_ERROR("Failed to save index_dict");
        return -1;
    }
#endif

    return 0;
}

int st_alphabet_save_txt(st_alphabet_t *alphabet, FILE *fp)
{
    st_label_t *labels;
    int i;

    ST_CHECK_PARAM(alphabet == NULL || fp == NULL, -1);

    labels = alphabet->labels;
    for(i = 0; i < alphabet->label_num; i++) {
        if (labels[i].symid != -1) {
            if (fprintf(fp, "%s\t%d\n", labels[i].label,
                        labels[i].symid) < 0) {
                ST_ERROR("Failed to fprintf sym[%d]", i);
                return -1;
            }
        }
    }

    return 0;
}

static int st_alphabet_generate_index_dict(st_alphabet_t *alphabet)
{
    st_dict_node_t snode;
    st_dict_t *index_dict = NULL;
    int i;

    if ((index_dict = st_dict_create((int)(alphabet->label_num * 1.5),
        ST_DICT_REALLOC_NUM, NULL, NULL, false)) == NULL) {
        ST_ERROR("Failed to alloc index_dict");
        goto ERR;
    }

    for(i = 0; i < alphabet->label_num; i++) {
        get_sign(alphabet->labels[i].label, strlen(alphabet->labels[i].label),
                &snode.sign1, &snode.sign2);
        snode.uint1 = (uint)i;
        if (st_dict_add(index_dict, &snode, NULL) < 0) {
            ST_ERROR("Failed to st_dict_add.");
            goto ERR;
        }
    }

    alphabet->index_dict = index_dict;
    return 0;

ERR:
    safe_st_dict_destroy(index_dict);
    return -1;
}

int st_alphabet_load_txt(st_alphabet_t *alphabet, FILE *fp, int label_num)
{
    char line[MAX_LINE_LEN];
    char syms[MAX_SYM_LEN] ;
    int id;
    int i;

    st_label_t *labels;

    ST_CHECK_PARAM(alphabet == NULL || fp == NULL, -1);

    labels = (st_label_t *)st_malloc(label_num * sizeof(st_label_t));
    if (labels == NULL) {
        ST_ERROR("Failed to allocate memory for labels.");
        return -1;
    }

    for(i = 0; i < label_num; i++) {
        labels[i].symid = -1;
        labels[i].label[0] = 0;
    }

    i = 0;
    while(fgets(line, MAX_LINE_LEN, fp)) {
        if (sscanf(line, "%s %d", syms, &id) != 2) {
            continue;
        }

        if (id >= label_num) {
            ST_ERROR("Wrong id[%d]>=label_num[%d].", id, label_num);
            return -1;
        }

        if (labels[id].symid != -1) {
            ST_ERROR("Duplicated symbol [%d:%s].", id, syms);
            return -1;
        }

        strncpy(labels[id].label, syms, MAX_SYM_LEN);
        labels[id].label[MAX_SYM_LEN - 1] = 0;
        labels[id].symid = id;

        i++;
        if (i >= label_num) {
            break;
        }
    }

    for(i = 0; i < label_num; i++) {
        if (labels[i].symid == -1) {
            ST_ERROR("Empty symbol for id[%d]", i);
            return -1;
        }
    }

    alphabet->labels = labels;
    alphabet->max_label_num = label_num;
    alphabet->label_num = label_num;

    if (st_alphabet_generate_index_dict(alphabet) < 0) {
        ST_ERROR("Failed to st_alphabet_generate_index_dict.");
        return -1;
    }

    return 0;

}

st_alphabet_t* st_alphabet_load_from_txt(FILE *fp, int label_num)
{
    st_alphabet_t *alphabet = NULL;

    ST_CHECK_PARAM(fp == NULL, NULL);

    if ((alphabet = st_alphabet_alloc()) == NULL) {
        ST_ERROR("Failed to st_alphabet_alloc.");
        return NULL;
    }

    if (st_alphabet_load_txt(alphabet, fp, label_num) < 0) {
        ST_ERROR("Failed to st_alphabet_load_txt.");
        goto ERR;

    }

    return alphabet;
ERR:
    safe_st_alphabet_destroy(alphabet);
    return NULL;
}

st_alphabet_t* st_alphabet_load_from_txt_file(const char *file)
{
    char line[MAX_LINE_LEN];
    st_alphabet_t *alphabet = NULL;
    FILE *fp = NULL;
    int label_num;

    ST_CHECK_PARAM(file == NULL, NULL);

    fp = st_fopen(file, "r");
    if (fp == NULL) {
        ST_ERROR("Failed to open file[%s]", file);
        goto ERR;
    }

    label_num = 0;
    while(fgets(line, MAX_LINE_LEN, fp) != NULL) {
        ++label_num;
    }

    if (label_num <= 0) {
        goto ERR;
    }

    rewind(fp);

    alphabet = st_alphabet_load_from_txt(fp, label_num);
    if (alphabet == NULL) {
        ST_ERROR("Failed to st_alphabet_load_from_txt.");
        goto ERR;
    }

    safe_st_fclose(fp);
    return alphabet;
ERR:
    safe_st_fclose(fp);
    safe_st_alphabet_destroy(alphabet);
    return NULL;
}

static int st_alphabet_load_bin(st_alphabet_t *alphabet, FILE *fp)
{
    int ret = 0;

    ST_CHECK_PARAM(alphabet == NULL || fp == NULL, -1);

    ret = fread(&alphabet->label_num, sizeof(int), 1, fp);
    if (ret != 1) {
        ST_ERROR("Failed to read label_num");
        return -1;
    }
    alphabet->max_label_num = alphabet->label_num;

    alphabet->labels = (st_label_t *)
        st_malloc(sizeof(st_label_t)*alphabet->label_num);
    if (alphabet->labels == NULL) {
        ST_ERROR("Failed to st_malloc labels. [%d]", alphabet->label_num);
        return -1;
    }

    ret = fread(alphabet->labels, sizeof(st_label_t), alphabet->label_num, fp);
    if (ret != alphabet->label_num) {
        ST_ERROR("Failed to read labels");
        return -1;
    }

#ifdef _ST_ALPHABET_SAVE_DICT_
    if ((alphabet->index_dict = st_dict_load_from_bin(fp)) == NULL) {
        ST_ERROR("Failed to load index_dict");
        return -1;
    }
#else
    if (st_alphabet_generate_index_dict(alphabet) < 0) {
        ST_ERROR("Failed to st_alphabet_generate_index_dict.");
        return -1;
    }
#endif

    return 0;
}

st_alphabet_t* st_alphabet_load_from_bin(FILE *fp)
{
    st_alphabet_t *alphabet;

    ST_CHECK_PARAM(fp == NULL, NULL);

    if ((alphabet = st_alphabet_alloc()) == NULL) {
        ST_ERROR("Failed to st_alphabet_alloc.");
        return NULL;
    }

    if (st_alphabet_load_bin(alphabet, fp) < 0) {
        ST_ERROR("Failed to st_alphabet_load_bin.");
        goto ERR;

    }

    return alphabet;
ERR:
    safe_st_alphabet_destroy(alphabet);
    return NULL;
}

st_alphabet_t* st_alphabet_dup(st_alphabet_t *a)
{
    st_alphabet_t *alphabet = NULL;

    ST_CHECK_PARAM(a == NULL, NULL);

    alphabet = st_alphabet_alloc();
    if (alphabet == NULL) {
        ST_ERROR("Failed to alphabet_alloc.");
        goto ERR;
    }

    alphabet->max_label_num = a->max_label_num;
    alphabet->label_num = a->label_num;

    alphabet->labels = (st_label_t *)
        st_malloc(a->max_label_num * sizeof(st_label_t));
    if (alphabet->labels == NULL) {
        ST_ERROR("Failed to allocate memory for labels.");
        goto ERR;
    }

    memcpy(alphabet->labels, a->labels,
            sizeof(st_label_t)*a->max_label_num);

    alphabet->index_dict = st_dict_dup(a->index_dict);
    if (alphabet->index_dict == NULL) {
        ST_ERROR("Failed to st_dict_dup.");
        goto ERR;
    }

    return alphabet;

ERR:
    safe_st_alphabet_destroy(alphabet);
    return NULL;
}
