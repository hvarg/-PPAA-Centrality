#include <stdlib.h>
#include <stdio.h>
#include "ilist.h"
#include "centrality.h"

float *betweenness_centrality(graph *G)
{
  return betweenness_centrality_range(G, 0, G->size);
}

float *betweenness_centrality_range(graph *G, int start, int end)
{
  /* MEM init */
  float *BC = (float *) malloc(sizeof(float) * G->size);
  int s, t, v, w, ph, count,
      *d = (int *) malloc(sizeof(int) * G->size);
  float *sigma = (float *) malloc(sizeof(float) * G->size),
        *delta = (float *) malloc(sizeof(float) * G->size);
  ilist **P = (ilist **) malloc(sizeof(ilist*) * G->size),
        **S = (ilist **) malloc(sizeof(ilist*) * G->size);
  // S probably dont use all the memory allocated here, but its better
  // than a dinamic array or something like that.
  int i, j, k;
  item *elem1, *elem2;
  for (s=0; s < G->size; s++){
    BC[s] = 0.0;
    S[s] = NULL;
    P[s] = NULL;
  }
  if (end > G->size) end = G->size;

  /* Centrality */
  for (s=start; s < end; s++) {
    /* Initialization and reset of variables.
     * BC is the only variable that is not reset. */
    for (t=0; t < G->size; t++) {
      P[t] = new_ilist();
      sigma[t] = 0.0;
      d[t] = -1;
    }
    sigma[s] = 1.0;
    d[s] = 0;
    ph = 0;
    S[ph] = new_ilist();
    ilist_add(S[ph], s);
    count = 1;
    /* Shortest path discovery and conunting.
     * Uses: S, ph, sigma, P */
    while (count > 0){
      count = 0;
      for (elem1 = S[ph]->first; elem1 != NULL; elem1 = elem1->next) {
        v = elem1->value;
        for (i = 0; i < G->nsize[v]; i++) {
          w = G->neigh[v][i];
          if (d[w] < 0) {
            if (S[ph+1] == NULL) S[ph+1] = new_ilist();
            ilist_add(S[ph+1], w);
            count++;
            d[w] = d[v] + 1;
          }
          if (d[w] == d[v] + 1) {
            sigma[w] += sigma[v];
            ilist_add(P[w], v);
          }
        }
      }
      ph++;
    }
    ph--;
    /* Dependency accumulation. 
     * Uses: S, ph, sigma, P. */
    for (t=0; t < G->size; t++)
      delta[t] = 0.0;
    while (ph > 0) {
      for (elem1 = S[ph]->first; elem1 != NULL; elem1 = elem1->next) {
        w = elem1->value;
        for (elem2 = P[w]->first; elem2 != NULL; elem2 = elem2->next) {
          v = elem2->value;
          delta[v] += (sigma[v]/sigma[w]) * (1+delta[w]);
        }
        BC[w] += delta[w];
      }
      ph--;
    }
    /* Free stuff */
    for (t=0; t < G->size; t++) {
      if (S[t] != NULL){
        ilist_del(S[t]);
        S[t] = NULL;
      }
      if (P[t] != NULL) {
        ilist_del(P[t]);
        P[t] = NULL;
      }
    }
  }
  free(d);
  free(sigma);
  free(delta);
  free(P);
  free(S);
  return BC;
}

/* vim: set ts=2 sw=2 sts=2 tw=80 : */
