#ifndef POOL_H_
#define POOL_H_

#include <pthread.h>
#include <semaphore.h>

/** TODO **/
struct item{
  void *value;
  struct item *next;
};

struct list{
  struct item *first;
  struct item *last;
  unsigned int size;
};
/****/

/* Pool thread. */
typedef struct {
  pthread_t       *threads;   // Threads.
  struct list     *queue;     // Job queue. FIXME
  unsigned int    size,       // Total number of threads.
                  working;    // Number of currently working threads.
  sem_t           *qmutex,    // Queue mutex.
                  *st;        // Semaphore for start threads.
  pthread_mutex_t *wmutex;    // Working mutex.
  pthread_cond_t  *idle;      // When idle
} pool;

/* Job to be done. */
typedef struct {
  void (*func)(void* args);   // Function.
  void *args;                 // Arguments.
} job;

pool *pool_create(unsigned int NTHREADS);
void pool_del(pool *P);
void *_worker(void *vp);
void pool_send_job(pool *P, void *func, void *args);
void pool_wait(pool *P);
void pool_rm_job(pool *P);



struct list *new_list(void);
void list_del(struct list *alist);
void *extract_first(struct list *alist);
void list_add(struct list *alist, void *element);

#endif
/* vim: set ts=2 sw=2 sts=2 tw=80 : */
