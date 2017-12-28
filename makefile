FLAGS = -O3 -pthread
OBJS  = threadpool.c main.c
HEAD  = threadpool.h
NAME  = test

$(NAME): $(OBJS) $(HEAD)
	gcc $(FLAGS) -o $(NAME) $(OBJS)

debug: $(OBJS) $(HEAD)
	gcc -pthread -g -o $(NAME) $(OBJS)

