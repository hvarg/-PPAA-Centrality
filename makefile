FLAGS = -O3 -pthread
OBJS1  = threadpool.c sgfile.c centrality.c ilist.c main_tp.c
HEAD1  = threadpool.h sgfile.h centrality.h ilist.h
NAME1  = bc_tp
OBJS2  = pipeline.c sgfile.c ilist.c main_pl.c
HEAD2  = pipeline.h sgfile.h ilist.h
NAME2  = bc_pl

all:
	gcc $(FLAGS) -o $(NAME1) $(OBJS1)
	gcc $(FLAGS) -o $(NAME2) $(OBJS2)

$(NAME1): $(OBJS1) $(HEAD1)
	gcc $(FLAGS) -o $(NAME1) $(OBJS1)

$(NAME2): $(OBJS2) $(HEAD2)
	gcc $(FLAGS) -o $(NAME2) $(OBJS2)

debug: $(OBJS) $(HEAD)
	gcc -pthread -g -o $(NAME1) $(OBJS1)
	gcc -pthread -g -o $(NAME2) $(OBJS2)
