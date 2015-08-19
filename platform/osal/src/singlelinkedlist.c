#include "singlelinkedlist.h"
#include "coov4.h"

#include <string.h>

typedef struct _node_{
  void * element;
  struct _node_ * next;
} * Node;


typedef struct _SingleLinkedList_ {
  OE oe;
  Node first;
  Node last_elm;
  int size;
  Node last;
  uint lasti;
} * SingleLinkedList;


static Node NodeNew(OE oe, void * element, Node next)
{
  Node res = oe->getmem(sizeof(*res));
  if (!res) return res;
  
  res->element = element;
  res->next = next;

  return res;
}

COO_DEF(List, void, add_element, void * e)
  SingleLinkedList ths = (SingleLinkedList)this->impl;
  OE oe = ths->oe;
  if (ths->first == 0) {
    ths->first = ths->last_elm = NodeNew(oe, e, 0);
    ths->size = 1;
    ths->last = 0;
    ths->lasti = 0;
  } else {
    Node cur = ths->last_elm;
    cur->next = ths->last_elm = NodeNew(oe,e,0);
    ths->size++;
    ths->last = 0;
    ths->lasti = 0;
  }

}


COO_DEF(List, void *, get_element, uint i)
  SingleLinkedList ths = (SingleLinkedList)this->impl;

  if (i >= ths->size) return 0;


  if (ths->last) {

    // are we asking for the next item
    if (i == ths->lasti+1) {
      Node res = ths->last->next;
      ths->last = res;
      ths->lasti = i;
      return res->element;
    }

    // are we asking for the prev element again
    if (i == ths->lasti) {
      Node res = ths->last;
      return res->element;
    }
  }
// TODO(rwz): Optimize maybe if (i > size/2) { start from last }.
  {
    uint j = 0;
    Node cur = ths->first;
    while(cur->next && j++ < i)
      cur = cur->next;
    ths->last = cur;
    ths->lasti = i;
    return cur->element;
  }
}

COO_DEF(List, uint, size) 
  SingleLinkedList ths = (SingleLinkedList)this->impl;
  return ths->size;
}

COO_DEF(List, void *, rem_element, uint i)
  SingleLinkedList ths = (SingleLinkedList)this->impl;
  if (i >= ths->size) return 0;
  {
    void * elm = 0;
    OE oe = ths->oe;
    uint j = 0;
    Node cur = ths->first;
    Node prv = 0;
    while(cur->next && j++ < i) {
        prv = cur;
        cur=cur->next;
    }

    if (!prv) // first elm
      ths->first = cur->next;
    else
      prv->next = cur->next;


    if (cur == ths->last_elm) {
      ths->last_elm = prv;
    }

    elm = cur->element;
    oe->putmem(cur);
    ths->size--;
    ths->last = 0;
    ths->lasti = 0;
    return elm;
  }
}



List SingleLinkedList_new(OE oe) {
  SingleLinkedList sll = 0;
  List res = (List)oe->getmem(sizeof(*res));
  if (!res) return 0;
  zeromem(res, sizeof(*res));

  sll = (SingleLinkedList)oe->getmem(sizeof(*sll));
  if (!sll) {
    oe->putmem(res);
    return 0;
  }
  zeromem(sll, sizeof(*sll));
  
  res->add_element = COO_attach(res, List_add_element);
  res->get_element = COO_attach(res, List_get_element);
  res->rem_element = COO_attach(res, List_rem_element);
  res->size =        COO_attach(res, List_size);
  sll->first = 0;
  sll->size = 0;
  sll->oe = oe;
  res->impl = sll;
  return res;  
}


void SingleLinkedList_destroy( List * l ) {
  SingleLinkedList sll = 0;
  Node cur = 0;

  if (!l) return;
  if (!*l) return;

  sll = (*l)->impl;

  if ( sll) {
    OE oe = sll->oe;
    cur = sll->first;

    while(cur) {
      Node nxt = cur->next;
      oe->putmem(cur);
      cur=nxt;
    }
    oe->putmem( sll );
    oe->putmem(*l);
    *l = 0;
  }
}
