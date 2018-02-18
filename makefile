FLAGS = -O3 -pthread
OBJS  = threadpool.c sgfile.c centrality.c ilist.c main.c
HEAD  = threadpool.h sgfile.h centrality.h ilist.h
NAME  = thpool_cent

$(NAME): $(OBJS) $(HEAD)
	gcc $(FLAGS) -o $(NAME) $(OBJS)

debug: $(OBJS) $(HEAD)
	gcc -pthread -g -o $(NAME) $(OBJS)

