#ifndef __MSG_H__
#define __MSG_H__

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dynset.h"
#include "packet.h"

typedef struct
{
    unsigned char *k;
    unsigned int k_len;
    
    unsigned char *v;
    unsigned int v_len;
} item;

item *new_item( )
{
    item *i  = (item*)malloc( sizeof( item ) );
    i->k     = NULL;
    i->k_len = 0;
    i->v     = NULL;
    i->v_len = 0;
    return i;
}

typedef struct
{
    unsigned char type;
    dynset *kvs;
} message;

#endif
