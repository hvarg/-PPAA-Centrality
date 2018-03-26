FLAGS = -O3 -pthread
OBJS  = pipeline.c sgfile.c ilist.c main.c
HEAD  = pipeline.h sgfile.h ilist.h
NAME  = pipeline_cent

$(NAME): $(OBJS) $(HEAD)
	gcc $(FLAGS) -o $(NAME) $(OBJS)

debug: $(OBJS) $(HEAD)
	gcc -pthread -g -o $(NAME) $(OBJS)

