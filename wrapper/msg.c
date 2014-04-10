#include "msg.h"

dynset *fragments;
//The fragments could look like:
// f1: p1 p2 [] p4
// f2: [] p2 p3 []
// f3: p1 [] [] p4
// If msg had a count like the packets do, these fragments would be grouped by
// the msg count. Since they don't we'll group them by their immediate neighbors
// And also their pid. If two packets have two different pids, it's unlikely
// they would be part of the same message.
// 
// If a message, msg0 spanned two packets, p0, p1, and another, msg1, spanned
// the next three packets we might get a packet stream like:
// p1p3p2p0p4
// In which case the appropriate sorting would be iterative as new packets are
// received:
// t1:   f1: p1
// t2:   f1: p1
//       f2: p3
// t3:   f1: p1
//       f2: p3 p2
// t4:   f1: p1 p0 < No longer a fragment
//       f2: p3 p2
// t5:   f2: p3 p2 p4
// This would require being able to split fragments in cases like:
// t1:   f1: p0 p4
// We have a start packet, and an end packet, but no middle packets.
// If we then received p2 (msg1's start packet):
// t2:   f1: p0 p2 p4 < we need to split this
// t3:   f1: p0
//       f2: p2 p4
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

dynset *packet_sorter_add( packet *p )
{
    if( fragments == NULL ) fragments = new_dynset( );
    int matches = 0;
    int i;
    for( i=0; i < fragments->len; i++ )
    {
        dynset *s = (dynset*)fragments->items[ i ];
        int j;
        for( j=0; j < s->len; j++ )
        {
            packet *q = (packet*)s->items[j];
            printf( "packet is " );
            if( p->data.header.startbyte == MESSAGE_START )
            {
                if(p->data.header.payload[MAX_PAYLOAD_SIZE] == MESSAGE_CONTINUE)
                {
                    //
                    //
                    //
                    // FIXME: This is part of matching stuff.
                    if(p->data.header.pid.integer == q->data.header.pid.integer)
                    {
                        //Only checking if we come before a packet we have,since
                        //we're a starting byte.
                        if( p->data.header.order == (q->data.header.order-1) )
                        {
                            printf("packet comes before q\n");
                            dynset_add( s, p );
                            matches+=1;
                        }
                    }
                    //
                    //
                    //
                }
                else
                {
                    //assume it's a full message ?
                    //If it isn't, it must be an ending byte, otherwise it's
                    //garbage
                }
        }
        else if( p->data.header.startbyte == MESSAGE_RESUME )
        {
            if( p->data.header.payload[MAX_PAYLOAD_SIZE] == MESSAGE_CONTINUE )
            {
                //
                //
                //
                // FIXME: This is part of matching stuff.
                if( p->data.header.pid.integer == q->data.header.pid.integer )
                {
                    //printf("pid matches.\n");
                    if( p->data.header.order == (q->data.header.order-1) )
                    {
                        printf("packet comes before q\n");
                        dynset_add( s, p );
                        matches+=1;
                    }
                    else if( p->data.header.order == (q->data.header.order+1) )
                    {
                        printf("packet comes after q\n");
                        dynset_add( s, p );
                        matches+=1;
                    }
                }
                //
                //
                //
            }
            else
            {
                //
                //
                //
                // FIXME: This is part of matching stuff.
                if( p->data.header.pid.integer == q->data.header.pid.integer )
                {
                    if( p->data.header.order == (q->data.header.order+1) )
                    {
                        printf("packet comes after q\n");
                        dynset_add( s, p );
                        matches+=1;
                    }
                }
                //
                //
                //
            }
        }
        else
        {
            //malformed packet
        }
        
        }
    }
    if( matches == 0 )
    {
        dynset *s = new_dynset( );
        dynset_add( s, p );
        dynset_add( fragments, s );
    }
}

dynset *split_message( message *m )
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
                        //p->data.header.order = s->len;
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
unsigned char key_4[ ] = "keyfive";

