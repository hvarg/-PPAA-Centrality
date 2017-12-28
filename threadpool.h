#ifndef POOL_H_
#define POOL_H_

#include <pthread.h>
#include <semaphore.h>

/* Job to be done. */
typedef struct _job{
  void        (*func)(void* args);   // Function.
  void        *args;                 // Arguments.
  struct _job *next;                 // Next Job.
} job;

/* Jobs queue. */
typedef struct {
  job           *first,       // First Job.
                *last;        // Last Job.
  unsigned int  size;         // Queue size.
} jqueue;


/* Pool thread. */
typedef struct {
  pthread_t       *threads;   // Threads.
  jqueue          *queue;     // Job queue. FIXME
  unsigned int    size,       // Total number of threads.
                  working;    // Number of currently working threads.
  sem_t           *qmutex,    // Queue mutex.
                  *st;        // Semaphore for start threads.
  pthread_mutex_t *wmutex;    // Working mutex.
  pthread_cond_t  *idle;      // When idle
} pool;

/* Function declarations */
pool*     pool_create       (unsigned int NTHREADS);
void      pool_del          (pool *P);
void*     _worker           (void *vp);
void      pool_send_job     (pool *P, void *func, void *args);
void      pool_wait         (pool *P);
void      _pool_rm_job      (pool *P);
jqueue*   jqueue_new        (void);
void      jqueue_del        (jqueue *jq);
job*      jdequeue          (jqueue *jq);
void      jenqueue          (jqueue *jq, job *j);

#endif
/* vim: set ts=2 sw=2 sts=2 tw=80 : */
