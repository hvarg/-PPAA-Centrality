#ifndef PIPEL_H_
#define PIPEL_H_

#include <pthread.h>
#include <semaphore.h>
#include "ilist.h"
#include "sgfile.h"

/* Data of the job to be done. */
typedef struct _job{
  int         ph,id;
  float       *sigma, *bc;
  ilist       **P, **S;
  struct _job *next;
} job;

/* Jobs queue. */
typedef struct {
  job           *first, *last;  // First and last jobs.
  unsigned int  size;           // Queue size.
} jqueue;


/* Pipeline to compute betweenness centrality. */
typedef struct {
  graph           *G;               // Graph to be computed.
  float           *bc;              // Betweenness centrality.
  pthread_t       *thr_discovery,   // Discovery threads.
                  *thr_accum,       // Accumulation threads.
                  thr_sum;          // Thread to do the final sum.
  unsigned int    n_discovery,      // Number of discovery threads.
                  n_accum,          // Number of accumulation threads.
                  count;            // Count for the discovery task.
  jqueue          *queue,           // Job queue for 'accum'.
                  *queue2;          // Job queue for 'sum'.
  sem_t           *mutex_count,     // 'count' mutex.
                  *qmutex,          // 'queue' mutex.
                  *qmutex2,         // 'queue2' mutex.
                  *st,              // Semaphore for start 'accum' threads.
                  *st2;             // Semaphore for start 'sum' threads.
  pthread_mutex_t *wmutex;          // Working mutex.
} pipeline_cent;

/* Function declarations */
pipeline_cent*    pipeline_create   (graph *G, unsigned int n, unsigned int m);
void              pipeline_start    (pipeline_cent *P);
void              pipeline_del      (pipeline_cent *P);
void              pipeline_wait     (pipeline_cent *P);
void*             _discovery        (void *vp);
void*             _accum            (void *vp);
void*             _sum              (void *vp);
jqueue*           jqueue_new        (void);
void              jqueue_del        (jqueue *jq);
job*              jdequeue          (jqueue *jq);
void              jenqueue2         (jqueue *jq, job *j, float *bc);
void              jenqueue          (jqueue *jq, int id, int ph, float *sigma,
                                     ilist **P, ilist **S);

#endif
/* vim: set ts=2 sw=2 sts=2 tw=80 : */
