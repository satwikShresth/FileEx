#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dict.h"

#define DEBUG 1

static unsigned long _hash(const char *key)
{
    int c;

    unsigned long hash = 5381;

    while ( (c = *key++) )
        hash = hash * 33 + c;

    return hash;
}


#if DEBUG
static void dict_debug_bin_fill(struct dict *D)
{
    unsigned int idx;
    unsigned long int n;
    struct list *cur;

    for (idx=0, n=0; idx<D->nbins; idx++, n=0) {
        for(cur=D->bins[idx]; cur; cur=cur->next)
            n++;

        fprintf(stderr, "bin[%d]: %lu\n", idx, n);
    }

}
#endif


static int dict_rehash(struct dict *D)
{
    int idx;

    int new_idx;
    unsigned int new_nbins;
    struct list **new_bins;

    struct list *cur;

#if DEBUG
    /* print # of items in each bin */
    dict_debug_bin_fill(D);
    fprintf(stderr, "Rehashing...\n");
    /* pause for 3 seconds */ 
    sleep(3);
#endif

    new_nbins = 2 * D->nbins;
    new_bins = malloc(new_nbins*sizeof(*new_bins));
    if (!new_bins)return 0;
    for (new_idx=0; new_idx<D->nbins; new_idx++)new_bins[new_idx] = NULL;
    for (idx = 0;idx<D->nbins;idx++)
    {
      cur = D->bins[idx];
      for(;!(cur==NULL);cur=cur->next)
      {
        new_idx = cur->hash % new_nbins;
        new_bins[new_idx] = malloc(sizeof(*new_bins[new_idx]));
        new_bins[new_idx]->hash=cur->hash;
        new_bins[new_idx]->key=cur->key;
      }
    }

    free(D->bins);
    D->bins = new_bins;
    D->nbins = new_nbins;
    return 1;
}


int dict_init(struct dict *D, void (*deleter)(void* user_data))
{
    int i;

    D->nbins = 6;
    D->bins = malloc(D->nbins*sizeof(*D->bins));
    if (!D->bins)
        return 0;

    for (i=0; i<D->nbins; i++)
        D->bins[i] = NULL;

    D->deleter = deleter;
    D->count = 0; 

    return 1;
}


void dict_destroy(struct dict *D)
{
    unsigned int idx;
    struct list *cur;

    for (idx=0; idx<D->nbins; idx++) {
        while(D->bins[idx]) {
            cur = D->bins[idx];
            D->bins[idx] = cur->next;

            if (D->deleter)
                D->deleter(cur->user_data);

            free(cur->key);
            free(cur);
        }
    }

    free(D->bins);
}


int dict_insert(struct dict *D, const char *key, void *user_data)
{
    float load = (float)(D->count + 1) / D->nbins;
    unsigned int idx  = _hash(key) % D->nbins;
    struct list *item;

    if (load >= 0.75)
        if (!dict_rehash(D))
            return 0;

    item = malloc(sizeof(*item));

    if (!item) {
        fprintf(stderr, "Insert Failed\n");
        return 0;
    }

    item->key = malloc(strlen(key)+1);
    if (!item->key) {
        fprintf(stderr, "Insert Failed\n");
        free(item);
        return 0;
    }

    strcpy(item->key, key);

    item->hash = _hash(key);
    item->user_data = user_data;

    item->next = D->bins[idx];
    D->bins[idx] = item;
    D->count++;     /* this line is new */

    return 1;
}


void dict_delete(struct dict *D, const char *key)
{
    void *user_data;

    if (D->deleter && (user_data = dict_pop(D, key)))
        D->deleter(user_data);
}


void *dict_peek(struct dict *D, const char *key)
{
    struct list *cur;

    unsigned long hash = _hash(key);
    unsigned int idx = hash % D->nbins;

    for (cur=D->bins[idx]; cur; cur=cur->next)
        if (hash == cur->hash)
            if (!strcmp(cur->key, key))
                return cur->user_data;

    return NULL;
}


void *dict_pop(struct dict *D, const char *key)
{
    struct list *cur;
    void *user_data;

    unsigned long hash = _hash(key);
    unsigned int idx = hash % D->nbins;
    struct list **prev_next = &D->bins[idx];

    for (cur=D->bins[idx]; cur; cur=cur->next) {
        if (hash == cur->hash)
            if (!strcmp(cur->key, key)) {
                *prev_next = cur->next;
                user_data = cur->user_data;
                free(cur->key);
                free(cur);
                D->count--; 
                return user_data;
            }

        prev_next = &cur->next;
    }

    return NULL;
}


float dict_loadfactor(struct dict *D)
{
    return (float)D->count / D->nbins;
}