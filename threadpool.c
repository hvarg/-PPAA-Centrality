#include <stdlib.h>
#include "threadpool.h"

#ifdef __APPLE__
#define named_sem_init(VAR, NAME, MODE) {\
if (((VAR) = sem_open((NAME), O_CREAT|O_EXCL, 0644, (MODE))) == SEM_FAILED) {\
  sem_unlink((NAME));\
  if (!((VAR) = sem_open((NAME), O_CREAT|O_EXCL, 0644, (MODE))))\
    return NULL;\
}\
}
#define QMNAME "/s1"
#define STNAME "/s2"
#endif

/* Create a job queue. */
jqueue* jqueue_new (void)
{
  jqueue *new = (jqueue*) malloc(sizeof(jqueue));
  new->first = NULL;
  new->last  = NULL;
  new->size  = 0;
  return new;
}

/* Delete a job queue. */
void jqueue_del (jqueue *jq)
{
  job *act, *next;
  for (act = jq->first; act != NULL; act = next) {
    next = act->next;
    free(act);
  }
  free(jq);
}

/* Extract a job from the queue. */
job* jdequeue (jqueue *jq)
{
  job *tmp = jq->first;
  if (jq->last == tmp)
    jq->last = NULL;
  jq->first = tmp->next;
  jq->size--;
  return tmp;
}

/* Add a job to the queue. */
void jenqueue (jqueue *jq, job *j)
{
  j->next  = NULL;
  if (jq->first == NULL) {
    jq->first = j;
    jq->last  = j;
  } else {
    (jq->last)->next = j;
    jq->last = j;
  }
  jq->size++;
}

/* Create a pool thread with n threads. */
pool *pool_create (unsigned int n)
{
  pool *P    = (pool*) malloc(sizeof(pool));
  P->threads = (pthread_t*) malloc(sizeof(pthread_t)*n);
  P->queue   = jqueue_new();
  P->size    = n;
  P->wmutex  = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  P->idle    = (pthread_cond_t*)  malloc(sizeof(pthread_cond_t));
  P->working = 0;
  pthread_mutex_init(P->wmutex, NULL);
  pthread_cond_init(P->idle, NULL);
#ifdef __APPLE__
  named_sem_init(P->qmutex, QMNAME, 1);
  named_sem_init(P->st, STNAME, 0);
#else
  P->qmutex  = (sem_t*) malloc(sizeof(sem_t));
  P->st      = (sem_t*) malloc(sizeof(sem_t));
  sem_init(P->qmutex,0,1);
  sem_init(P->st,0,0);
#endif
  int i;
  for (i = 0; i < n; i++)
    pthread_create(&(P->threads[i]), NULL, _worker, P);
  return P;
}

/* Delete a pool thread. */
void pool_del (pool *P)
{
  delete_jobs(P, P->size);
  pool_wait(P);
  free(P->threads);
  jqueue_del(P->queue);
  pthread_mutex_destroy(P->wmutex);
  pthread_cond_destroy(P->idle);
#ifdef __APPLE__
  sem_close(P->qmutex);
  sem_close(P->st);
  sem_unlink(QMNAME);
  sem_unlink(STNAME);
#else
  sem_destroy(P->qmutex);
  sem_destroy(P->st);
  free(P->qmutex);
  free(P->st);
#endif
  free(P->wmutex);
  free(P->idle);
  free(P);
}

/* Worker. This is what the thread is doing all the time. */
void *_worker (void *vp)
{
  pool *P = (pool*) vp;
  job *my_job;
  while (1) {
    sem_wait(P->qmutex);
    if (P->queue->size != 0) {
      pthread_mutex_lock(P->wmutex);
      P->working++;
      pthread_mutex_unlock(P->wmutex);
      my_job = jdequeue(P->queue);
      sem_post(P->qmutex);
      if (my_job->func != NULL) {
        my_job->func(my_job->args);
      }
      pthread_mutex_lock(P->wmutex);
      P->working--;
      if (!P->working)
        pthread_cond_signal(P->idle);
      pthread_mutex_unlock(P->wmutex);
      if (my_job->func == NULL) {
        free(my_job);
        return NULL;
      }
      else 
        free(my_job);
    } else {
      sem_post(P->qmutex);
      sem_wait(P->st);
    }
  }
}

/* Send a NULL job to stop m workers. Its used only when the pool is deleted. */
void delete_jobs (pool *P, int m)
{
  int i;
  job *j;
  sem_wait(P->qmutex);
  for (i = 0; i < m; i++) {
    j = (job*) malloc(sizeof(job));
    j->func = NULL;
    jenqueue(P->queue, j);
  }
  for (i = 0; i < m; i++) {
    sem_post(P->st);
  }
  sem_post(P->qmutex);
}

/* Add a job to the queue. */
void pool_send_job (pool *P, void *func, void *args)
{
  job *some_job = (job*) malloc(sizeof(job));
  some_job->func = func;
  some_job->args = args;
  sem_wait(P->qmutex);
  jenqueue(P->queue, some_job);
  sem_post(P->qmutex);
  sem_post(P->st);
}

/* Wait for all jobs to finish. */
void pool_wait (pool *P)
{
  pthread_mutex_lock(P->wmutex);
  while (P->working || P->queue->size) {
    pthread_cond_wait(P->idle, P->wmutex);
  }
  pthread_mutex_unlock(P->wmutex);
}

/* vim: set ts=2 sw=2 sts=2 tw=80 : */
