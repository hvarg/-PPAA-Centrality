#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threadpool.h"
#include "sgfile.h"


int main(int argc, const char * args[])
{
  /* Arguments */
  if (argc < 2) {
    printf("Modo de uso: ./centrality archivo.sg [#dth] [#ath]\n");
    return EXIT_SUCCESS;
  }
  char  *filename = (char*) args[1];
  int i,
      n = (argc > 2) ? atoi(args[2]) : 3,
      m  = (argc > 3) ? atoi(args[3]) : 3;
  printf("File: %s\n", filename);
  printf("Threads: %d, %d\n", n, m);

  /* Open graph */
  graph *G = open_sg(filename);
  if (G == NULL) 
    return EXIT_FAILURE;

  /* Create pool */
  pipeline_cent  *P  = pipeline_create(G, n, m);

  pipeline_start(P);
  pipeline_wait(P);

  /* Saving results. */
  short fn_size = strlen(filename);
  char *bc_out  = (char *) malloc(sizeof(char) * (fn_size + 11));
  strcpy(bc_out, filename);
  strcat(bc_out, ".bc.result");
  FILE *fbc = fopen(bc_out, "w");
  for (i=0; i< G->size; i++){
    fprintf(fbc,"%d: %f\n", i, P->bc[i]);
  }
  fclose(fbc);
  free(bc_out);
  pipeline_del(P);
  graph_del(G);

  return EXIT_SUCCESS;
}

/* vim: set ts=2 sw=2 sts=2 tw=80 : */
