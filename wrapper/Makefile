
CC = gcc
CCFLAGS = -g -O3 -Wshadow -Wconversion -D_THREAD_SAFE -D_REENTRANT 
TARGET = msg

SOURCES = msg.c \
          packet.c \
          dynset.c
		  
%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@ 

$(TARGET): $(SOURCES:.c=.o)
	$(CC) $(CCFLAGS) -o $@ $^

.PHONY: clean count
	
clean:
	@ $(RM) *.o $(TARGET)

count:
	@ wc -l *.c *.h | sort

