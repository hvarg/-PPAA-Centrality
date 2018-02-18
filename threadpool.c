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

#define QMNAME  "/s1"
#define STNAME  "/s2"
#define QMNAME2 "/s3"
#define STNAME2 "/s4"
#endif

/* Create a job queue. */
jqueue* jqueue_new (void) {
  jqueue *new = (jqueue*) malloc(sizeof(jqueue));
  new->first = NULL;
  new->last  = NULL;
  new->size  = 0;
  return new;
}

/* Delete a job queue. */
void jqueue_del (jqueue *jq) {
  job *act, *next;
  for (act = jq->first; act != NULL; act = next) {
    next = act->next;
    free(act);
  }
  free(jq);
}

/* Extract a job from the queue. */
job* jdequeue (jqueue *jq) {
  job *tmp = jq->first;
  if (jq->last == tmp)
    jq->last = NULL;
  jq->first = tmp->next;
  jq->size--;
  return tmp;
}

/* Add a job to the queue.
 * This function is used to send data from discovery to accumulation. */
void jenqueue (jqueue *jq, int id, int ph, float *sigma, ilist **P, ilist **S) {
  /* Create job and set variables. */
  job *j    = (job*) malloc(sizeof(job));
  j->ph     = ph;
  j->id     = id;
  j->sigma  = sigma;
  j->P      = P;
  j->S      = S;
  j->next   = NULL;
  /* Add to queue. */
  if (jq->first == NULL) {
    jq->first = j;
    jq->last  = j;
  } else {
    (jq->last)->next = j;
    jq->last = j;
  }
  jq->size++;
}

/* Add a job to the queue.
 * Adds the bc to an existing job and send it from accum to sum task. */