unsigned char value_0[ ] = "valueone";
unsigned char value_1[ ] = "valuetwo";
unsigned char value_2[ ] = "valuethree";
unsigned char value_3[ ] = "valuefour";
unsigned char value_4[ ] = "valuefive";
void main( )
{
    current_packet_count = 0;
    item *i_0 = new_item( );
    item *i_1 = new_item( );
    item *i_2 = new_item( );
    item *i_3 = new_item( );
    item *i_4 = new_item( );
    
    i_0->k = key_0;
    i_0->k_len = sizeof( key_0 );
    i_1->k = key_1;
    i_1->k_len = sizeof( key_1 );
    i_2->k = key_2;
    i_2->k_len = sizeof( key_2 );
    i_3->k = key_3;
    i_3->k_len = sizeof( key_3 );
    i_4->k = key_4;
    i_4->k_len = sizeof( key_4 );
    
    i_0->v = value_0;
    i_0->v_len = sizeof( value_0 );
    i_1->v = value_1;
    i_1->v_len = sizeof( value_1 );
    i_2->v = value_2;
    i_2->v_len = sizeof( value_2 );
    i_3->v = value_3;
    i_3->v_len = sizeof( value_3 );
    i_4->v = value_4;
    i_4->v_len = sizeof( value_4 );
    
    message *m1 = new_message( );
    message *m2 = new_message( );
    
    dynset_add( m1->kvs, i_0 );
    dynset_add( m1->kvs, i_1 );
    dynset_add( m2->kvs, i_2 );
    dynset_add( m2->kvs, i_3 );
    dynset_add( m2->kvs, i_4 );
    
    //We have a message we want to send, but we only send packets.
    dynset *s = split_message( m1 );
    dynset *s2 = split_message( m2 );
    
    //printf("%i\n", s->len);
    int l;
    for( l=0; l < s->len;l++ )
    {
        packet *pb = (packet*)s->items[l];
        //packet_sorter_add( pb );
        printf( "\nPacket: " );
        int k;
        for(k=0; k < MAX_PACKET_SIZE; k++ )
        {
            if( pb->data.buffer[k] > 31 && pb->data.buffer[k] < 127 )
                printf("%c", pb->data.buffer[k]);
            else
                printf("%x", pb->data.buffer[k]);
        }
        printf("\n");
    }
    packet_sorter_add( (packet*)s->items[1] );
    packet_sorter_add( (packet*)s->items[0] );
    
    for( l=0; l < s2->len;l++ )
    {
        packet *pb = (packet*)s2->items[l];
        //packet_sorter_add( pb );
        printf( "\nPacket: " );
        int k;
        for(k=0; k < MAX_PACKET_SIZE; k++ )
        {
            if( pb->data.buffer[k] > 31 && pb->data.buffer[k] < 127 )
                printf("%c", pb->data.buffer[k]);
            else
                printf("%x", pb->data.buffer[k]);
        }
        printf("\n");
    }
    packet_sorter_add( (packet*)s2->items[2] );
    packet_sorter_add( (packet*)s2->items[0] );
    packet_sorter_add( (packet*)s2->items[1] );
    
    for( l=0; l < fragments->len;l++ )
    {
        dynset *s = (dynset*)fragments->items[l];
        printf( "frag[ %i ] = {\n", l );
        int k;
        for(k=0; k < s->len; k++ )
        {
            printf( "\t");
            packet *pb = (packet*)s->items[k];
        
        printf( "[" );
        int j;
        for(j=0; j < MAX_PACKET_SIZE; j++ )
        {
            if( pb->data.buffer[j] > 31 && pb->data.buffer[j] < 127 )
                printf("%c", pb->data.buffer[j]);
            else
                printf("%x", pb->data.buffer[j]);
        }
        printf( "]\n" );
        }
        printf("}\n");
    }
    
    //
    //
    //
    
    //We have some packets, but we only want messages.
    
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
