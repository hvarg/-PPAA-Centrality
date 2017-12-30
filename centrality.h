#ifndef CENTRALITY_H_
#define CENTRALITY_H_

#include "sgfile.h"

float *betweenness_centrality(graph *G);
float *betweenness_centrality_range(graph *G, int init, int end);

#endif
/* vim: set ts=2 sw=2 sts=2 tw=80 : */
