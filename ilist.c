#include "ilist.h"
#include <stdlib.h>

/* Create a new list. */
ilist *new_ilist(void)
{
  ilist *new = (ilist*) malloc(sizeof(ilist));
  new->first = NULL;
  new->last  = NULL;
  return new;
}

/* Add a element to a list. The element is stored in a 'item'. */
void ilist_add(ilist *l, int val)
{
  item *new = (item *) malloc(sizeof(item));
  new->value = val;
  new->next  = NULL;
  if (l->first == NULL) {
    l->first = new;
    l->last  = new;
  } else {
    (l->last)->next = new;
    l->last = new;
  }
}

/* Delete a list (and free). */
void ilist_del(ilist *l)
{
  item *act, *next;
  for(act = l->first; act != NULL; act = next){
    next = act->next;
    free(act);
  }
  free(l);
}

/* vim: set ts=2 sw=2 sts=2 tw=80 : */
