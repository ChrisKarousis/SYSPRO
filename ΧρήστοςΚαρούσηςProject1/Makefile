OBJS    = mysh.o history.o alias.o exec.o
SOURCE  = mysh.c history.c alias.c exec.c
HEADER  = history.h alias.h exec.h
OUT     = mysh
CC      = gcc
FLAGS   = -g -c

$(OUT) : $(OBJS)
		$(CC) -g $(OBJS) -o $@

mysh.o : mysh.c
		$(CC) $(FLAGS) mysh.c

history.o : history.c
		$(CC) $(FLAGS) history.c

alias.o : alias.c
		$(CC) $(FLAGS) alias.c

exec.o : exec.c
		$(CC) $(FLAGS) exec.c

clean :
		rm -f $(OBJS) $(OUT)

count :
		wc $(SOURCE) $(HEADER)

