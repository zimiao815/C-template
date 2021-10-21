###############git#######

CFLAGS=-O0 -g -D_GNU_SOURCE
LDFLAGS= -lpthread  -lcurl #-lssl -lcrypto  #-lm -luci  -lz


C_FILES = main.c curl_op.c debug.c file_op.c linkedlist.c \
		  config.c pthread_op.c safe.c tick.c tick.h unit.c 

C_OBJS = $(C_FILES:%.c=%.o)


a.out: $(C_OBJS)
	rm $@ -fr
	$(CC) $(CFLAGS)   -o $@   $^ $(LDFLAGS)

clean:
	rm *.o core.*  -fr 
.c.o:
	$(CC) $(CFLAGS)  -c    $<		
		
		
		