void jenqueue2 (jqueue *jq, job *j, float *bc) {
  j->bc = bc;
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

/* Create a pipeline to compute the betweenness centrality of some graph G,
 * with n threads to do the discovery task, m threads to do the accumulation
 * task and one thread to do the sum of the results. */
pipeline_cent *pipeline_create (graph *G, unsigned int n, unsigned int m) {
  pipeline_cent *P  = (pipeline_cent*) malloc(sizeof(pipeline_cent));
  P->G              = G;
  P->n_discovery    = n;
  P->n_accum        = m;
  P->thr_discovery  = (pthread_t*) malloc(sizeof(pthread_t) * n);
  P->thr_accum      = (pthread_t*) malloc(sizeof(pthread_t) * m);
  P->bc             = (float*) calloc(G->size, sizeof(float));
  P->count          = 0;
  P->queue          = jqueue_new();
  P->queue2         = jqueue_new();
  P->wmutex         = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(P->wmutex, NULL);
  /* Apple does not accept anonymous semaphores so...
   * TODO: this can be more beautiful. */
#ifdef __APPLE__
  named_sem_init(P->qmutex, QMNAME, 1);
  named_sem_init(P->qmutex2, QMNAME2, 1);
  named_sem_init(P->st, STNAME, 0);
  named_sem_init(P->st2, STNAME2, 0);
  /*if ((P->qmutex = sem_open(QMNAME, O_CREAT|O_EXCL, 0644, 1)) == SEM_FAILED) {
    sem_unlink(QMNAME);
    if (!(P->qmutex = sem_open(QMNAME, O_CREAT|O_EXCL, 0644, 1)))
      return NULL;
  }
  if ((P->st = sem_open(STNAME, O_CREAT|O_EXCL, 0644, 0)) == SEM_FAILED) {
    sem_unlink(STNAME);
    if (!(P->st = sem_open(STNAME, O_CREAT|O_EXCL, 0644, 0)))
      return NULL;
  }
  if ((P->qmutex2 = sem_open(QMNAME2, O_CREAT|O_EXCL, 0644, 1)) == SEM_FAILED) {
    sem_unlink(QMNAME2);
    if (!(P->qmutex2 = sem_open(QMNAME2, O_CREAT|O_EXCL, 0644, 1)))
      return NULL;
  }
  if ((P->st2 = sem_open(STNAME2, O_CREAT|O_EXCL, 0644, 0)) == SEM_FAILED) {
    sem_unlink(STNAME2);
    if (!(P->st2 = sem_open(STNAME2, O_CREAT|O_EXCL, 0644, 0)))
      return NULL;
  }*/
#else
  P->qmutex  = (sem_t*) malloc(sizeof(sem_t));
  P->qmutex2 = (sem_t*) malloc(sizeof(sem_t));
  P->st      = (sem_t*) malloc(sizeof(sem_t));
  P->st2     = (sem_t*) malloc(sizeof(sem_t));
  sem_init(P->qmutex, 0,1);
  sem_init(P->qmutex2,0,1);
  sem_init(P->st, 0,0);
  sem_init(P->st2,0,0);
#endif
  return P;
}

/* Start the tasks. */
void pipeline_start (pipeline_cent *P) {
  pthread_mutex_lock(P->wmutex);
  for (int i = 0; i < P->n_discovery; i++)
    pthread_create(&(P->thr_discovery[i]), NULL, _discovery, P);
  for (int i = 0; i < P->n_accum; i++)
    pthread_create(&(P->thr_accum[i]), NULL, _accum, P);
  pthread_create(&(P->thr_sum), NULL, _sum, P);
}

/* Delete the pipeline. G is NOT deleted. */
void pipeline_del (pipeline_cent *P) {
  pipeline_wait(P);
  free(P->thr_discovery);
  free(P->thr_accum);
  jqueue_del(P->queue);
  jqueue_del(P->queue2);
  pthread_mutex_destroy(P->wmutex);
#ifdef __APPLE__
  sem_close(P->qmutex);
  sem_close(P->qmutex2);
  sem_close(P->st);
  sem_close(P->st2);
  sem_unlink(QMNAME);
  sem_unlink(QMNAME2);
  sem_unlink(STNAME);
  sem_unlink(STNAME2);
#else
  sem_destroy(P->qmutex);
  sem_destroy(P->qmutex2);
  sem_destroy(P->st);
  sem_destroy(P->st2);
  free(P->qmutex);
  free(P->qmutex2);
  free(P->st);
  free(P->st2);
#endif
  free(P->wmutex);
  free(P->bc);
  free(P);
}

/* Discovery task for brandes algorithm. */
void *_discovery (void *vp) {
  pipeline_cent *P = (pipeline_cent*) vp;
  int id, i;
  int c, ph, v, w,
      *d = (int *) malloc(sizeof(int) * (P->G)->size);
  item *elem;
  ilist **Pr, **S;
  float *sigma;
  /* Get a node id. The nodes are processed sequentially. */
  sem_wait(P->mutex_count);
  while (P->count < (P->G)->size) {
    id = P->count++;
    sem_post(P->mutex_count);
    /* Initiation. New specific vars for this node and reset the othres. */
    sigma = (float *) malloc(sizeof(float) * (P->G)->size);
    Pr    = (ilist **) malloc(sizeof(ilist*) * (P->G)->size);
    S     = (ilist **) malloc(sizeof(ilist*) * (P->G)->size);
    for (i = 0; i < (P->G)->size; i++) {
      S[i] = NULL;
      Pr[i] = NULL;
    }
    c  = 1;
    ph = 0;
    S[ph] = new_ilist();
    ilist_add(S[ph], id);
    for (i = 0; i < (P->G)->size; i++) {
      Pr[i] = new_ilist();
      sigma[i] = 0.0f;
      d[i] = -1;
    }
    sigma[id] = 1.0;
    d[id] = 0;
    /* Compute accumulation */
    while (c > 0) {
      c = 0;
      for (elem = S[ph]->first; elem != NULL; elem = elem->next) {
        v = elem->value;
        for (i = 0; i < (P->G)->nsize[v]; i++) {
          w = (P->G)->neigh[v][i];
          if (d[w] < 0) {
            if (S[ph+1] == NULL) S[ph+1] = new_ilist();
            ilist_add(S[ph+1], w);
            c++;
            d[w] = d[v] + 1;
          }
          if (d[w] == d[v] + 1) {
            sigma[w] += sigma[v];
            ilist_add(Pr[w], v);
          }
        }
      }
      ph++;
    }
    ph--;
    /* Send this node data to accum task. */
    sem_wait(P->qmutex);
    jenqueue(P->queue, id, ph, sigma, Pr, S);
    sem_post(P->qmutex);
    sem_post(P->st);
    /* Start again. */
    sem_wait(P->mutex_count);
  }
  free(d);
  return NULL;
}

/* Accumulation task for brandes algorithm*/
void *_accum (void *vp) {
  pipeline_cent *P = (pipeline_cent*) vp;
  job *j;
  int t, v, w;
  float *bc,
        *delta = (float *) malloc(sizeof(float) * (P->G)->size);
  item *node1, *node2;
  do {
    sem_wait(P->qmutex);
    if (P->queue->size == 0) {
      /* No jobs currently, waiting. */
      sem_post(P->qmutex);
      sem_wait(P->st);
    } else {
      /* Get the job (node data). */
      j = jdequeue(P->queue);
      sem_post(P->qmutex);
      /* End condition of this thread. */
      if (j->id < 0) {
        free(j);
        free(delta);
        sem_post(P->qmutex); // Because this thread is dead now.
        return NULL;
      } else {
        bc = (float*) calloc((P->G)->size, sizeof(float));  // This node bc.
        for (t=0; t < (P->G)->size; delta[t++] = 0.0f);     // Reset delta
        /* Dependency accumulation. */
        while (j->ph > 0) { 
          for (node1 = (j->S[j->ph])->first;
               node1 != NULL;
               node1 = node1->next) {
            w = node1->value;
            for (node2 = (j->P[w])->first;
                 node2 != NULL;
                 node2 = node2->next) {
              v = node2->value;
              delta[v] += (j->sigma[v]/j->sigma[w]) * (1+delta[w]);
            }
            bc[w] += delta[w];
          }
          j->ph--;
        }
        /* This data is not needed anymore. */
        for (t = 0; t < (P->G)->size; t++) {
          if (j->S[t] != NULL) ilist_del(j->S[t]);
          if (j->P[t] != NULL) ilist_del(j->P[t]);
        }
        free(j->sigma);
        free(j->P);
        free(j->S);
        /* Send this node part of betweenness centrality to sum task. */
        sem_wait(P->qmutex2);
        jenqueue2(P->queue2, j, bc);
        sem_post(P->qmutex2);
        sem_post(P->st2);
      }
    }
  } while (1);
}

/* Sum of the partial results. */
void *_sum (void *vp) {
  pipeline_cent *P = (pipeline_cent*) vp;
  job *j;
  int ended = 0, i;
  /* This thread ends when has processed all the nodes. */
  while (ended < (P->G)->size) {
    sem_wait(P->qmutex2);
    if (P->queue2->size == 0) {
      /* No jobs currently. waiting. */
      sem_post(P->qmutex2);
      sem_wait(P->st2);
    } else {
      /* Get the node bc. */
      j = jdequeue(P->queue2);
      sem_post(P->qmutex2);
      for (i = 0; i < (P->G)->size; i++)
        P->bc[i] += j->bc[i]; // The work, sum the bc.
      ended++;
      /* Delete the job. */
      free(j->bc);
      free(j);
    }
  }
  /* Send the end condition to accum task. */
  for (i = 0; i < P->n_accum; i++) {
    sem_wait(P->qmutex);
    jenqueue(P->queue, -1, 0, NULL, NULL, NULL); //this will end a accum task.
    sem_post(P->qmutex);
    sem_post(P->st);
  }
  /* Unlock the pipeline. */
  pthread_mutex_unlock(P->wmutex);
  return NULL;
}

/* Wait for all jobs to finish. */
void pipeline_wait (pipeline_cent *P) {
  pthread_mutex_lock(P->wmutex);
  pthread_mutex_unlock(P->wmutex);
}

/* vim: set ts=2 sw=2 sts=2 tw=80 : */
