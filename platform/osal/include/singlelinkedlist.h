#ifndef SINGLE_LINKED_LIST_H
#define SINGLE_LINKED_LIST_H
#include "list.h"
void SingleLinkedList_destroy( List * l );
struct _operating_environment_;
List SingleLinkedList_new(struct _operating_environment_ * oe);
#endif
