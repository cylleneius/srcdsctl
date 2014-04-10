#include "msg.h"

item *new_item( unsigned char *k, unsigned char *v )
{
    item *i  = (item*)malloc( sizeof( item ) );
    i->k     = k;
    if( k != NULL )
        i->k_len = strlen( k ) + 1;
    else
        i->k_len = 0;
    i->v     = v;
    if( v != NULL )
        i->v_len = strlen( v ) + 1;
    else
        i->v_len = 0;
    return i;
}

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
fragment_item *new_fragment_item( void *c )
{
    fragment_item *i = (fragment_item*)malloc( sizeof( fragment_item ) );
    i->prev = NULL;
    i->next = NULL;
    i->curr = c;
    return i;
}

fragment_sequence *new_fragment_sequence( )
{
    fragment_sequence *s =(fragment_sequence*)malloc(sizeof(fragment_sequence));
    s->head = NULL;
    s->tail = NULL;
    return s;
}

// Add i after the last item in sequence
void fragment_sequence_add( fragment_sequence *s, fragment_item *i )
{
    if( s->head == NULL )
    {
        s->head = i;
        s->tail = i;
    }
    else
    {
        s->tail->next = i;
        i->prev = i;
        s->tail = i;
    }
}

// Add j before i
void fragment_sequence_add_before( fragment_sequence *s, fragment_item*i,
                                                         fragment_item*j )
{
    if( i->prev != NULL )
    {
        ((fragment_item*)i->prev)->next = j;
        j->prev = i->prev;
        
        i->prev = j;
        j->next = i;
    }
    else
    {
        i->prev = j;
        j->next = i;
    }
    if( i == s->head )
    {
        s->head = j;
    }
}

// Add j after i
void fragment_sequence_add_after( fragment_sequence *s, fragment_item*i,
                                                        fragment_item*j )
{
    if( i == j ) return;
    if( i->next != NULL )
    {
        ((fragment_item*)i->next)->prev = j;
        j->next = i->next;
        
        i->next = j;
        j->prev = i;
    }
    else
    {
        i->next = j;
        j->prev = i;
    }
    if( i == s->tail )
    {
        s->tail = j;
    }
}

message *new_message( )
{
    message *m   = (message*)malloc( sizeof( message ) );
    m->type      = 0;
    m->kvs     = new_dynset( );
    return m;
}

int match_packet_order( packet *p, packet *q )
{
    if( p->data.header.pid.integer == q->data.header.pid.integer )
    {
        if( p->data.header.order == (q->data.header.order-1) )
        {
            return -1;
        }
        if( p->data.header.order == (q->data.header.order+1) )
        {
            return 1;
        }
    }
    return 0;
}

dynset *packet_sorter_add( packet *p )
{
    if( fragments == NULL ) fragments = new_dynset( );
    int matches = 0;
    if( p->data.header.startbyte == MESSAGE_START )
    {
        if(p->data.header.payload[MAX_PAYLOAD_SIZE] == MESSAGE_CONTINUE)
        {
            int i;
            for( i=0; i < fragments->len; i++ )
            {
                fragment_sequence *s = (dynset*)fragments->items[ i ];
                packet *q = (packet*)s->head->curr;
                if( match_packet_order( p, q ) == -1 )
                {
                    fragment_sequence_add_before( s, s->head,
                                                       new_fragment_item( p ) );
                    matches+=1;
                }
            }
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
            int i;
            fragment_sequence *one = NULL;
            fragment_sequence *two = NULL;
            for( i=0; i < fragments->len; i++ )
            {
                fragment_sequence *s = (dynset*)fragments->items[ i ];
                packet *q1 = (packet*)s->head->curr;
                packet *q2 = (packet*)s->tail->curr;
                if( match_packet_order( p, q1 ) == -1 )
                {
                    fragment_sequence_add_before( s, s->head,
                                                       new_fragment_item( p ) );
                    one = s;
                    matches+=1;
                }
                if( match_packet_order( p, q2 ) == 1 )
                {
                    fragment_sequence_add_after( s, s->tail,
                                                  new_fragment_item( p ) );
                    two = s;
                    matches+=1;
                }
            }
            //middle byte
            if( matches == 2 )
            {
                if( one->head->curr == two->tail->curr )
                {
                    //skip the first one
                    fragment_item *i = one->head->next;
                    for( ; i != NULL; i = i->next )
                    {
                        if( i == i->next )
                        {
                            i->next = NULL;
                            break;
                        }
                        fragment_sequence_add_after( two, two->tail, i );
                    }
                    dynset_del( fragments, one );
                    free( one );
                }
            }
        }
        else
        {
            int i;
            for( i=0; i < fragments->len; i++ )
            {
                fragment_sequence *s = (dynset*)fragments->items[ i ];

                packet *q = (packet*)s->tail->curr;
                if( match_packet_order( p, q ) == 1 )
                {
                    fragment_sequence_add_after( s, s->tail,
                                                  new_fragment_item( p ) );
                    matches+=1;
                }
            }
        }
    }
    else
    {
        //malformed packet
    }
    
    if( matches == 0 )
    {
        fragment_sequence *s = new_fragment_sequence( );
        fragment_sequence_add( s, new_fragment_item( p ) );
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
unsigned char key_5[ ] = "keysix";

unsigned char value_0[ ] = "valueone";
unsigned char value_1[ ] = "valuetwo";
unsigned char value_2[ ] = "valuethree";
unsigned char value_3[ ] = "valuefour";
unsigned char value_4[ ] = "valuefive";
unsigned char value_5[ ] = "valuesix";

void main( )
{
    current_packet_count = 0;
    item *i_0 = new_item( key_0, value_0 );
    item *i_1 = new_item( key_1, value_1 );
    item *i_2 = new_item( key_2, value_2 );
    item *i_3 = new_item( key_3, value_3 );
    item *i_4 = new_item( key_4, value_4 );
    item *i_5 = new_item( key_5, value_5 );
    
    message *m1 = new_message( );
    message *m2 = new_message( );
    
    dynset_add( m1->kvs, i_0 );
    dynset_add( m1->kvs, i_1 );
    dynset_add( m2->kvs, i_2 );
    dynset_add( m2->kvs, i_3 );
    dynset_add( m2->kvs, i_4 );
    dynset_add( m2->kvs, i_5 );
    
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
    packet_sorter_add( (packet*)s2->items[0] );
    packet_sorter_add( (packet*)s2->items[2] );
    packet_sorter_add( (packet*)s2->items[3] );
    packet_sorter_add( (packet*)s2->items[1] );
    
    for( l=0; l < fragments->len;l++ )
    {
        fragment_sequence *s = (fragment_sequence*)fragments->items[l];
        printf( "frag[ %i ] = {\n", l );
        fragment_item *k = s->head;
        for(; k != NULL; k = k->next )
        {
            printf( "\t");
            packet *pb = (packet*)k->curr;
        
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
