#include <stdlib.h>
#include "threadpool.h"

#define QMNAME "/s1"
#define STNAME "/s2"
/** TODO **/
struct list *new_list()
{
  struct list * new = (struct list *) malloc(sizeof(struct list));
  new->first = NULL;
  new->last  = NULL;
  new->size  = 0;
  return new;
}

void list_del(struct list *alist)
{
  struct item *act, *next;
  for(act = alist->first; act != NULL; act = next){
    next = act->next;
    free(act->value);
    free(act);
  }
  free(alist);
}

void *extract_first(struct list *alist)
{
  struct item *tmp = alist->first;
  void *val = tmp->value;
  if (alist->last == tmp)
    alist->last = NULL;
  alist->first = tmp->next;
  alist->size--;
  free(tmp);
  return val;
}

void list_add(struct list *alist, void *element)
{
  struct item *new = (struct item *) malloc(sizeof(struct item));
  new->value = element;
  new->next  = NULL;
  if (alist->first == NULL) {
    alist->first = new;
    alist->last  = new;
  } else {
    (*alist->last).next = new;
    alist->last = new;
  }
  alist->size++;
}
/****/

/* Create a pool thread with n threads. */
pool *pool_create(unsigned int n)
{
  pool *P    = (pool*) malloc(sizeof(pool));
  P->threads = (pthread_t*) malloc(sizeof(pthread_t)*n);
  P->queue   = new_list(); //FIXME
  P->size    = n;
  P->wmutex  = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  P->idle    = (pthread_cond_t*)  malloc(sizeof(pthread_cond_t));
  P->working = 0;
  pthread_mutex_init(P->wmutex, NULL);
  pthread_cond_init(P->idle, NULL);
#ifdef __APPLE__
  if ((P->qmutex = sem_open(QMNAME, O_CREAT|O_EXCL, 0644, 1)) == SEM_FAILED) {
    sem_unlink(QMNAME);
    if (!(P->qmutex = sem_open(QMNAME, O_CREAT|O_EXCL, 0644, 1)))
      return NULL;
  }
  if ((P->st = sem_open(STNAME, O_CREAT|O_EXCL, 0644, 0)) == SEM_FAILED) {
    sem_unlink(STNAME);
    if (!(P->st = sem_open(STNAME, O_CREAT|O_EXCL, 0644, 0)))
      return NULL;
  }
#else
  P->qmutex  = (sem_t*) malloc(sizeof(sem_t));
  P->st      = (sem_t*) malloc(sizeof(sem_t));
  sem_init(P->qmutex,0,1);
  sem_init(P->st,0,0);
#endif
  for (int i = 0; i < n; i++){
    pthread_create(&(P->threads[i]), NULL, _worker, P);
  }
  return P;
}

void pool_del(pool *P)
{
  //TODO: memory leak, destroy P->threads[i]
  for(int i=0; i < P->size; i++)
    pool_rm_job(P);
  pool_wait(P);

  free(P->threads);
  list_del(P->queue);
  pthread_mutex_destroy(P->wmutex);
  pthread_cond_destroy(P->idle);

#ifdef __APPLE__
  // For some reason i cant close this one, maybe bc is used? FIXME
  sem_close(P->qmutex);
  sem_close(P->st);
  sem_unlink("/s1");
  sem_unlink("/s2");
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

void *_worker(void *vp)
{
  pool *P = (pool*) vp;
  job *my_job;
  while (1) {
    sem_wait(P->qmutex);
    if (P->queue->size != 0) {
      pthread_mutex_lock(P->wmutex);
      P->working++;
      pthread_mutex_unlock(P->wmutex);
      my_job = (job*) extract_first(P->queue);
      sem_post(P->qmutex);
      if ((int) my_job != 0) {
        my_job->func(my_job->args);
        free(my_job);
      }
      pthread_mutex_lock(P->wmutex);
      P->working--;
      if (!P->working)
        pthread_cond_signal(P->idle);
      pthread_mutex_unlock(P->wmutex);
      if ((int) my_job == 0) {
        return NULL;
      }
    } else {
      sem_post(P->qmutex);
      sem_wait(P->st);
    }
  }
}

void pool_rm_job(pool *P)
{
  sem_wait(P->qmutex);
  list_add(P->queue, 0);
  sem_post(P->qmutex);
  sem_post(P->st);
}

void pool_send_job(pool *P, void *func, void *args)
{
  job *some_job = (job*) malloc(sizeof(job));
  some_job->func = func;
  some_job->args = args;
  sem_wait(P->qmutex);
  list_add(P->queue, some_job);
  sem_post(P->qmutex);
  sem_post(P->st);
}

void pool_wait(pool *P)
{
  pthread_mutex_lock(P->wmutex);
  while (P->working || P->queue->size) {
    pthread_cond_wait(P->idle, P->wmutex);
  }
  pthread_mutex_unlock(P->wmutex);
}

/* vim: set ts=2 sw=2 sts=2 tw=80 : */
