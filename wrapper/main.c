#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "4950"

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

typedef struct
{
    int fd;
    struct addrinfo *p;
} udpsocket;

udpsocket *open_socket( char *address, char recv )
{
    udpsocket *s = malloc( sizeof( udpsocket ) );
    int sockfd;
    int rv;
    struct addrinfo hints, *servinfo, *p;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    rv = getaddrinfo( address, SERVERPORT, &hints, &servinfo );
    if( rv != 0 )
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }
    
    // loop through all the results and make a socket
    for( p = servinfo; p != NULL; p = p->ai_next )
    {
        sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
        if( sockfd == -1 )
        {
            perror("open_socket: socket");
            continue;
        }
        if( recv == 1 )
        {
            if( bind(sockfd, p->ai_addr, p->ai_addrlen) == -1 )
            {
                close(sockfd);
                perror("open_socket: bind");
                continue;
            }
        }
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "open_socket: failed to bind socket\n");
        return NULL;
    }
    s->fd = sockfd;
    s->p  = p;
    //freeaddrinfo(servinfo);
    return s;
}

void send_message( udpsocket *u, message *m )
{
    dynset *s = split_message( m );
    int numbytes;
    int l;
    for( l=0; l < s->len; l++ )
    {
        packet *pb = (packet*)s->items[l];
        numbytes = sendto( u->fd, pb->data.buffer, MAX_PACKET_SIZE, 0,
                                   u->p->ai_addr, u->p->ai_addrlen);
        if( numbytes == -1)
        {
            perror("send_message: sendto");
            exit(1);
        }
    }
    
    printf("Sent %d bytes\n", numbytes);
}

message *recv_message( udpsocket *u )
{
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    int numbytes;
    addr_len = sizeof their_addr;
    packet *p = new_packet_empty( );
    numbytes = recvfrom( u->fd, p->data.buffer, MAX_PACKET_SIZE, 0,
                         (struct sockaddr *)&their_addr, &addr_len );
    
    if( numbytes == -1 )
    {
        perror("recvfrom");
        exit(1);
    }
    
    //FIXME: implement a "message complete" return path.
    //       so we can say:
    //       message *m = packet_sorter_add( p )
    //       where m can be NULL or a full message (key,value pairs)
    packet_sorter_add( p );
}

int main(int argc, char *argv[])
{
    if( argc > 1 )
    {
        if( strcmp( argv[1], "send") == 0 )
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
            printf( "Opening socket for %s\n", argv[2] );
            udpsocket *sock = open_socket( argv[2], 0 );
            if( sock != NULL )
            {
                send_message( sock, m1 );
                send_message( sock, m2 );
            }
            return 0;
        }
        
        if( strcmp( argv[1], "recv") == 0 )
        {
            udpsocket *sock = open_socket( argv[2],1 );
            
            while( 1 )
            {
                recv_message( sock );
            int l;
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
                        if( pb->data.buffer[j] > 31 && pb->data.buffer[j] < 127)
                            printf("%c", pb->data.buffer[j]);
                        else
                            printf("%x", pb->data.buffer[j]);
                    }
                    printf( "]\n" );
                }
                printf("}\n");
            }
            }
        }
    }
    
    //We have some packets, but we only want messages.
    
    // unsigned char *msg = (unsigned char*)malloc( s->len * MAX_PAYLOAD_SIZE );
    // unsigned int offset= 0;
    // for( l=0; l < s->len;l++ )
    // {
        // packet *pb = (packet*)s->items[l];
        // memcpy( msg + offset, pb->data.header.payload, MAX_PAYLOAD_SIZE );
        // offset += MAX_PAYLOAD_SIZE;
    // }
    // printf( "message buffer: " );
    // 
    // for(l=0; l < s->len * MAX_PAYLOAD_SIZE; l++ )
    // {
        // if( msg[l] > 31 && msg[l] < 127 )
            // printf("%c", msg[l]);
        // else
            // printf("%x", msg[l]);
    // }
    // printf("\n");
    //We get a packet, extract the first five bytes, then parse the remaining.
    
    //We'll want to check if the last byte is a continue, if it is we'll wait
    //for the other part(s).
    
    //We'll check the first byte, in case it's a resume. If it is we'll match
    //it up with any continue we have pending.
    
    
    // int i = 0;
    // int j;
    // for( j = i; j < s->len * MAX_PAYLOAD_SIZE; j++ )
    // {
        // if( msg[ j ] == DELIMITER )
        // {
            // item *t = (item*)malloc( sizeof( item ) );
            // 
            // int len = j - i;
            // unsigned char *tmp = (unsigned char*)malloc( len+1 );
            // memcpy( tmp, msg+i, len );
            // tmp[ len ] = '\0';
            // printf( "%s\n", tmp );
            // 
            // i = j+1;
        // }
    // }
}

