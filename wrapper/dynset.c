#include "dynset.h"

dynset *new_dynset( )
{
    dynset *l = (dynset*)malloc( sizeof( dynset ) );
    l->del_item = NULL;
    l->items = NULL;
    l->len   = 0;
    l->max   = 0;
    return l;
}

void del_dynset( dynset *l )
{
    if( l )
    {
        if( l->len > 0 )
        {
            int i;
            for( i = 0; i < l->len; i++ )
            {
                if( l->del_item ) l->del_item( l->items[ i ] );
            }
        }
        free( l );
    }
}

int dynset_check( dynset *l, void *item )
{
    if( l )
    {
        //printf("%x %x %x\n", l, item, l->max );
        if( l->len == 0 ) return -1;
        int x;
        for( x = 0; x < l->len; x++ )
        {
            if( l->items[ x ] == item ) return x;
        }
    }
    return -1;
}

int dynset_add( dynset *l, void *item )
{
    if( l )
    {
        if( dynset_check( l, item ) == -1 )
        {
            l->len += 1;
            if( l->len > l->max )
            {
                l->max = l->len;
                l->items = (void **) realloc( l->items,
                           sizeof( void * ) * l->max );
            }
            if( l->items )
            {
                l->items[ l->len - 1 ] = item;
                // Success.
                return 0;
            }
            else
            {
                l->len -= 1;
                // Error: No memory.
                return -1;
            }
        }
        else
        {
            // Already here.
            return 1; 
        }
    }
    //Error: l or item is NULL
    return -1;
}

int dynset_del( dynset *l, void *item )
{
    if( l )
    {
        int i = dynset_check( l, item );
        if( i != -1 )
        {
            int x;
            for( x = i+1; x < l->len; x++ )
            {
                l->items[ x - 1 ] = l->items[ x ];
            }
            l->len -= 1;
        }
        return l->len;
    }
    return -1;
}

void dynset_clear( dynset *l )
{
    int x;
    for( x=0; x < l->len; x++ )
    {
        l->items[ x ] = NULL;
    }
    l->len = 0;
}
