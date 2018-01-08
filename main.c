#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threadpool.h"
#include "sgfile.h"
#include "centrality.h"

/* Global variables. */
float *BC;
int    NT;
graph *G;

/* Used to save the segments to compute and its results. */
struct data {
  int   start, end;
  float *bc;
};

/* Used to sum the partial results of the centrality. */
struct sdat {
  int i;
  struct data **D;
};

/* Thread's job. Compute part of the total centrality. */ 
void *__cent(void *DT)
{
  struct data *dt = (struct data*) DT;
  dt->bc = betweenness_centrality_range(G, dt->start, dt->end);
  return NULL;
}

/* Thread's job. Sums the results of the previus proccess. */
void *__sum(void *DT)
{
  struct sdat *sd = (struct sdat*) DT;
  int j, k; 
  for (j=sd->D[sd->i]->start; j<sd->D[sd->i]->end; j++) {
    for (k=0; k<NT; k++){
      BC[j] += sd->D[k]->bc[j];
    }
  }
  return NULL;
}

int main(int argc, const char * args[])
{
  /* Arguments */
  if (argc < 2) {
    printf("Modo de uso: ./centrality archivo.sg [#threads]\n");
    return EXIT_SUCCESS;
  }
  char  *filename = (char*) args[1]; 
  NT = (argc > 2) ? atoi(args[2]) : 4;
  printf("File: %s\n", filename);
  printf("Threads: %d\n", NT);

  /* Open graph */
  G = open_sg(filename);
  if (G == NULL) 
    return EXIT_FAILURE;

  /* Create pool */
  int   gap = (G->size%NT == 0) ? G->size/NT : (G->size/NT)+1;
  pool  *P  = pool_create(NT);

  /* Create data structures for each job. */
  int i;
  struct data **D  = (struct data**) malloc(sizeof(struct data*)*NT);
  struct sdat **SD = (struct sdat**) malloc(sizeof(struct sdat*)*NT);
  for (i = 0; i < NT; i++) {
    D[i] = (struct data*) malloc(sizeof(struct data));
    D[i]->start = i*gap;
    D[i]->end   = (i+1)*gap;
    SD[i] = (struct sdat*) malloc(sizeof(struct sdat));
    SD[i]->i = i;
    SD[i]->D = D;
  }

  /* Send jobs to pool */
  for (i = 0; i < NT; i++) {
    pool_send_job(P, __cent, D[i]);
  }
  pool_wait(P);

  BC = (float*) calloc(G->size, sizeof(float));
  for (i = 0; i < NT; i++) {
    pool_send_job(P, __sum, SD[i]);
  }
  pool_wait(P);
  pool_del(P);

  /* Free jobs data */
  for (i = 0; i < NT; i++) {
    free(D[i]);
    free(SD[i]);
  }
  free(D);
  free(SD);

  /* Saving results. */
  short fn_size = strlen(filename);
  char *bc_out  = (char *) malloc(sizeof(char) * (fn_size + 11));
  strcpy(bc_out, filename);
  strcat(bc_out, ".bc.result");
  FILE *fbc = fopen(bc_out, "w");
  for (i=0; i< G->size; i++){
    fprintf(fbc,"%d: %f\n", i, BC[i]);
  }
  fclose(fbc);
  free(bc_out);

  graph_del(G);

  return EXIT_SUCCESS;
}

/* vim: set ts=2 sw=2 sts=2 tw=80 : */
