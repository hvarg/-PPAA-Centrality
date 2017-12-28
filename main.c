#include <stdio.h>
#include <stdlib.h>
#include "threadpool.h"
#include "sgfile.h"


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
  int i;
  
  /* Arguments */
  if (argc < 2) {
    printf("Modo de uso: ./centrality archivo.sg [#threads]\n");
    return EXIT_SUCCESS;
  }
  char  *filename = (char*) args[1]; 
  int   NT = (argc > 2) ? atoi(args[2]) : 4;
  printf("File: %s\n", filename);
  printf("Threads: %d\n", NT);
  
  /* Open graph */
  graph *G = open_sg(filename);
  if (G == NULL) 
    return EXIT_FAILURE;
  
  /* Create pool */
  int   gap = (G->size%NT == 0) ? G->size/NT : (G->size/NT)+1;
  pool  *P  = pool_create(NT);
  
  /* Create data structures for each job. */
  struct data  **D = (struct data**) malloc(sizeof(struct data*)*NT);
  for (i = 0; i < NT; i++) {
    D[i] = (struct data*) malloc(sizeof(struct data));
    D[i]->start = i*gap;
    D[i]->end   = ((i+1)*gap - 1) < G->size ? ((i+1)*gap -1) : G->size - 1;
  }

  /* Send jobs to pool */
  for (i = 0; i < NT; i++) {
    pool_send_job(P, __cent, D[i]);
  }
  pool_wait(P);
  pool_del(P);

  /* Free Data */
  for (i = 0; i < NT; i++) {
    free(D[i]);
  }
  free(D);

  /* Print graph 
  int j;
  for (i = 0; i < G->size; i++) {
    printf("%d:", i);
    for (j = 0; j < G->nsize[i]; j++) {
      printf(" %d", G->neigh[i][j]);
    }
    printf("\n");
  }*/

  graph_del(G);

  return EXIT_SUCCESS;
}

/* vim: set ts=2 sw=2 sts=2 tw=80 : */
