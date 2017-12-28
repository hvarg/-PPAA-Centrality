#ifndef SGFILE_H_
#define SGFILE_H_

typedef struct {
  unsigned int  size,     // Number of nodes.
                **neigh,  // Neighbors of each node.
                *nsize;   // Number of neighbors of each node.
} graph;

graph         *open_sg  (const char *filename);
void          graph_del (graph *G);
unsigned int  _digits   (unsigned int n);

#endif
/* vim: set ts=2 sw=2 sts=2 tw=80 : */
