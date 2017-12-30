#ifndef ILIST_H_
#define ILIST_H_

typedef struct _item {
  int value;
  struct _item *next;
} item;

typedef struct{
  item *first, *last;
} ilist;

ilist *new_ilist(void);
void ilist_add(ilist *l, int val);
void ilist_del(ilist *l);

#endif
/* vim: set ts=2 sw=2 sts=2 tw=80 : */
