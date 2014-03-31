#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>

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
    item **items;
    unsigned int num_items;
} message;

typedef struct
{
    union
    {
        unsigned char bytes[4];
        unsigned int integer;
    } pid;
    unsigned char order;
    message *msg;
} packet;

unsigned char msgtest0[] = "xxxx0<1keyone|valueone|keytwo|valuetwo|>";
unsigned char msgtest1[] = "xxxx1<1keyone|valu@";
unsigned char msgtest2[] = "xxxx1$eone|keytwo|valuetwo|>";


#define MESSAGE_START     '<'
#define MESSAGE_CONTINUE  '@'
#define MESSAGE_RESUME    '$'
#define MESSAGE_END       '>'
#define DELIMITER         '|'


void main( )
{
    //
    // Let's make a (empty) packet!
    packet *p      = (packet*)malloc( sizeof( packet ) );
    p->pid.integer = getpid( );
    p->order       = 0;
    p->msg         = NULL;
    //
    //
    
    //
    // Let's make a (empty) message!
    message *m   = (message*)malloc( sizeof( message ) );
    m->type      = 0;
    m->items     = NULL;
    m->num_items = 0;
    //
    //
    
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
