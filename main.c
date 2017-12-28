#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threadpool.h"


/* Aqui se guardaran los segmentos a calcular y su resultado. */
struct data{
  int   start, end;
};

/* Trabajo que hara cada thread. Calcula una parte de la centralidad total. */
void *__cent(void *DT)
{
  struct data *dt = (struct data*) DT;
  printf("%d -> %d\n", dt->start, dt->end);
  return NULL;
}

int main(int argc, const char * args[])
{
  int   R  = (argc > 1) ? atoi(args[1]) : 10,
        NT = (argc > 2) ? atoi(args[2]) : 4,
        i, gap;
  struct data  **D;
  pool  *P;

  printf("Range: %d\n", R);
  printf("Threads: %d\n", NT);
  gap = (R%NT == 0) ? R/NT : (R/NT)+1;
  D   = (struct data**) malloc(sizeof(struct data*)*NT);
  P   = pool_create(NT);

  for (i = 0; i < NT; i++) {
    D[i] = (struct data*) malloc(sizeof(struct data));
    D[i]->start = i*gap;
    D[i]->end   = ((i+1)*gap - 1) < R ? ((i+1)*gap -1) : R;
  }

  for (i = 0; i < NT; i++) {
    pool_send_job(P, __cent, D[i]);
  }
  pool_wait(P);
  pool_del(P);

  for (i = 0; i < NT; i++) {
    free(D[i]);
  }
  free(D);

  return EXIT_SUCCESS;
}

/* vim: set ts=2 sw=2 sts=2 tw=80 : */
