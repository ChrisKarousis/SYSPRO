OBJS    = poller.o pollSwayer.o
SOURCE  = poller.c pollSwayer.c
OUT     = poller pollSwayer
CC      = gcc
FLAGS   = -g -c -lpthread

$(OUT) : $(OBJS)
		$(CC) -g $(@:=.o) -o $@ -lpthread

poller.o : poller.c
		$(CC) $(FLAGS) poller.c

pollSwayer.o : pollSwayer.c
		$(CC) $(FLAGS) pollSwayer.c

clean :
		rm -f $(OBJS) $(OUT)

count :
		wc $(SOURCE)

