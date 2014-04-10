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

typedef struct
{
    unsigned char type;
    dynset *kvs;
} message;


typedef struct
{
    void *prev;
    void *next;
    void *curr;
} fragment_item;

typedef struct
{
    fragment_item *head;
    fragment_item *tail;
} fragment_sequence;

#endif
