#include <stdio.h>
#include <stdlib.h>
#include "sgfile.h"

/* Create a graph from a file.sg */
graph *open_sg (const char *filename)
{
  unsigned int  i = 0,
                j, id;
  char          c = '\0',
                *buffer;
  FILE          *fp = fopen(filename, "r");
  graph         *G = (graph*) malloc(sizeof(graph));
  if(fp == NULL)
    return NULL;

  /* Count lines of file. */
  G->size = 0;
  while ((c=getc(fp)) != EOF)
    if (c == '\n')
      G->size++;
  fseek(fp, 0, SEEK_SET);

  /* Count neighbors. */
  G->nsize = (unsigned int*) calloc(G->size, sizeof(unsigned int));
  G->neigh = (unsigned int**) malloc(G->size * sizeof(unsigned int*));
  while((c=getc(fp)) != EOF) {
    if (c == ' ') {
      G->nsize[i]++;
    } else if (c == '\n') {
      G->neigh[i] = (unsigned int*) malloc(G->nsize[i] * sizeof(unsigned int));
      i++;
    }
  }
  fseek(fp, 0, SEEK_SET);

  /* Parse */
  i = 0;
  buffer  = (char*) calloc(_digits(G->size) + 1, sizeof(char));

  while ((c = getc(fp)) != EOF) {
    if (c == ':') {
      buffer[i] = '\0';
      id = atoi(buffer);
      i = 0;
      j = 0;
      if ((c = getc(fp)) == '\n')
        continue;
    } else if (c == ' ' || c == '\n') {
      buffer[i] = '\0';
      G->neigh[id][j++] = atoi(buffer);
      i = 0;
    } else {
      buffer[i++] = c;
    }
  }

  fclose(fp);
  return G;
}

/* Delete a graph. */
void graph_del (graph *G)
{
  for (int i = 0; i < G->size; free(G->neigh[i++]));
  free(G->nsize);
  free(G);
}

/* stupid but fast! */
unsigned int _digits (unsigned int n)
{
  if (n < 10) return 1;
  if (n < 100) return 2;
  if (n < 1000) return 3;
  if (n < 10000) return 4;
  if (n < 100000) return 5;
  if (n < 1000000) return 6;
  if (n < 10000000) return 7;
  if (n < 100000000) return 8;
  return -1;
}

