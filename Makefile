CFLAGS=-O0 -g -D_GNU_SOURCE
LDFLAGS= -lpthread  -lcurl #-lssl -lcrypto  #-lm -luci  -lz
MONITOR_OBJS = main.o 


a.out:main.o config.o safe.o debug.o file_op.o sock.o unit.o tick.o pthread_op.o http.o curl_op.o linkedlist.o
	gcc main.o config.o safe.o debug.o file_op.o sock.o unit.o tick.o pthread_op.o http.o curl_op.o linkedlist.o $(LDFLAGS)
main.o:main.c
	gcc -c $(CFLAGS)  main.c
debug.o:debug.c
	gcc -c $(CFLAGS) debug.c
safe.o:safe.c
	gcc -c $(CFLAGS) safe.c
config.o:config.c
	gcc -c $(CFLAGS) config.c
file_op.o:file_op.c
	gcc -c $(CFLAGS) file_op.c
sock.o:sock.c
	gcc -c $(CFLAGS) sock.c
unit.o:unit.c
	gcc -c $(CFLAGS) unit.c
tick.o:tick.c
	gcc -c $(CFLAGS) tick.c
pthread_op.o:pthread_op.c
	gcc -c $(CFLAGS) pthread_op.c 
http.o:http.c
	gcc -c $(CFLAGS) http.c 
curl_op.o:curl_op.c
	gcc -c $(CFLAGS) curl_op.c 
linkedlist.o:linkedlist.c
	gcc -c $(CFLAGS) linkedlist.c 
	
clean:
	$(RM) -f *.o *~ a.out
