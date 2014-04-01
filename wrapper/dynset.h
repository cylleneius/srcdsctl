#ifndef __DYNSET_H__
#define __DYNSET_H__
#include <strings.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
    void **items;
    unsigned int len;
    unsigned int max;
    
    int (*del_item)( void *item );
} dynset;

dynset *new_dynset( );
void del_dynset( dynset * );
void dynset_clear( dynset *l );

int dynset_add( dynset *l, void *item ); //-1 on error, 0 success, 1 dup: no add
int dynset_del( dynset *l, void *item ); //-1 on error, N new length
int dynset_check( dynset *l, void *item ); //-1 if not found

#endif
