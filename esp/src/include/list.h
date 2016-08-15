/*
 * list.h
 *
 *  Created on: 13. stu 2015.
 *      Author: Marko
 */

#ifndef IOT_DEMO_INCLUDE_LIST_H_
#define IOT_DEMO_INCLUDE_LIST_H_

#include "ets_sys.h"

typedef struct list{
    struct list *prev;
    struct list *next;
    void *item;
} list_t;

list_t *list_createList();

void list_addItem(list_t *list, void *item);

list_t *list_removeItem(list_t *list, void *item, bool releaseItem);

void list_deleteList(list_t *list, bool releaseItems);

int16_t list_getLength(list_t *list);

void *list_toArray(list_t *list);

#endif /* IOT_DEMO_INCLUDE_LIST_H_ */
