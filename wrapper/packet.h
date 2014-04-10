#ifndef __PACKET_H__
#define __PACKET_H__

#include "dynset.h"

#define MESSAGE_START     '<'
#define MESSAGE_CONTINUE  '@'
#define MESSAGE_RESUME    '$'
#define MESSAGE_END       '>'
#define DELIMITER         '|'
#define MAX_PACKET_SIZE 30
#define PACKET_CONTROL_BYTES 8 // PPPPOSLT T=< or $
#define MAX_PAYLOAD_SIZE MAX_PACKET_SIZE - (PACKET_CONTROL_BYTES + 1) // endbyte

unsigned char current_packet_count;

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

#endif
