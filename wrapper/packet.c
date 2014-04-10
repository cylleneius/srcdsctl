#include "packet.h"

packet *new_packet( )
{
    //
    // Let's make a (empty) packet!
    packet *p      = (packet*)malloc( sizeof( packet ) );
    p->data.header.pid.integer = getpid( );
    p->data.header.order       = current_packet_count;
    p->offset  = 0;
    //
    //
    current_packet_count += 1;
    
    return p;
}


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
        //q->data.header.order = p->data.header.order + 1;
        q->data.header.startbyte = MESSAGE_RESUME;
        dynset_add( s, q );
        
        q = packet_add( s, q, str+wr, len-wr );
        return q;
    }
    
    return p;
}
