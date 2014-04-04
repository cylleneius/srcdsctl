#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dynset.h"

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

message *new_message( )
{
    //
    // Let's make a (empty) message!
    message *m   = (message*)malloc( sizeof( message ) );
    m->type      = 0;
    m->kvs     = new_dynset( );
    //
    //
    return m;
}

#define MAX_PACKET_SIZE 30
#define PACKET_CONTROL_BYTES 8 // PPPPOSLT T=< or $
#define MAX_PAYLOAD_SIZE MAX_PACKET_SIZE - (PACKET_CONTROL_BYTES + 1) // endbyte

typedef struct
{
    union
    {
        unsigned char buffer[ MAX_PACKET_SIZE ];
        struct
        {
            union
            {
                unsigned char bytes[4];
                unsigned int integer;
            } pid;
            unsigned char order;
            unsigned char s;
            unsigned char l;
            unsigned char startbyte;
            unsigned char payload[ MAX_PAYLOAD_SIZE ];
        } header;
    } data;
    
    unsigned int offset;
    //
} packet;

packet *new_packet( )
{
    //
    // Let's make a (empty) packet!
    packet *p      = (packet*)malloc( sizeof( packet ) );
    p->data.header.pid.integer = getpid( );
    p->data.header.order       = 0;
    p->offset  = 0;
    //
    //
    return p;
}

unsigned char msgtest0[] = "xxxx0<1keyone|valueone|keytwo|valuetwo|>";
unsigned char msgtest1[] = "xxxx1<1keyone|valu@";
unsigned char msgtest2[] = "xxxx1$eone|keytwo|valuetwo|>";


#define MESSAGE_START     '<'
#define MESSAGE_CONTINUE  '@'
#define MESSAGE_RESUME    '$'
#define MESSAGE_END       '>'
#define DELIMITER         '|'



//Write to dest buffer from src buffer starting at offset, return length written
unsigned int write_to_buffer( unsigned char *dest, unsigned int offset,
                                                   unsigned int d_len,
                              unsigned char *src , unsigned int s_len )
{
    if( s_len + offset < d_len )
    {
        memcpy( dest + offset, src, s_len );
        memcpy( dest + offset+s_len-1, "|", 1 );
        return s_len;
    }
    else
    {
        unsigned int diff = s_len + offset - d_len;
        unsigned int s_offset = s_len - diff;
        memcpy( dest + offset, src, s_offset );
        
        return s_offset;
    }
    return 0;
}

packet *packet_add( dynset *s, packet *p, unsigned char *str, unsigned int len )
{
    unsigned int wr;
    wr = write_to_buffer( p->data.header.payload,p->offset,MAX_PAYLOAD_SIZE,str,len );
    p->offset += wr;
    if( wr != len )
    {
        p->data.header.payload[ p->offset ] = MESSAGE_CONTINUE;
        //
        //
        packet *q = new_packet( );
        q->data.header.order = p->data.header.order + 1;
        q->data.header.startbyte = MESSAGE_RESUME;
        dynset_add( s, q );
        
        q = packet_add( s, q, str+wr, len-wr );
        return q;
    }
    
    return p;
}

dynset *stuff_message( message *m )
{
    dynset *s = new_dynset( );
    if( m )
    {
        int num_packets = 0;
        int total_length = 0;
        
        int j;
        
        packet *p = NULL;
        
        for( j=0; j < m->kvs->len; j++ )
        {
            item *kv = (item*)m->kvs->items[ j ];
            if( kv != NULL )
            {
                if( kv->k != NULL && kv->v != NULL )
                {
                    if( p == NULL )
                    {
                        //
                        //
                        p = new_packet( );
                        p->data.header.order = s->len;
                        p->data.header.startbyte = MESSAGE_START;
                        dynset_add( s, p );
                        //
                        //
                    }
                    
                    p = packet_add( s, p, kv->k, kv->k_len );
                    p = packet_add( s, p, kv->v, kv->v_len );
                }
            }
        }
        
        p->data.header.payload[ p->offset ] = MESSAGE_END;
    }
    return s;
}

unsigned char key_0[ ] = "keyone";
unsigned char key_1[ ] = "keytwo";
unsigned char key_2[ ] = "keythree";
unsigned char key_3[ ] = "keyfour";

unsigned char value_0[ ] = "valueone";
unsigned char value_1[ ] = "valuetwo";
unsigned char value_2[ ] = "valuethree";
unsigned char value_3[ ] = "valuefour";

void main( )
{
    item *i_0 = new_item( );
    item *i_1 = new_item( );
    item *i_2 = new_item( );
    item *i_3 = new_item( );
    
    i_0->k = key_0;
    i_0->k_len = sizeof( key_0 );
    i_1->k = key_1;
    i_1->k_len = sizeof( key_1 );
    i_2->k = key_2;
    i_2->k_len = sizeof( key_2 );
    i_3->k = key_3;
    i_3->k_len = sizeof( key_3 );

    i_0->v = value_0;
    i_0->v_len = sizeof( value_0 );
    i_1->v = value_1;
    i_1->v_len = sizeof( value_1 );
    i_2->v = value_2;
    i_2->v_len = sizeof( value_2 );
    i_3->v = value_3;
    i_3->v_len = sizeof( value_3 );
    
    //packet *p = new_packet( );
    message *m = new_message( );
    
    dynset_add( m->kvs, i_0 );
    dynset_add( m->kvs, i_1 );
    dynset_add( m->kvs, i_2 );
    dynset_add( m->kvs, i_3 );
    //We have a message we want to send, but we only send packets.
    dynset *s = stuff_message( m );
    
    
    
    
    printf("%i\n", s->len);
    int l;
    for( l=0; l < s->len;l++ )
    {
        packet *pb = (packet*)s->items[l];
        printf( "Packet: " );
        int m;
        for(m=0; m < MAX_PACKET_SIZE; m++ )
        {
            if( pb->data.buffer[m] > 31 && pb->data.buffer[m] < 127 )
                printf("%c", pb->data.buffer[m]);
            else
                printf("%x", pb->data.buffer[m]);
        }
        printf("\n");
    }
    
    //
    //
    unsigned char *msg = (unsigned char*)malloc( s->len * MAX_PAYLOAD_SIZE );
    unsigned int offset= 0;
    for( l=0; l < s->len;l++ )
    {
        packet *pb = (packet*)s->items[l];
        memcpy( msg + offset, pb->data.header.payload, MAX_PAYLOAD_SIZE );
        offset += MAX_PAYLOAD_SIZE;
    }
    printf( "message buffer: " );
    
    for(l=0; l < s->len * MAX_PAYLOAD_SIZE; l++ )
    {
        if( msg[l] > 31 && msg[l] < 127 )
            printf("%c", msg[l]);
        else
            printf("%x", msg[l]);
    }
    printf("\n");
    //We get a packet, extract the first five bytes, then parse the remaining.
    
    //We'll want to check if the last byte is a continue, if it is we'll wait
    //for the other part(s).
    
    //We'll check the first byte, in case it's a resume. If it is we'll match
    //it up with any continue we have pending.
    
    int i = 0;
    int j;
    for( j = i; j < s->len * MAX_PAYLOAD_SIZE; j++ )
    {
        if( msg[ j ] == DELIMITER )
        {
            item *t = (item*)malloc( sizeof( item ) );
            
            int len = j - i;
            unsigned char *tmp = (unsigned char*)malloc( len+1 );
            memcpy( tmp, msg+i, len );
            tmp[ len ] = '\0';
            printf( "%s\n", tmp );
            
            i = j+1;
        }
    }
}
