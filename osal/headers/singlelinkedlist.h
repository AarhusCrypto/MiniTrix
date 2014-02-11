#ifndef SINGLE_LINKED_LIST_H
#define SINGLE_LINKED_LIST_H
#include "list.h"
#include "osal.h"

List SingleLinkedList_new(OE oe);
void SingleLinkedList_destroy( List * l );

#endif
