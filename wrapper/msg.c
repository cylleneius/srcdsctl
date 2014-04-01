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

typedef struct
{
    union
    {
        unsigned char bytes[4];
        unsigned int integer;
    } pid;
    unsigned char order;
    
    unsigned char *buffer;
} packet;

packet *new_packet( )
{
    //
    // Let's make a (empty) packet!
    packet *p      = (packet*)malloc( sizeof( packet ) );
    p->pid.integer = getpid( );
    p->order       = 0;
    p->buffer      = NULL;
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

#define MAX_PACKET_SIZE 50

//Write to dest buffer from src buffer starting at offset, return length written
unsigned int write_to_buffer( unsigned char *dest, unsigned int offset,
                                                   unsigned int d_len,
                              unsigned char *src , unsigned int s_len )
{
    if( s_len + offset < d_len )
    {
        memcpy( dest + offset, src, s_len );
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


dynset *stuff_message( message *m )
{
    dynset *s = new_dynset( );
    if( m )
    {
        int num_packets = 0;
        int total_length = 0;
        
        //First we determine length
        int i;
        for( i=0; i < m->kvs->len; i++ )
        {
            //Since dynset holds voids we have to cast it.
            item *kv = (item*)m->kvs->items[ i ];
            printf("%x %x %x %i %i\n", kv,kv->k, kv->v, kv->k_len, kv->v_len );
            total_length += kv->k_len + kv->v_len + 2;
        }
        
        //Checking if we can just treat our message as one packet.
        int test_total = total_length + 8;
        printf("Test total: %i\n", test_total );
        if( test_total > MAX_PACKET_SIZE )
        {
            int max = MAX_PACKET_SIZE - 8;
            int n = total_length / max;
            int nr = total_length % max;
            printf("n: %i nr: %i\n", n, nr );
            
            //Here n is the number of packets we need. (if nr is 0)
            //Otherwise the number of packets we need is n + 1
            int j;
            
            packet *p = NULL;
            int b_offset = 0;
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
                            p->buffer = malloc( max );
                            dynset_add( s, p );
                            //
                            //
                        }
                        
                        unsigned int wr =0;
                        //
                        //
                        //
                        wr = write_to_buffer( p->buffer, b_offset, max,
                                              kv->k, kv->k_len );
                        b_offset += wr;
                        if( wr != kv->k_len )
                        {
                            //
                            //
                            b_offset = 0;
                            p = new_packet( );
                            p->buffer = malloc( max );
                            dynset_add( s, p );
                            //
                            //
                            wr = write_to_buffer( p->buffer, b_offset, max,
                                                  kv->k+wr, kv->k_len-wr );
                            b_offset += wr;
                        }
                        //
                        //
                        //
                        wr = write_to_buffer( p->buffer, b_offset, max,
                                              kv->v, kv->v_len );
                        b_offset += wr;
                        if( wr != kv->v_len )
                        {
                            //
                            //
                            b_offset = 0;
                            p = new_packet( );
                            p->buffer = malloc( max );
                            dynset_add( s, p );
                            //
                            //
                            wr = write_to_buffer( p->buffer, b_offset, max,
                                                  kv->v+wr, kv->v_len-wr );
                            b_offset += wr;
                        }
                        //
                        //
                        //

                    }
                }
            }
            printf("%i\n", s->len);
            int i;
            for( i=0; i < s->len;i++ )
            {
                packet *pb = (packet*)s->items[i];
                printf( "Packet: " );
                int m;
                for(m=0; m < MAX_PACKET_SIZE; m++ )
                {
                    if( pb->buffer[m] >
                    printf("%c", pb->buffer[m]);
                }
                printf("\n");
            }
        }
    }
    return NULL;
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
    
    //We get a packet, extract the first five bytes, then parse the remaining.
    
    //We'll want to check if the last byte is a continue, if it is we'll wait
    //for the other part(s).
    
    //We'll check the first byte, in case it's a resume. If it is we'll match
    //it up with any continue we have pending.
    
    //At this point our message buffer will look like this: (from msgtest)
    //1keyone|valueone|keytwo|valuetwo|
    //And, after extracting the type byte, is ready to be parsed:
    //keyone|valueone|keytwo|valuetwo|
    
    int i = 7;
    int j;
    for( j = i; j < sizeof( msgtest0 ); j++ )
    {
        if( msgtest0[ j ] == DELIMITER )
        {
            item *t = (item*)malloc( sizeof( item ) );
            
            int len = j - i;
            unsigned char *tmp = (unsigned char*)malloc( len );
            strncpy( tmp, msgtest0+i, len );
            
            printf( "%s\n", tmp );
            
            i = j+1;
        }
    }

}
